# coding=UTF-8
# various tools

# Left Arrow <--
def sendLeft(fqterm,lp):
	fqterm.sendParsedString(lp,"^[[C")
# Left Arrow -->
def sendRight(fqterm,lp):
	fqterm.sendParsedString(lp,"^[[D")
# Up Arrow  
def sendUp(fqterm,lp):
	fqterm.sendParsedString(lp,"^[[A")
# Down Arrow  
def sendDown(fqterm,lp):
	fqterm.sendParsedString(lp,"^[[B")
# Page Up
def sendPageUp(fqterm,lp):
	fqterm.sendParsedString(lp,"^[[5")
# Page Down
def sendPageDown(fqterm,lp):
	fqterm.sendParsedString(lp,"^[[6")
# Enter 
def sendDown(fqterm,lp):
	fqterm.sendParsedString(lp,"^M")

# Page State
# Firebird BBS only
def getPageState(fqterm,lp):
	pageState="unknown"
	lastline=fqterm.getText(lp,fqterm.rows(lp)-1)
	if(lastline.find("阅读文章")!=-1 or
		lastline.find("阅读精华区")!=-1 or
		lastline.find("下面还有")!=-1 or
		lastline.find("回信")!=-1):
		pageState="reading"
	elif(lastline.find("时间")!=-1 and
		lastline.find("使用者")!=-1):
		pageState="list"

	return pageState

# analyze article list	
# return a dictionary
# {"num":xxx,"sign":"","id":"","date":"","title":""}
def getListInfo(text):
	lst=text.split()
	info["num"]=long(lst[0])
	info["sign"]=lst[1]
	info["id"]=lst[2]
	info["date"]=lst[3]+lst[4]
	info["title"]=lst[5]
	return info

# get the current user ID
# only for FireBird BBS
def getCurrentID(fqterm,lp):
	text = fqterm.getText(lp,fqterm.rows(lp)-1)
	if(text.find("使用者")==-1):
		return
	lst = string.split(text,"]")
	text = lst[2]
	lst = string.split(text,"[")
	return lst[1]


