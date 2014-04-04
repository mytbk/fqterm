fqterm.import("utils.js");
//NOTE: 1. this file should be in utf8
//      2. if you cannot enter "BMS", this script may fail.
//first enter the folder, then start script.
var timeout = 2000;
var path = "c:\\elite\\";   //DO NOT miss last slash
var base_path = path;
var newlinere;
if (fqterm.platform() == "Win") {
    newlinere = /%0D%0A/g;
} else if (fqterm.platform() == "Mac"){
    newlinere = /%0D/g;
} else {
    newlinere = /%0A/g;
}
newlinere = /\r\n|\n|\r/mg;
//var path = /home/dp2/elite/
var download = function(filename) {
    fqterm.sendString("r");
    sleep(timeout);
    var article = fqterm.copyArticle();
    fqterm.sendString("q");
    sleep(timeout);
    return article;
}

var make_html_header = function(num)
{
	num = parseInt(num);
	var result = 	'<html><head>\n'+
			'<meta http-equiv="Content-Language" content="zh-cn">\n'+
			'<meta http-equiv="Content-Type" content="text/html; charset=utf-8">\n'+
			'<title>FQTerm Article Downloader</title>\n'+
			'</head>\n'+
			'<body>\n'+
			'<p><b><h1>FQTerm Article Downloader</h1></b></p>\n'+
			'<p><p align=center><a href=' + (num - 1) + '.html>Prevoius</a>\n'+
		   	'<a href=index.html>Index</a>\n'+
			'<a href=' + (num + 1) + '.html>Next</a></p align=center></p>'+
			'<hr><p></p>\n';
	return result;
}

var make_html_ender = function(num)
{
	num = parseInt(num);
	var result = 	'<hr><p></p>\n'+
			'<p><p align=center><a href=' + (num - 1) + '.html>Prevoius</a>\n'+
			'<a href=index.html>Index</a>\n'+
			'<a href=' + (num + 1) + '.html>Next</a></p align=center></p>\n'+
			'<p><b>FQTerm --- BBS client based on Qt library</b><p>\n'+
			'<p><a href=http://code.google.com/p/fqterm>\n'+
			'http://code.google.com/p/fqterm</a><p>\n'+
			'</body>\n'+
			'</html>\n';
	return result;
}

var make_index_header = function() {
    var result = '<html><head>\n' +
		         '<meta http-equiv="Content-Language" content="zh-cn">\n' +
		         '<meta http-equiv="Content-Type" content="text/html; charset=utf-8">\n' +
		         '<title>FQTerm Article Downloader</title>\n' +
		         '</head>\n' +
		         '<p><b><h1>FQTerm Article Downloader</h1></b></p>\n' +
		         '<p><p align=center>\n' +
				 '<a href=\"../index.html\">Up</a>\n' +
				 '</p align=center</p>\n' +
				 '<hr><p></p>\n\n';
    return result;
}

var make_index_ender = function() {
    var result = '<hr><p></p>\n' +
                 '<p><p align=center>\n' +
				 '<a href=\"../index.html\">Up</a>\n' +
				 '</p align=center</p>\n' +
				 '<p><b>FQTerm --- BBS client based on Qt library</b><p>\n' +
				 '<p><a href=http://code.google.com/p/fqterm>\n' +
				 'http://code.google.com/p/fqterm</a><p>\n' +
				 '</body>\n' +
				 '</html>\n';
    return result;
}

var get_list_num = function(str_line) {
    // get the number
    try{
        var re = /[0-9]+/;
        var num = re.exec(str_line);
        return num[0];
    } catch(err) {
        return "";
    }
}

var get_list_categary = function(str_line) {
    try{
        var re = /\[[^0-9]{2}\]/; //utf8!
        var cat = re.exec(str_line);
        return cat[0];
    } catch(err) {
        return "";
    }
}

var get_list_title = function(str_line) {
    try{
        var re = /\[[^0-9]{2}\]/; //utf8!
        var cat = re.exec(str_line);
        var i = str_line.search(re);
        return str_line.substr(i + cat[0].length);
    } catch(err) {
        return "";
    }
}

var txt2html = function(txt) {
    var result = escape(txt);
    var re_nlchar;
    var newline;
    if(result.indexOf('%0D%0A') > -1){
       re_nlchar = /%0D%0A/g ;
       newline = "%0D%0A";
    }else if(result.indexOf('%0A') > -1) {
       re_nlchar = /%0A/g ;
       newline = "%0A";
    }else if(result.indexOf('%0D') > -1){
       re_nlchar = /%0D/g ;
       newline = "%0A";
    }
    result = result.replace(re_nlchar, newline + '<br />');
    result = result.replace(/%20/g, '&nbsp;');
    return unescape(result);
}

var upper_dir = function(path) {
    var spliter;
    if (fqterm.platform() == "Win") {
        spliter = '\\';
    } else {
        spliter = '/';
    }
    if (path.lastIndexOf(spliter) == path.length - 1) {
        path = path.substr(0, path.length - 1);
    }
    var i = path.lastIndexOf(spliter);

    return path.substr(0, i + 1);
}

var lower_dir = function(path, subdir) {
    var spliter;
    if (fqterm.platform() == "Win") {
        spliter = '\\';
    } else {
        spliter = '/';
    }
    if (path.lastIndexOf(spliter) != path.length - 1) {
        path += spliter;
    }
    return path + subdir + spliter;
}


var down_folder = function() {
    fqterm.makePath(path);
    var h = make_index_header();
    fqterm.writeFile(path + 'index.html', h);
    while (true) {
        var line = fqterm.caretY();
        var str_line = fqterm.getText(line);
        var article_num = get_list_num(str_line);
        var article_category = get_list_categary(str_line);
        var article_title = get_list_title(str_line);
        if (article_category == '[文件]') {
            var a = '<p><a href=' + article_num + '.html>[文件] ' + article_title + '</a></p>\n';
            fqterm.appendFile(path + "index.html", a);
            var content = make_html_header(article_num) + txt2html(download()) + '\n' + make_html_ender(article_num);
            fqterm.writeFile(path + article_num + ".html", content);
        } else if (article_category == '[目录]') {
            var a = '<p><a href=' + article_num + '/index.html>[目录] ' + article_title + '</a></p>\n';
            fqterm.appendFile(path + "index.html", a);
            path = lower_dir(path, article_num);
            fqterm.sendString('r');
            sleep(timeout);
            down_folder();
        } else {
            break;
        }
        fqterm.sendString('j');
        sleep(timeout);
        var str_next = fqterm.getText(fqterm.caretY());
        article_num_next = get_list_num(str_next);
        if (article_num_next == "" || parseInt(article_num_next) <= parseInt(article_num)) {
            break;
        }
    }
    var e = make_index_ender();
    fqterm.appendFile(path + 'index.html', e);
    fqterm.sendString('q');
    path = upper_dir(path);
    sleep(timeout);
}
//fqterm.sendString('x');
//sleep(timeout);
down_folder();
