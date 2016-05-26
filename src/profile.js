function Profile() {
  this.initialize.apply(this, arguments);
}

Profile.prototype.constructor = Profile;

Profile.prototype.initialize = function() {
  this.initMembers();
}

Profile.prototype.initMembers = function() {
}

Profile.prototype.core = function() {
  return Core;
}

Profile.prototype.handleInterception = function(keyCode, keyDown, keyE0, hwid) {
  // this.core().send_default();
  // this.core().send("b", keyDown);
  if(keyDown) {
    // console.log(keyCode);
    console.log(Input.indexToString(keyCode, keyE0));

    // if(keyCode === 59) {
    //   this.core().destroy();
    // }
  }
  // console.log(this.core());
  // this.core().send();
}
