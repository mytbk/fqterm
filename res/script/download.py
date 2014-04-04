# coding=UTF-8
"""
download the directory

	13/09/04	kingson
	* add this changelog
	
Notes:
	1. make sure the path is empty, otherwise mkdir complain and stoped
	2. the time for sleep() varies dependent on sites
	3. tested for FireBird BBS only
Bugs:
	1.
TODO:
	1. add some GUI for control, e.g. stop, pause, resume,  inside script
	2. make it robust
"""

import fqterm
import sys,os,string,time,re

# the pointer to fqtermWindow object
lp=long(sys.argv[0])

wait_time = 2.0

def txt2html(txt):
	txt=string.replace(txt,"\n","\n<br>")
	return string.replace(txt," ","&nbsp;")
	
def upper_dir(path):
	if(sys.platform=="win32"):	# '\' for win32
		i=path.rfind("\\",0,-1)
	else:
		i=path.rfind("/",0,-1)	# '/' for *nix
	return path[:i+1]

def lower_dir(path, subdir):
	if(sys.platform=="win32"):	# '\' for win32
		return path+subdir+"\\"
	else:
		return path+subdir+"/"	# '/' for *nix

def write_html_header(hfile, num):
	hfile.write("""<html><head>
		<meta http-equiv="Content-Language" content="zh-cn">
		<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
		<title>FQTerm Article Downloader</title>
		</head>""")
	hfile.write("<body>")
	hfile.write("<p><b><h1>FQTerm Article Downloader</h1></b></p>")
	txt="""<p><p align=center><a href=%d.html>Prevoius</a>     
		<a href=index.html>Index</a>      
		<a href=%d.html>Next</a></p align=center></p>""" % (num-1,num+1)
	hfile.write(txt)
	hfile.write("<hr><p></p>\n")

def write_html_ender(hfile,num):
	hfile.write("<hr><p></p>")
	txt="""<p><p align=center><a href=%d.html>Prevoius</a>     
		<a href=index.html>Index</a>      
		<a href=%d.html>Next</a></p align=center></p>""" % (num-1,num+1)
	hfile.write(txt)
	hfile.write("<p><b>FQTerm --- BBS client based on Qt library</b><p>")
	hfile.write("""<p><a href=http://code.google.com/p/fqterm>
				http://code.google.com/p/fqterm</a><p>""")
	hfile.write("</body>")
	hfile.write("</html>")

def write_index_header(hfile):
	hfile.write("""<html><head>
		<meta http-equiv="Content-Language" content="zh-cn">
		<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
		<title>FQTerm Article Downloader</title>
		</head>""")
	hfile.write("<p><b><h1>FQTerm Article Downloader</h1></b></p>")
	hfile.write("""<p><p align=center>
				<a href=\"../index.html\">Up</a>
				</p align=center</p>""")
	hfile.write("<hr><p></p>\n")

def write_index_ender(hfile):
	hfile.write("<hr><p></p>")
	hfile.write("""<p><p align=center>
				<a href=\"../index.html\">Up</a>
				</p align=center</p>""")
	hfile.write("<p><b>FQTerm --- BBS client based on Qt library</b><p>")
	hfile.write("""<p><a href=http://code.google.com/p/fqterm>
				http://code.google.com/p/fqterm</a><p>""")
	hfile.write("</body>")
	hfile.write("</html>")

def get_list_num(str_line):
	# get the number
	matchobj = re.search("[0-9]+",str_line)
	if(matchobj==None):
		# wrong format
		return None
	else:
	   return str_line[matchobj.start():matchobj.end()]

def get_list_categary(str_line):
	# get the categary
	matchobj = re.search("\[[^0-9]{6}\]",str_line)
	if(matchobj==None):
		# wrong format
		return None
	else:
		return str_line[matchobj.start():matchobj.end()]

def get_list_title(str_line):
	matchobj = re.search("\[[^0-9]{6}\]",str_line)
	if(matchobj==None):
		# wrong format
		return None
	else:
		# get the title	
		return  str_line[matchobj.end()+1:]

