//script function names:
//antiIdle()     called on antiIdle, if this function exists, the embedded antiIdle procedure will not be called. (use fqterm.isAntiIdle() to get info.)
//onBell()       called on bell received. if this callback exists, the autoReply procedure will not be started. (use fqterm.isAutoReply() to get info.)
//autoReply()  (deprecated)  called on autoReply, if this callback exists, the embedded autoReply procedure will not be called 
//dataEvent()    called when contents in buffer changed. (after onBell, you will definitely receive one)
//keyEvent(type(key event type), states (OR flags), key (key code))     called when keyevent received.
//mouseEvent(type(mouse event type), states (OR flags), x, y, delta(for wheel))   called when mouseevent received. x, y are in term coordinates. If this callback exists, the embedded mouse/wheel support will be disabled (click to read article/enter menu), selecting/right click menu will still be there.
//detectMenu()   set menu rect here and return true to override the embedded menu selection rect. (call fqterm.setMenuRect()). and the mouse click on article, menu, ... will not take effect. you need to implement them in your mouse event.


sleep = function(ms) {
  var originInterval = fqterm.getUIEventInterval();
  fqterm.setUIEventInterval(1);
  var start = new Date().getTime();
  while(true) {
       if(new Date().getTime() - start > ms) break;
  }
  fqterm.setUIEventInterval(originInterval);
}


//script key event type
fqterm.KEYEVENT = {
    UNKNOWN       : -1,
    KEY_PRESS     : 0
}

//script mouse event type
fqterm.MOUSEEVENT = {
    UNKNOWN       : -1,
    MOUSE_PRESS   : 0,
    MOUSE_RELEASE : 1,
    MOUSE_MOVE    : 2,
    MOUSE_DBCLICK : 3,
    WHEEL         : 4
}

//script button state & key modifier
fqterm.BUTTONSTATE = {
    LEFT_BUTTON   : 0x01,
    RIGHT_BUTTON  : 0x02,
    MID_BUTTON    : 0x04,
    ALT           : 0x08,
    CTRL          : 0x10,
    SHIFT         : 0x20
}
