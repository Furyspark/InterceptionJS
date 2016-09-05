#include "interception.h"
#include <node.h>
#include <v8.h>
#include <uv.h>
#include <string>
#include "scancodes.h"

#if       _WIN32_WINNT < 0x0500
  #undef  _WIN32_WINNT
  #define _WIN32_WINNT   0x0500
#endif
// #include <thread>

using namespace v8;

enum MouseWheelFlag
{
  MOUSE_WHEEL_NONE = 0,
  MOUSE_WHEEL_V = 1,
  MOUSE_WHEEL_H = 2
};

enum MouseMoveFlag
{
  MOUSE_MOVE_REL = 0,
  MOUSE_MOVE_ABS = 1
};

namespace demo {
  struct Work {
    uv_work_t request;
    Persistent<Function> callback;

    InterceptionContext *context;
    InterceptionDevice *device;
    InterceptionStroke *stroke;
    Isolate *isolate;
    int keyCode;
    bool keyDown;
    bool keyE0;
    bool hasData;
    int mouseWheel;
    int mouseMove;
    int deviceType;
    int x;
    int y;
    std::string hwid;

    bool active;
    uv_async_t *async;
  };

  using v8::Function;
  using v8::FunctionCallbackInfo;
  using v8::FunctionTemplate;
  using v8::Isolate;
  using v8::Local;
  using v8::Persistent;
  using v8::Null;
  using v8::Object;
  using v8::Value;
  using v8::Number;
  using v8::Boolean;

  InterceptionDevice globalDevice;
  InterceptionContext globalContext;
  InterceptionStroke globalStroke;
  InterceptionDevice lastKeyboardDevice;
  InterceptionDevice lastMouseDevice;
  Work *work;

  void HandleInterception(uv_work_t *req) {
    Work *w = static_cast<Work *>(req->data);
    InterceptionContext context = *work->context;
    InterceptionDevice device = *work->device;

    wchar_t hardware_id[500];

    while(interception_receive(globalContext, globalDevice = interception_wait(globalContext), &globalStroke, 1) > 0) {
      size_t length = interception_get_hardware_id(globalContext, globalDevice, hardware_id, sizeof(hardware_id));
      std::wstring hwid2(hardware_id);
      std::string hardwareID = std::string(hwid2.begin(), hwid2.end());

      if(interception_is_keyboard(globalDevice)) {
        InterceptionKeyStroke kstroke = *(InterceptionKeyStroke *) &globalStroke;
        lastKeyboardDevice = globalDevice;
        w->deviceType = 0;
        w->keyCode = kstroke.code;
        w->keyDown = (kstroke.state == INTERCEPTION_KEY_DOWN || kstroke.state == INTERCEPTION_KEY_DOWN + INTERCEPTION_KEY_E0);
        w->keyE0 = (kstroke.state == INTERCEPTION_KEY_UP + INTERCEPTION_KEY_E0 || kstroke.state == INTERCEPTION_KEY_DOWN + INTERCEPTION_KEY_E0);
      }
      else if(interception_is_mouse(globalDevice)) {
        lastMouseDevice = globalDevice;
        InterceptionMouseStroke mstroke = *(InterceptionMouseStroke *) &globalStroke;
        w->deviceType = 1;
        // Misc
        w->x = mstroke.x;
        w->y = mstroke.y;
        // Buttons
        w->keyCode = mstroke.state;
        w->keyDown = (mstroke.state == INTERCEPTION_MOUSE_LEFT_BUTTON_DOWN || mstroke.state == INTERCEPTION_MOUSE_MIDDLE_BUTTON_DOWN ||
          mstroke.state == INTERCEPTION_MOUSE_RIGHT_BUTTON_DOWN || mstroke.state == INTERCEPTION_MOUSE_BUTTON_4_DOWN ||
          mstroke.state == INTERCEPTION_MOUSE_BUTTON_5_DOWN);
        // Mouse wheel
        w->mouseWheel = MOUSE_WHEEL_NONE;
        if(mstroke.state == INTERCEPTION_MOUSE_WHEEL) {
          w->mouseWheel = MOUSE_WHEEL_V;
          w->y = mstroke.rolling;
        }
        else if(mstroke.state == INTERCEPTION_MOUSE_HWHEEL) {
          w->mouseWheel = MOUSE_WHEEL_H;
          w->x = mstroke.rolling;
        }
        // Mouse move
        w->mouseMove = MOUSE_MOVE_REL;
        if(mstroke.flags & INTERCEPTION_MOUSE_MOVE_ABSOLUTE) w->mouseMove = MOUSE_MOVE_ABS;
      }
      w->hwid = hardwareID;
      w->hasData = true;
      w->stroke = &globalStroke;
      break;
    }
  }

