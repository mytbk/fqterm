# send current article to one assigned email address
# 1. need CJK codec 
import smtplib
from email.Header import Header
from email.Message import Message
import fqterm,sys,string

lp=long(sys.argv[0])
article=fqterm.getArticle(lp, 100)[0]

from_addr = "mime@email.com"
to_addr="yours@email.com"

subject = article.split("\n")
subject = subject[1]
subject = subject[7:]

mess = Message()
h = Header(subject,'GB2312')
mess['Subject']=h

msg = ("Content-Type: text/plain; charset= GB2312\r\n"
	"From: %s\r\nTo: %s\r\n%s\r\n\r\n"
	% (from_addr,to_addr, mess.as_string()))
msg = msg + article

smtp_host = "smtp.email.com"
server = smtplib.SMTP(smtp_host)
server.set_debuglevel(1)
#maybe your smtp need auth
#server.login(usr,pwd)
server.sendmail(from_addr,to_addr,msg)
server.quit()
