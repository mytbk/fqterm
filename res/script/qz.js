fqterm.importFile("libpost.js");
fqterm.importFile("testz.js");

qz=function(){
    articleID = lastPost();
    ++articleID;
    var IDStr=isz(articleID);
    if (IDStr==-1){
	var cont = true;
	try
	{
	    if (fqterm.yesnoBox("似乎不是整哦，继续抢吗？")==false){
		cont = false;
	    }
	}
	catch(err)
	{
	}
	if (cont){
	    IDStr = articleID;
	}else{
	    return;
	}
    }
    postEmpty(IDStr);
    if (isBoard()){
	fqterm.sendString("j=_"); //focus to the new post
	sleep(500);
	myFile=copyline();
	fqterm.sendString("E"); //Edit the post
	pastefile(myFile);
    }
}

qz();