  void HandleInterceptionComplete(uv_work_t *req, int status) {
    Work *w = static_cast<Work *>(req->data);
    Isolate *isolate = Isolate::GetCurrent();
    v8::HandleScope handleScope(isolate);

    if(w->hasData) {
      Handle<Value> arglol[9];
      arglol[0] = Number::New(isolate, w->keyCode);
      arglol[1] = Boolean::New(isolate, w->keyDown);
      arglol[2] = Boolean::New(isolate, w->keyE0);
      arglol[3] = v8::String::NewFromUtf8(isolate, w->hwid.c_str());
      arglol[4] = Number::New(isolate, w->deviceType);
      arglol[5] = Number::New(isolate, w->mouseWheel);
      arglol[6] = Number::New(isolate, w->mouseMove);
      arglol[7] = Number::New(isolate, w->x);
      arglol[8] = Number::New(isolate, w->y);
      Local<Function> func = Local<Function>::New(isolate, w->callback);
      func->Call(isolate->GetCurrentContext()->Global(), 9, arglol);
      w->hasData = false;
    }

    w->active = true;

    uv_queue_work(uv_default_loop(), &w->request, HandleInterception, HandleInterceptionComplete);
  }

  void StartInterception(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();

    InterceptionContext context;
    InterceptionDevice device;
    work = new Work();
    work->request.data = work;
    work->context = &context;
    work->device = &device;
    work->isolate = isolate;
    work->hasData = false;

    Local<Function> interceptionCallback = Local<Function>::Cast(args[0]);
    Persistent<Function, CopyablePersistentTraits<Function>> persistCallback(isolate, interceptionCallback);
    work->callback.Reset(isolate, persistCallback);

    globalContext = interception_create_context();

    interception_set_filter(globalContext, interception_is_keyboard, INTERCEPTION_FILTER_KEY_DOWN | INTERCEPTION_FILTER_KEY_UP | INTERCEPTION_FILTER_KEY_E0);
    interception_set_filter(globalContext, interception_is_mouse, INTERCEPTION_FILTER_MOUSE_LEFT_BUTTON_DOWN | INTERCEPTION_FILTER_MOUSE_LEFT_BUTTON_UP |
      INTERCEPTION_FILTER_MOUSE_RIGHT_BUTTON_DOWN | INTERCEPTION_FILTER_MOUSE_RIGHT_BUTTON_UP |
      INTERCEPTION_FILTER_MOUSE_MIDDLE_BUTTON_DOWN | INTERCEPTION_FILTER_MOUSE_MIDDLE_BUTTON_UP |
      INTERCEPTION_FILTER_MOUSE_BUTTON_4_DOWN | INTERCEPTION_FILTER_MOUSE_BUTTON_4_UP |
      INTERCEPTION_FILTER_MOUSE_BUTTON_5_DOWN | INTERCEPTION_FILTER_MOUSE_BUTTON_5_UP |
      INTERCEPTION_FILTER_MOUSE_WHEEL | INTERCEPTION_FILTER_MOUSE_HWHEEL | INTERCEPTION_FILTER_MOUSE_MOVE);

    uv_async_t meow;
    work->async = &meow;

    work->active = true;
    uv_queue_work(uv_default_loop(), &work->request, HandleInterception, HandleInterceptionComplete);
  }