def down_folder():
	global path
	first_num = -1
	while(1):
		line=fqterm.caretY(lp) 
		str_line=fqterm.getText(lp,line)
	
		article_num = get_list_num(str_line)
		if (first_num == -1):
			first_num = article_num
		if(article_num==None):
			print "Wrong format list"
			# end the index.html
			f=open(path+"index.html","a+")
			write_index_ender(f)
			f.close()
			# wrong formated list, leave out
			print "leave %s" % path
			fqterm.sendString(lp,'q')
			path=upper_dir(path)
			time.sleep(wait_time)
			return	

		article_categary = get_list_categary(str_line)
		article_title = get_list_title(str_line)	

		# download if its file
		if article_categary == '[文件]':
			# log in index.html
			f=open(path+"index.html","a+")
			f.write("<p><a href="+article_num+".html>")
			f.write("[文件] "+article_title+"</a></p>\n")
			f.close()
			# downlaod and save article 
			f=open(path+article_num+".html","w")
			fqterm.sendString(lp,'r')
			time.sleep(wait_time)
			write_html_header(f,int(article_num))
			f.write(txt2html(fqterm.getArticle(lp, 100)[0]))
			f.write("\n")
			write_html_ender(f,int(article_num))
			f.close()
			time.sleep(wait_time)
			fqterm.sendString(lp,"q")
			time.sleep(wait_time)
		# recursive callback if its directory
		elif article_categary == '[目录]':
			# log in index.html
			f=open(path+"index.html","a+")
			f.write("<p><a href="+article_num+"/index.html>")
			f.write("[目录] "+article_title+"</a></p>\n")
			f.close()
			# make dir and enter
			path=lower_dir(path,article_num)
			os.mkdir(path)
			print "enter %s" % path
			# create index.html and write the header
			f=open(path+"index.html","w")
			write_index_header(f)
			f.close()
			# recursive call
			fqterm.sendString(lp,'r')
			time.sleep(wait_time)
			# increase delay when condition not met
			down_folder()
		else:
			print "Unrecognized Categary"
			# end the index.html
			f=open(path+"index.html","a+")
			write_index_ender(f)
			f.close()
			# wrong formated list, leave out
			print "leave %s" % path
			fqterm.sendString(lp,'q')
			path=upper_dir(path)
			time.sleep(wait_time)
			return	
		str_next=fqterm.getText(lp,line+1)
		# move cursor down and get the num
		fqterm.sendString(lp,'j')
		time.sleep(wait_time)
		str_next=fqterm.getText(lp,fqterm.caretY(lp))
		article_num_next = get_list_num(str_next)
		if(article_num_next==None):
			print "Wrong format list"
			# end the index.html
			f=open(path+"index.html","a+")
			write_index_ender(f)
			f.close()
			# wrong formated list, leave out
			print "leave %s" % path
			fqterm.sendString(lp,'q')
			path=upper_dir(path)
			time.sleep(wait_time)
			return	
		# exit current dir when the number of the next one equals or small then last
		# this may cause problem when the server is extra slow 
		if(int(article_num_next) <= int(article_num)):
			#print "leaving, because cur: %s < pre: %s" % (article_num_next , article_num)
			if(path!=path_dir):
				# end the index.html
				f=open(path+"index.html","a+")
				write_index_ender(f)
				f.close()
				# leave out
				print "leave %s" % path
				fqterm.sendString(lp,'q')
				path=upper_dir(path)
				time.sleep(wait_time)
			return

# NOTE: make sure path ended with '/'(*nix) or '\\'(windowz)
#path_dir=path="e:\\temp\\test\\"
path_dir=path="/home/dp2/temp/test/"

# try to save all to home dir
# path_dir=path=os.environ['HOME']+"/.fqterm/downloads/"+time.ctime()+"/"
os.makedirs(path)

# enter
fqterm.sendString(lp,'x') 
time.sleep(wait_time)
# create index.html and write the header
f=open(path+"index.html","w")
write_index_header(f)
f.close()
down_folder()
# end the index.html
f=open(path+"index.html","a+")
write_index_ender(f)
f.close()
# exit
fqterm.sendString(lp,'q')

