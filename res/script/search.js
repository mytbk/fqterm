fqterm.searchSelected = function(engine) {
  
  var text = encodeURIComponent(String(fqterm.getSelect(false)));
  var url = "about:blank";
  engine = engine.toLowerCase();
  if (engine == "google") {
    searchUrl = "http://www.google.com/search?client=fqterm&rls=en&q=" + text + "&sourceid=fqterm";
  } else if (engine == "baidu") {
    searchUrl = "http://www.baidu.com/s?ie=utf-8&wd=" + text;
  } else if (engine == "bing") {
    searchUrl = "http://www.bing.com/search?q=" + text; 
  } else if (engine == "yahoo") {
    searchUrl = "http://search.yahoo.com/search?ei=UTF-8&p=" + text;
  } else if (engine == "custom") {
    fqterm.msgBox("Modify search.js to customize search engine...");
    return false;
  }else {
    return false;
  }
  fqterm.openUrl(searchUrl);
  return true;
}
