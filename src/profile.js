function Profile() {
  this.initialize.apply(this, arguments);
}

Profile.prototype.constructor = Profile;

Profile.DEVICE_TYPE_KEYBOARD = 0;
Profile.DEVICE_TYPE_MOUSE    = 1;

Profile.MOUSE_WHEEL_NONE = 0;
Profile.MOUSE_WHEEL_V    = 1;
Profile.MOUSE_WHEEL_H    = 2;

Profile.MOUSE_MOVE_REL  = 0;
Profile.MOUSE_MOVE_ABS  = 1;


Profile.prototype.initialize = function() {
  this.initMembers();
}

Profile.prototype.initMembers = function() {
}

Profile.prototype.core = function() {
  return Core;
}

Profile.prototype.handleInterception = function(keyCode, keyDown, keyE0, hwid, deviceType, mouseWheel, mouseMove, x, y) {
  if(keyDown) {
    if(deviceType === Profile.DEVICE_TYPE_KEYBOARD) {
      var key = Input.indexToString(keyCode, keyE0);
      if(key === "escape") close();
      if(key === "q") {
        Core.send("mousewheel", true, 0, 100);
      }
    }
  }
  if(deviceType === Profile.DEVICE_TYPE_MOUSE) {
    if(keyCode > 0) {
      if(mouseWheel === Profile.MOUSE_WHEEL_V) {
        console.log("y" + y.toString());
      }
      else if(mouseWheel === Profile.MOUSE_WHEEL_H) {
        console.log("x" + x.toString());
      }
      else {
        console.log(Input.mouseIndexToString(keyCode));
      }
    }
    else {
      console.log(x.toString() + "," + y.toString());
    }
  }
  Core.send_default();
}
