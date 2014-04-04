fqterm.import("utils.js");
fqterm.mouseEvent = function(type, states, x, y, delta) {
    if (type == fqterm.MOUSEEVENT.MOUSE_PRESS &&
        states & fqterm.BUTTONSTATE.LEFT_BUTTON && 
        states & fqterm.BUTTONSTATE.CTRL) {
        fqterm.msgBox(fqterm.getFullTextAt(y, x, 1));
    }
    return false;
}

fqterm.detectMenu = function() {
      fqterm.setMenuRect(1, 3, 20);
      return false;
}

fqterm.antiIdle = function() {
    //fqterm.msgBox("antiIdle");
    return false;
}


fqterm.onBell = function() {
    //fqterm.msgBox("onBell");
}

fqterm.autoReply = function() {
    //fqterm.msgBox("autoReply");
    return false;
}


fqterm.dataEvent = function() {
    //fqterm.msgBox("data event");
}

fqterm.keyEvent = function(type, states, key) {
//    if (type == fqterm.KEYEVENT.KEY_PRESS &&
//        states & fqterm.BUTTONSTATE.CTRL && key) {
//        fqterm.msgBox(key);
//    }
}  


var timeEvent = function() {
      //fqterm.msgBox("haha");
}
var id = fqterm.setInterval(1000, timeEvent);
//fqterm.clearInterval(id);