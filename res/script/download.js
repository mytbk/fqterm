fqterm.import("utils.js");

var n = 4;
var timeout = 1000;
var filename = "c:\\test.txt";
fqterm.writeFile(filename, "")

while(n--)
{
	fqterm.sendString("r");
	sleep(timeout);
	var article = fqterm.copyArticle();
	fqterm.appendFile(filename, article)
	fqterm.sendString("q");
	sleep(timeout);
	fqterm.sendString("j");
	sleep(timeout);
}

