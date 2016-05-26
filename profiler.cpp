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

namespace demo {
  struct Work {
    uv_work_t request;
    Persistent<Function> callback;

    InterceptionContext *context;
    InterceptionDevice *device;
    InterceptionKeyStroke *stroke;
    Isolate *isolate;
    int keyCode;
    bool keyDown;
    bool keyE0;
    bool hasData;
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
  InterceptionKeyStroke globalStroke;
  Work *work;

  void HandleInterception2(uv_work_t *req) {
    Work *w = static_cast<Work *>(req->data);
    Isolate *isolate = Isolate::GetCurrent();

    Handle<Value> arglol[1];
    arglol[0] = String::NewFromUtf8(isolate, "meow");
    // arglol[0] = Number::New(isolate, 0);
    // Local<Function> func = Local<Function>::New(isolate, work->callback);
    // func->Call(isolate->GetCurrentContext()->Global(), 1, arglol);
  }

  void HandleInterceptionComplete2(uv_work_t *req, int status) {
    // Work *w = static_cast<Work *>(req->data);
    // w->hasData = false;
  }

  void async_cb(uv_async_t* handle) {
    // Work w;
    // w = *((Work*) handle->data);
    Isolate *isolate = Isolate::GetCurrent();
    v8::HandleScope handleScope(isolate);

    // Local<Function> func = Local<Function>::New(isolate, work->callback);
    // func->Call(isolate->GetCurrentContext()->Global(), 0, NULL);

    if(work->hasData) {
      Handle<Value> arglol[4];
      // arglol[0] = String::NewFromUtf8(isolate, "meow");
      // arglol[0] = Number::New(isolate, 0);
      arglol[0] = Number::New(isolate, work->keyCode);
      arglol[1] = Boolean::New(isolate, work->keyDown);
      arglol[2] = Boolean::New(isolate, work->keyE0);
      arglol[3] = v8::String::NewFromUtf8(isolate, work->hwid.c_str());
      Local<Function> func = Local<Function>::New(isolate, work->callback);
      func->Call(isolate->GetCurrentContext()->Global(), 4, arglol);
      work->hasData = false;
    }
  }

  // void HandleInterception(Isolate *isolate, InterceptionContext *context, InterceptionDevice *device, InterceptionKeyStroke *stroke, Work *work) {
  void HandleInterception(uv_work_t *req) {
  // void HandleInterception(void *arg) {
    // uv_work_t *req = ((uv_work_t *) arg);
    Work *w = static_cast<Work *>(req->data);
    // Isolate *isolate = work->isolate;
    InterceptionContext context = *work->context;
    InterceptionDevice device = *work->device;
    InterceptionKeyStroke stroke;

    wchar_t hardware_id[500];

    // Handle<Value> arglol[1];
    // arglol[0] = String::NewFromUtf8(isolate, "meow");
    // Local<Function> func = Local<Function>::New(isolate, work->callback);
    // func->Call(isolate->GetCurrentContext()->Global(), 1, arglol);

    // if(!work->hasData) {
      while(interception_receive(globalContext, globalDevice = interception_wait(globalContext), (InterceptionStroke *)&globalStroke, 1) > 0) {
      // while(interception_receive(context, device = interception_wait(context), (InterceptionStroke)*stroke, 1) > 0) {
      // uv_async_init(uv_default_loop(), w->async, async_cb);

        size_t length = interception_get_hardware_id(globalContext, globalDevice, hardware_id, sizeof(hardware_id));
        std::wstring hwid2(hardware_id);
        std::string hardwareID = std::string(hwid2.begin(), hwid2.end());
        // meow = true;
      // Isolate *isolate = Isolate::GetCurrent();
      // //   Local<Number> keyCode = Number::New(isolate, (*stroke).code);
      // //   Local<Boolean> keyDown = Boolean::New(isolate, ((*stroke).state == INTERCEPTION_KEY_DOWN || (*stroke).state == INTERCEPTION_KEY_DOWN + INTERCEPTION_KEY_E0));
      // //   Local<Boolean> keyE0 = Boolean::New(isolate, ((*stroke).state == INTERCEPTION_KEY_UP + INTERCEPTION_KEY_E0 || (*stroke).state == INTERCEPTION_KEY_DOWN + INTERCEPTION_KEY_E0));
      // //
      //
        // if(!w->hasData) {
          // Isolate *isolate = Isolate::GetCurrent();
          w->keyCode = globalStroke.code;
          w->keyDown = (globalStroke.state == INTERCEPTION_KEY_DOWN || globalStroke.state == INTERCEPTION_KEY_DOWN + INTERCEPTION_KEY_E0);
          w->keyE0 = (globalStroke.state == INTERCEPTION_KEY_UP + INTERCEPTION_KEY_E0 || globalStroke.state == INTERCEPTION_KEY_DOWN + INTERCEPTION_KEY_E0);
          w->hwid = hardwareID;
          w->hasData = true;
          w->stroke = &globalStroke;
          // uv_async_send(w->async);
          // uv_queue_work(uv_default_loop(), &work->request, HandleInterception2, HandleInterceptionComplete2);
        // }
      // //
        // interception_send(context, device, (InterceptionStroke *)&stroke, 1);
      //
        break;
      }
    // }
  }

