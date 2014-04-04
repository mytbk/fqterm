
var text = String(fqterm.getSelect(false));
var url = String(fqterm.getURL());

/*
var sharejs = '<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">\n' +
    '<html xmlns="http://www.w3.org/1999/xhtml">\n' + 
    '<head>\n' + 
    '<meta http-equiv="Content-type" content="text/html;charset=UTF-8">\n' +
    '</head>\n' +
    '<body onload="simulateclick()">\n' +
    "<script type=\"text/javascript\" charset=\"utf-8\">\n(function(){\nvar _w = 142 , _h = 32;" +
    "var param = {\nurl:location.href,\n" +
    "type:'4',\n" +
    "count:'0', \n" +
    "appkey:'',\n" +
    "title:'%1',\n" +
    "pic:'%2',\n" +
    "ralateUid:'2868108420',\n" +
    "language:'zh_cn', \n" +
    "rnd:new Date().valueOf()\n}\n" +
    "var temp = [];\nfor( var p in param ){\ntemp.push(p + '=' + encodeURIComponent( param[p] || '' ) )\n}\n" +
    "document.write('<iframe allowTransparency=\"true\" frameborder=\"0\" scrolling=\"no\" src=\"http://hits.sinajs.cn/A1/weiboshare.html?' + temp.join('&') + '\"" +
    " width=\"'+ _w+'\" height=\"'+_h+'\"></iframe>')\n})()\n";
    sharejs = sharejs + " </script>\n </body>\n </html>\n";
    fqterm.writeFile("tmp.html", sharejs);
*/

var picUrl = "";

var pattern = new RegExp('.+\\.(png|jpg|gif|bmp)$', 'i');
if (pattern.test(url)) {
    picUrl = url;
} else {
    text = text + ' ' + url;
}

shareurl = 'http://v.t.sina.com.cn/share/share.php?title=' + encodeURIComponent(removeSpecial(text));
shareurl= shareurl + '&pic=' + encodeURIComponent(picUrl);
shareurl= shareurl + '&ralateUid=2868108420&url=' + encodeURIComponent("D:\Test\test.html");


fqterm.openUrl(shareurl);
function removeSpecial(str) {
   var res = str.replace(/"/g, '\\\"');
   res = res.replace(/'/g, '\\\'');
   res = res.replace(/\\/g, '\\\\');
   res = res.replace(/\n/g, ' ');
   res = res.replace(/\r/g, '');
   return res;
}
