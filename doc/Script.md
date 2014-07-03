## FQTerm脚本
在FQTerm中，可以使用JavaScript语言扩展FQTerm的功能。FQTerm的脚本功能由QtScript提供。

### 脚本执行方法
点击菜单中的**脚本->运行**可运行脚本，快捷键为```F7```.**脚本->停止**可停止脚本的运行，快捷键为```F8```.

### FQTerm的样例脚本
FQTerm的包中自带部分脚本，可以作为参考，当然也可以使用。这些脚本位于```res/script```目录，Linux用户安装后会放在```/usr/share/FQTerm/script```.
- downloadart.js: 下载BBS上的文章，如果需要连ANSI控制字符一起下载，可以用downloadart_ansi.js
- simpleBatch.js: 简单的批处理，可以用于发送重复的按键序列

### FQTerm函数列表
除JavaScript自身的函数外，FQTerm的脚本引擎中提供了一些函数供用户使用。使用时需要增加**fqterm**前缀，如```fqterm.msgBox```.

#### 运行其他脚本
- importFile(filename): 运行另一脚本，常用于导入函数定义，filename支持相对路径(相对于脚本文件所在目录)

#### 用户界面
- msgBox(msg): 弹出一个窗口，内容为msg指定的字符串
- yesnoBox(msg): 弹出窗口，并有选择按钮，选择Yes时返回true,否则返回false
- artDialog(content): 弹出一个窗口，内有文本框，其内容为content指定的串，有保存按钮用于将里面的内容存为文件
- FileDialog(): 弹出文件选择窗口，返回选择的文件的文件名
- askDialog(title,question,default): 弹出一个问答框，标题为title,问题为question,默认回答为default

#### 获取窗口内容
- caretX(): 返回光标列坐标(最左一列为第0列)
- caretY(): 返回光标行坐标(最上一行为第0行)
- getText(line): 返回指定行的内容
- getAttrText(line): 返回指定行的内容，且包含ANSI控制字符序列以表示字符属性(如颜色，闪烁等)

注意: getText()和getAttrText()返回的串都不包含换行符

#### 发送内容
- sendString(str): 向窗口发送str指定的字符串
- sendParsedString(str): 向窗口发送str指定的案件序列，它和sendString()的区别在于可以将"^A","^["等序列转为Ctrl-A,Esc等

#### 文件
- readFolder(path): 返回path指定目录下的文件列表，结构为一个数组
- readFile(filename): 读取filename指定的文本文件，返回文件的内容
- writeFile(filename,str): 将str指定的串写入filename指定的文件
- appendFile(filename,str): 将str指定的串追加到filename指定的文件

#### 其他函数
在[src/fqterm/fqterm_scriptengine.cpp](../src/fqterm/fqterm_scriptengine.cpp)中有所有的FQTerm脚本函数。以下是老FQTerm文档中列出的其他函数，可供参考。

- sessionID=long(sys.argv[0])
- formatError(sessionID) 
get the traceback info
return string
- getArticle(sessionID, interval)
copy current article
return (string, success)
- copyArticle(sessionID)
copy current article (obsolete)
return string
- columns(sessionID)
screen width
return int
- rows(sessionID)
screen height
return int
- isConnected(sessionID)
connected to server or not
return int
- disconnect(sessionID)
disconnect from server
- reconnect(sessionID)
reconnect
- getBBSCodec(sessionID)
get the bbs encoding, GBK or Big5
return string
- getAddress(sessionID)
get the bbs address
return string
- getPort(sessionID)
get the bbs port number
return int
- getProtocol(sessionID)
get the bbs protocol, 0/1/2 TELNET/SSH1/SSH2
return int
- getReplyKey(sessionID)
get the key to reply messages
return string (wtf...?)
- getURL(sessionID)
get the url string under mouse (not sure if works)
return string
- previewImage(sessionID, url)
preview the image link
- fromUTF8(str, codec)
decode from utf8 to string in specified codec
return string
- toUTF8(str, codec)
decode from string in specified codec to utf8
return string

