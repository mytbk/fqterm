# coding=UTF-8

# 从硬盘文件连续发文
# for fqterm: we need to add some status check.

import fqterm
import tools
import sys, string, random, glob, os, time

ID=long(sys.argv[0])
#print "sessionID is %s"%ID

random.seed()
os.chdir(r'/home/dp2/') #主目录
filenames = glob.glob(r'*.txt') #过滤条件
#filenames.extend(  )
for n in filenames:
	print n
	if len(n)>=5 and n[0:5]=='album': #特例
		continue
	#if n[-4:len(n)]!='.txt':
	#	continue
	f=file(n, 'r')
	title=f.readline()
	title=title.strip()
	while title and title.strip()=='':
		title=f.readline()
		title=title.strip()
	if not title:
		continue
	content=f.read()
	
	if not title or not content:
		continue

	data=['\x10', title, '\n', '\n', content, '\x17']

	for item in data:
		fqterm.sendString(ID, item)
		print item
		#if dataEvents.has_key(ID):
			#dataEvents[ID].wait()
	fqterm.sendString(ID, '\n')
	#WaitFor(ID, SST_LIST)

	interval=random.random()*7+3
	#print interval
	time.sleep(interval)
