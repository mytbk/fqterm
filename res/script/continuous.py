# This is demo script to continuousely download 4 articles
import fqterm
import sys, time

# f=open("/home/dp2/test.txt","w")
f = open("c:\\test.txt", "w")

lp=long(sys.argv[0])

for i in range(4):
	fqterm.sendString(lp,"r")
	time.sleep(1)
	f.write(fqterm.getArticle(lp, 100)[0])
	time.sleep(1)
	fqterm.sendString(lp,"q")
	time.sleep(1)
	fqterm.sendString(lp,"j")
	time.sleep(1)
f.close()