  void grunsk(uv_work_t *req, int status) {
  }

  void HandleInterceptionComplete(uv_work_t *req, int status) {
    Work *w = static_cast<Work *>(req->data);
    Isolate *isolate = Isolate::GetCurrent();
    v8::HandleScope handleScope(isolate);

    if(w->hasData) {
      Handle<Value> arglol[4];
      // arglol[0] = String::NewFromUtf8(isolate, "meow");
      // arglol[0] = Number::New(isolate, 0);
      arglol[0] = Number::New(isolate, w->keyCode);
      arglol[1] = Boolean::New(isolate, w->keyDown);
      arglol[2] = Boolean::New(isolate, w->keyE0);
      arglol[3] = v8::String::NewFromUtf8(isolate, w->hwid.c_str());
      Local<Function> func = Local<Function>::New(isolate, w->callback);
      func->Call(isolate->GetCurrentContext()->Global(), 4, arglol);
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

    uv_async_t meow;
    work->async = &meow;
    // meow.data = &work;
    // uv_async_init(uv_default_loop(), &meow, async_cb);
    // uv_async_send(&meow);

    work->active = true;
    // while(true == true) {
    // while(work->active) {
    //   work->active = false;
    uv_queue_work(uv_default_loop(), &work->request, HandleInterception, HandleInterceptionComplete);
    // }
    // }
  }

  void DestroyInterception(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();

    interception_destroy_context(globalContext);
  }

  void ContinueInterception(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();

    Local<Function> interceptionCallback = Local<Function>::Cast(args[0]);
    Persistent<Function, CopyablePersistentTraits<Function>> persistCallback(isolate, interceptionCallback);
    // work->callback = Persistent(isolate, persistCallback);
    work->callback.Reset(isolate, persistCallback);
    uv_queue_work(uv_default_loop(), &work->request, HandleInterception, HandleInterceptionComplete);
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

    short keyCode = GetKeyCode(keyName);
    bool isE0 = GetKeyIsE0(keyName);

    globalStroke.state = INTERCEPTION_KEY_UP;
    if(keyDown) globalStroke.state = INTERCEPTION_KEY_DOWN;
    if(isE0) globalStroke.state += INTERCEPTION_KEY_E0;
    globalStroke.code = keyCode;

    if(globalStroke.code != SCANCODE_NONE) {
      interception_send(globalContext, globalDevice, (InterceptionStroke *)&globalStroke, 1);
    }
  }

  void SendInterceptionDefault(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();

    interception_send(globalContext, globalDevice, (InterceptionStroke *)&globalStroke, 1);
    // PlaySound((LPCSTR) "activate_profile.wav", NULL, SND_FILENAME | SND_ASYNC);
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

    // Local<FunctionTemplate> continueTpl = FunctionTemplate::New(isolate, ContinueInterception);
    // Local<Function> continueFunc = continueTpl->GetFunction();
    // mainObj->Set(String::NewFromUtf8(isolate, "continue"), continueFunc);

    args.GetReturnValue().Set(mainObj);


    // Local<Function> interceptionCallback = Local<Function>::Cast(args[0]);
    // Persistent<Function, CopyablePersistentTraits<Function>> persistCallback(isolate, interceptionCallback);
    // Persistent<Function> bla;
    // bla.Reset(isolate, persistCallback);

    // InterceptionContext context;
    // InterceptionDevice device;
    // InterceptionKeyStroke stroke;
    //
    // context = interception_create_context();
    //
    // interception_set_filter(context, interception_is_keyboard, INTERCEPTION_FILTER_KEY_DOWN | INTERCEPTION_FILTER_KEY_UP | INTERCEPTION_FILTER_KEY_E0);
    //
    // while(interception_receive(context, device = interception_wait(context), (InterceptionStroke *)&stroke, 1) > 0) {
    //   // isolate = Isolate::GetCurrent();
    //   Handle<Value> arglol[1];
    //   // arglol[0] = String::NewFromUtf8(isolate, "meow");
    //   arglol[0] = Number::New(isolate, 0);
    //   Local<Function> func = Local<Function>::New(isolate, work->callback);
    //   func->Call(isolate->GetCurrentContext()->Global(), 1, arglol);
    //   // w->hasData = false;
    // }


    // Handle<Value> arglol[1];
    // arglol[0] = String::NewFromUtf8(isolate, "meow");
    // // Local<Object> woof = Object::New(isolate);
    // Local<Function> func = Local<Function>::New(isolate, work->callback);
    // func->Call(isolate->GetCurrentContext()->Global(), 1, arglol);

    // std::thread interceptionThread(HandleInterception, isolate, &context, &device, &stroke, work);
  }

  void Init(Local<Object> exports, Local<Object> module) {
    NODE_SET_METHOD(module, "exports", CreateInterception);
  }

  NODE_MODULE(addon, Init)
}
