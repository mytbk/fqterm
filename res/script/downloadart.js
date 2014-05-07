fqterm.importFile("utils.js");

// var saveFile = "/tmp/save.txt";
var LastLine = 23;
var timeout = 300;
var retries = 5;
// when the article is not completely read
// there'll be a 'XX%' at the last line
var r = new RegExp("%");
var esc = new RegExp('\x1b\x1b','g');
var content = "";

getLine = function(i){
    var line = fqterm.getText(i);
    return line+"\n";
}
    
// first copy the previous lines
for (var i=0; i<LastLine; ++i){
//    fqterm.appendFile(saveFile, getAnsiLine(i));
    content += getLine(i);
}

// then copy until article ends
while (1){
    var line = fqterm.getText(LastLine);
    if (!r.exec(line)){ // article ends
        break;
    }
    var prev = getLine(LastLine-1);
    var cur;
    fqterm.sendString("j");
    for (var i=0; i<retries; i++){
        var cur = getLine(LastLine-1);
        if (prev == cur){
            sleep(timeout);
        }
    }
//    fqterm.appendFile(saveFile, cur);
    content += cur;
}

fqterm.artDialog(content);

