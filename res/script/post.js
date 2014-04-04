fqterm.import("utils.js");
var path = "c:\\temp\\"; //DO NOT miss last slash here.
var timeout = 500;
var filelist = fqterm.readFolder(path);
for (var i = 0; i < filelist.length; ++i)
{
	var filename = filelist[i];
	var content = fqterm.readFile(path + filename);
	fqterm.sendParsedString("^p");
	sleep(timeout);
	fqterm.sendString(filename);
	fqterm.sendString("\n\n");
	sleep(timeout);
	fqterm.sendString(content);
	fqterm.sendParsedString("^W\n");
	sleep(timeout);
}