  void DestroyInterception(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();

    interception_destroy_context(globalContext);
  }

  bool GetKeyIsE0(std::string key) {
  	const int testNum = 12;
    std::string test[testNum];
    test[0] = "numpadenter";
    test[1] = "numpadsub";
    test[2] = "home";
    test[3] = "pgup";
    test[4] = "pgdn";
    test[5] = "end";
    test[6] = "insert";
    test[7] = "delete";
    test[8] = "left";
    test[9] = "right";
    test[10] = "up";
    test[11] = "down";
    for(int a = 0;a < testNum;a++) {
      if(key == test[a]) return true;
    }
    return false;
  }

  bool GetKeyIsMouse(std::string key) {
    if(key == "mousebuttonleft") return true;
    if(key == "mousebuttonmiddle") return true;
    if(key == "mousebuttonright") return true;
    if(key == "mousebutton4") return true;
    if(key == "mousebutton5") return true;
    if(key == "mousewheel") return true;
    if(key == "mousewheel") return true;
    return false;
  }

  short GetKeyCode(std::string key) {
    if(key == "left") return SCANCODE_LEFT;
    if(key == "right") return SCANCODE_RIGHT;
    if(key == "up") return SCANCODE_UP;
    if(key == "down") return SCANCODE_DOWN;

    if(key == "insert") return SCANCODE_INSERT;
    if(key == "home") return SCANCODE_HOME;
    if(key == "pgup") return SCANCODE_PGUP;
    if(key == "delete") return SCANCODE_DELETE;
    if(key == "end") return SCANCODE_END;
    if(key == "pgdn") return SCANCODE_PGDN;

    if(key == "1") return SCANCODE_1;
    if(key == "2") return SCANCODE_2;
    if(key == "3") return SCANCODE_3;
    if(key == "4") return SCANCODE_4;
    if(key == "5") return SCANCODE_5;
    if(key == "6") return SCANCODE_6;
    if(key == "7") return SCANCODE_7;
    if(key == "8") return SCANCODE_8;
    if(key == "9") return SCANCODE_9;
    if(key == "0") return SCANCODE_0;
    if(key == "vkbd") return SCANCODE_MINUS;
    if(key == "vkbb") return SCANCODE_PLUS;

    if(key == "escape") return SCANCODE_ESCAPE;
    if(key == "enter") return SCANCODE_ENTER;
    if(key == "tab") return SCANCODE_TAB;
    if(key == "backspace") return SCANCODE_BACKSPACE;
    if(key == "space") return SCANCODE_SPACE;

    if(key == "lctrl") return SCANCODE_LCTRL;
    if(key == "lshift") return SCANCODE_LSHIFT;
    if(key == "lalt") return SCANCODE_LALT;
    if(key == "space") return SCANCODE_SPACE;

    if(key == "sc029") return SCANCODE_APOSTROPHE;
    if(key == "vkdb") return SCANCODE_LBRACKET;
    if(key == "vkdd") return SCANCODE_RBRACKET;
    if(key == "vkba") return SCANCODE_COLON;
    if(key == "vkde") return SCANCODE_QUOTE;
    if(key == "vkbc") return SCANCODE_COMMA;
    if(key == "vkbe") return SCANCODE_DOT;
    if(key == "vkbf") return SCANCODE_SLASH;
    if(key == "vkdc") return SCANCODE_BACKSLASH;

    if(key == "capslock") return SCANCODE_CAPSLOCK;
    if(key == "numlock") return SCANCODE_NUMLOCK;
    if(key == "scrolllock") return SCANCODE_SCROLLLOCK;

    if(key == "f1") return SCANCODE_F1;
    if(key == "f2") return SCANCODE_F2;
    if(key == "f3") return SCANCODE_F3;
    if(key == "f4") return SCANCODE_F4;
    if(key == "f5") return SCANCODE_F5;
    if(key == "f6") return SCANCODE_F6;
    if(key == "f7") return SCANCODE_F7;
    if(key == "f8") return SCANCODE_F8;
    if(key == "f9") return SCANCODE_F9;
    if(key == "f10") return SCANCODE_F10;
    if(key == "f11") return SCANCODE_F11;
    if(key == "f12") return SCANCODE_F12;

    if(key == "f13") return SCANCODE_F13;
    if(key == "f14") return SCANCODE_F14;
    if(key == "f15") return SCANCODE_F15;
    if(key == "f16") return SCANCODE_F16;
    if(key == "f17") return SCANCODE_F17;
    if(key == "f18") return SCANCODE_F18;
    if(key == "f19") return SCANCODE_F19;
    if(key == "f20") return SCANCODE_F20;
    if(key == "f21") return SCANCODE_F21;
    if(key == "f22") return SCANCODE_F22;
    if(key == "f23") return SCANCODE_F23;
    if(key == "f24") return SCANCODE_F24;

    if(key == "numpad1") return SCANCODE_KP1;
    if(key == "numpad2") return SCANCODE_KP2;
    if(key == "numpad3") return SCANCODE_KP3;
    if(key == "numpad4") return SCANCODE_KP4;
    if(key == "numpad5") return SCANCODE_KP5;
    if(key == "numpad6") return SCANCODE_KP6;
    if(key == "numpad7") return SCANCODE_KP7;
    if(key == "numpad8") return SCANCODE_KP8;
    if(key == "numpad9") return SCANCODE_KP9;
    if(key == "numpad0") return SCANCODE_KP0;
    if(key == "numpadadd") return SCANCODE_KPPLUS;
    if(key == "numpadsub") return SCANCODE_KPMINUS;
    if(key == "numpadmult") return SCANCODE_KPMULT;

    if(key == "q") return SCANCODE_Q;
    if(key == "w") return SCANCODE_W;
    if(key == "e") return SCANCODE_E;
    if(key == "r") return SCANCODE_R;
    if(key == "t") return SCANCODE_T;
    if(key == "y") return SCANCODE_Y;
    if(key == "u") return SCANCODE_U;
    if(key == "i") return SCANCODE_I;
    if(key == "o") return SCANCODE_O;
    if(key == "p") return SCANCODE_P;
    if(key == "a") return SCANCODE_A;
    if(key == "s") return SCANCODE_S;
    if(key == "d") return SCANCODE_D;
    if(key == "f") return SCANCODE_F;
    if(key == "g") return SCANCODE_G;
    if(key == "h") return SCANCODE_H;
    if(key == "j") return SCANCODE_J;
    if(key == "k") return SCANCODE_K;
    if(key == "l") return SCANCODE_L;
    if(key == "z") return SCANCODE_Z;
    if(key == "x") return SCANCODE_X;
    if(key == "c") return SCANCODE_C;
    if(key == "v") return SCANCODE_V;
    if(key == "b") return SCANCODE_B;
    if(key == "n") return SCANCODE_N;
    if(key == "m") return SCANCODE_M;

    return SCANCODE_NONE;
  }

