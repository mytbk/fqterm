# coding=UTF-8
# 连续发文

import fqterm
import sys, string, random

sessionID=long(sys.argv[0])
#print "sessionID is %s"%ID

random.seed()
for j in range(13):
	i=random.random()*100000
	print i
	title='%f只青蛙%f张嘴'%(i,i)
	content='%f只眼睛%f条腿'%(2*i,4*i)
	#data=['^p', title, '\n', '\n', content, '^w','\n']
	fqterm.sendParsedString(sessionID, r'^p%s\n\n%s^w\n'%(title, content) )
	#for item in data:
	#	SendParsedString(ID, item)
	#	if dataEvents.has_key(ID):
	#		dataEvents[ID].wait()
