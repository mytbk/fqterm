# coding=UTF-8
"""
This information is used to track the changes following this format
	dd/mm/yy	Author
		* changes
		* ...

	10/09/04	kingson
		* merge code for ytht.net preview by cppgx
	08/09/04	kingson
		* add this changelog information
"""
import fqterm, string
import re, urllib

"""
get url from ytht
"""

def previewYTHT(lp):
	URL=fqterm.getURL(lp)
	if(URL=='' or URL==None):
		return
	if re.search('http://ytht.net/Ytht.Net' \
		'(\S+)/con\?B=(\d+)&F=M\.(\d+)\.A', URL) != None:
		# ytht artical URL
		print 'Analizing ytht artical URL'
		f_con = urllib.urlopen(URL)
		con = f_con.read()
		f_con.close()
		m_con = re.search('src=\"http://162.105.31.(\d+)(/|:(\d+)/)' \
			'Ytht.Net/boards/(\d+)/M\.(\d+)\.A\+\d+\"', con)
		if m_con != None:
			URL1 = con[(m_con.start() + 5):(m_con.end() - 1)]
			print 'URL1 = %s' % URL1
			f_con1 = urllib.urlopen(URL1)
			con1_lines = f_con1.readlines()
			f_con1.close()
			x = range(0, len(con1_lines))
			for i in x:
				m_con1 = re.search('<a href=\'http://162.105.31.(\d+)' \
				'(/|:(\d+)/)Ytht.Net/attach/bbscon/(\S+)\?B=(\d+)&amp;' \
				'F=M\.(\d+)\.A&amp;attachpos=(\d+)&amp;attachname=/(\S+)\'', \
				con1_lines[i])
				if m_con1 != None:
					URL2 = con1_lines[i][(m_con1.start() + 9): \
					(m_con1.end() - 1)].replace('&amp;', '&')
					print 'URL2 = %s' % URL2
					fqterm.previewImage(lp, URL2)

"""
this is called when beep received
"""
def onBell(lp):
	pass

"""
this is called when ready to autoreply
"""
def autoReply(lp):
	reply = "I am the auto replier, please wait..."
	reply_key = fqterm.getReplyKey(lp)
	if(reply_key==''):
		fqterm.sendParsedString(lp,"^Z")
	else:
		fqterm.sendParsedString(lp,reply_key)
	fqterm.sendString(lp,reply)
	fqterm.sendParsedString(lp,"^M")

"""
this is called when no activity after certain seconds, 
which is set in AddressBook dialog
"""
def antiIdle(lp):
	fqterm.sendParsedString(lp,"^@")
	print "antiIdle"

"""
whenever there is a mouse event
type	0-press  1-release  2-move  3-double click 4-wheel
state	0x01-left  0x02-right  0x04-middle 0x08-alt  0x10-control 0x20-shift
cx/cy	cursor position x/y in character
"""
def mouseEvent(lp, type, state, x, y, delta):
	# left click + control to preview image
	if type==0 and state==0x11:
		previewYTHT(lp)

"""
whenever there is a key event
type	0-press 1-release
state	0x08-alt  0x10-control 0x20-shift
key		refer to Qt/Doc
"""
def keyEvent(lp, type, state, key):
	pass

"""
whenever there is data from server after decoding
and displaying
"""
def dataEvent(lp):
	pass