  void SendInterception(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();

    std::string keyName = std::string(*String::Utf8Value(args[0]->ToString()));
    bool keyDown = args[1]->BooleanValue();
    int x = args[2]->Int32Value();
    int y = args[3]->Int32Value();

    if(GetKeyIsMouse(keyName)) {
      InterceptionMouseStroke mstroke = *(InterceptionMouseStroke *) &globalStroke;
      if(keyDown) {
        if(keyName == "mousebuttonleft") mstroke.state = INTERCEPTION_MOUSE_LEFT_BUTTON_DOWN;
        else if(keyName == "mousebuttonmiddle") mstroke.state = INTERCEPTION_MOUSE_MIDDLE_BUTTON_DOWN;
        else if(keyName == "mousebuttonright") mstroke.state = INTERCEPTION_MOUSE_RIGHT_BUTTON_DOWN;
        else if(keyName == "mousebutton4") mstroke.state = INTERCEPTION_MOUSE_BUTTON_4_DOWN;
        else if(keyName == "mousebutton5") mstroke.state = INTERCEPTION_MOUSE_BUTTON_5_DOWN;
      }
      else {
        if(keyName == "mousebuttonleft") mstroke.state = INTERCEPTION_MOUSE_LEFT_BUTTON_UP;
        else if(keyName == "mousebuttonmiddle") mstroke.state = INTERCEPTION_MOUSE_MIDDLE_BUTTON_UP;
        else if(keyName == "mousebuttonright") mstroke.state = INTERCEPTION_MOUSE_RIGHT_BUTTON_UP;
        else if(keyName == "mousebutton4") mstroke.state = INTERCEPTION_MOUSE_BUTTON_4_UP;
        else if(keyName == "mousebutton5") mstroke.state = INTERCEPTION_MOUSE_BUTTON_5_UP;
      }
      if(keyName == "mousewheel") {
        mstroke.state = INTERCEPTION_MOUSE_WHEEL;
        mstroke.rolling = y;
      }
      else if(keyName == "mousewheel") {
        mstroke.state = INTERCEPTION_MOUSE_WHEEL;
        mstroke.rolling = y;
      }
      mstroke.x = 0;
      mstroke.y = 0;
      mstroke.flags = INTERCEPTION_MOUSE_MOVE_RELATIVE;
      interception_send(globalContext, lastMouseDevice, (InterceptionStroke *)&mstroke, 1);
    }
    else {
      short keyCode = GetKeyCode(keyName);
      bool isE0 = GetKeyIsE0(keyName);
      InterceptionKeyStroke kstroke = *(InterceptionKeyStroke *) &globalStroke;

      kstroke.state = INTERCEPTION_KEY_UP;
      if(keyDown) kstroke.state = INTERCEPTION_KEY_DOWN;
      if(isE0) kstroke.state += INTERCEPTION_KEY_E0;
      kstroke.code = keyCode;

      if(kstroke.code != SCANCODE_NONE) {
        interception_send(globalContext, lastKeyboardDevice, (InterceptionStroke *)&kstroke, 1);
      }
    }
  }

