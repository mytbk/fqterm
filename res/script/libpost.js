fqterm.importFile("utils.js");
var timeout=100;

postID=function(){
	var thisLine = fqterm.getText(fqterm.caretY());
	var reg = new RegExp('[0-9]{1,}');
	return reg.exec(thisLine);
}

lastPost=function(){
	fqterm.sendString("$"); //last post
	sleep(timeout);
	return postID();
}

isBoard=function(){
	var secondLine = fqterm.getAttrText(1);
	var charString = "Ctrl-P";
	var myReg = new RegExp(charString);
	return myReg.exec(secondLine)==charString;
}

postAction=function(){
	if (isBoard()){
		fqterm.sendParsedString("^p");
	}else{
		fqterm.sendString("a");
	}
}

postEmpty=function(title){
    // postAction() is now deprecated
    if (isBoard()){
        fqterm.sendParsedString("^p");
        fqterm.sendString(title+"\n0\n\n"); // no qmd
    }else{
        fqterm.sendString("a"+title+"\n");
    }
    sleep(timeout);
    fqterm.sendParsedString("^W\n");
    sleep(timeout);
}

pastefile=function(filename){
	var content = fqterm.readFile(filename);
	for (var j = 0; j < content.length; ++j)
	{
		if (content[j] != '\033')
			fqterm.sendString(content[j]);
		else
			fqterm.sendParsedString("^[^[");
	}			
	fqterm.sendParsedString("^W\n");
	sleep(timeout);
}

copyline=function(){
	var line=fqterm.caretY();
	var filename = "/tmp/test.txt";
	fqterm.writeFile(filename, "");
	for (var i=line-1;i<=line+1;++i){
		var origline=fqterm.getAttrText(i);
		var reg = new RegExp('\x1b\x1b','g');
		var ANSIStr = origline.replace(reg,'\x1b[');
		fqterm.appendFile(filename, ANSIStr);
		fqterm.appendFile(filename, "\n");
	}
	return filename;
}