  void SendInterceptionDefault(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();

    interception_send(globalContext, globalDevice, &globalStroke, 1);
  }

  void CreateInterception(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();

    Local<Object> mainObj = Object::New(isolate);

    Local<Object> interceptionObj = Object::New(isolate);
    mainObj->Set(String::NewFromUtf8(isolate, "main"), interceptionObj);

    Local<FunctionTemplate> startTpl = FunctionTemplate::New(isolate, StartInterception);
    Local<Function> startFunc = startTpl->GetFunction();
    mainObj->Set(String::NewFromUtf8(isolate, "start"), startFunc);

    Local<FunctionTemplate> sendTpl = FunctionTemplate::New(isolate, SendInterception);
    Local<Function> sendFunc = sendTpl->GetFunction();
    mainObj->Set(String::NewFromUtf8(isolate, "send"), sendFunc);

    Local<FunctionTemplate> sendDefaultTpl = FunctionTemplate::New(isolate, SendInterceptionDefault);
    Local<Function> sendDefaultFunc = sendDefaultTpl->GetFunction();
    mainObj->Set(String::NewFromUtf8(isolate, "send_default"), sendDefaultFunc);

    Local<FunctionTemplate> destroyTpl = FunctionTemplate::New(isolate, DestroyInterception);
    Local<Function> destroyFunc = destroyTpl->GetFunction();
    mainObj->Set(String::NewFromUtf8(isolate, "destroy"), destroyFunc);

    args.GetReturnValue().Set(mainObj);
  }

  void Init(Local<Object> exports, Local<Object> module) {
    NODE_SET_METHOD(module, "exports", CreateInterception);
  }

  NODE_MODULE(addon, Init)
}
