## FQTerm
FQTerm是为Telnet BBS设计的终端模拟器(俗称Term)，支持Telnet,SSH协议。    
本项目源自QTerm-Qt3,从2008年起，由Curvlet和dp2重写并移植至Qt4.    
现在FQTerm的原开发组成员已经停止了该项目的维护，现由archlinux网友维护。    

## 文档
目前FQTerm还没有特别完整的文档，现在正在进行文档编写。用Markdown编写的文档请见[doc目录](doc/).

## 如何贡献源码
fork本项目，然后自己开发，提交自己的更改，再创建pull request.    
详情请看GitHub的帮助:
- https://help.github.com/articles/fork-a-repo
- https://help.github.com/articles/using-pull-requests

## 安装
Windows用户可以直接到[SourceForge页面](http://sourceforge.net/projects/fqterm/files/windows/)下载预编译的包。

ArchLinux用户可以直接从AUR安装，如
```
yaourt -S fqterm-git
```
或
```
# Qt5分支
yaourt -S fqterm-qt5-git
```

其他用户可手动编译，大致步骤如下:    
解决依赖: Qt, OpenSSL, alsa-lib(Linux)    
获取代码：      
```
git clone https://github.com/mytbk/fqterm.git
```
要用Qt5分支，可以在使用 ```--branch Qt5``` 参数，即：
```
git clone https://github.com/mytbk/fqterm.git --branch Qt5
```
然后开始编译：      

```
# 假设FQTerm源码目录为fqterm
mkdir build
cd build
cmake ../fqterm
make
# 以root身份安装，以下用sudo获取root权限
sudo make install
```

## Windows版本
- 在Linux下用MXE交叉编译，运行时错误，可能原因是MXE的Qt缺少语言Codec
- MinGW: 在[SourceForge](http://sourceforge.net/projects/fqterm/files/windows/)上的Windows预编译包是在MinGW下编译的(静态连接的OpenSSL,Qt4,TDM GCC,构建方式请见项目wiki),编译方法如下  
```
REM 假设源码在C:\fqterm, OpenSSL在C:\openssl, Qt在C:\Qt\4.8.6.static
mkdir build
cd build
cmake -G "MinGW Makefiles" -DOPENSSL_ROOT_DIR=C:\openssl -DCMAKE_CXX_FLAGS=-mwindows -DCMAKE_BUILD_TYPE=Release -DQT_QMAKE_EXECUTABLE=C:\Qt\4.8.6.static\bin\qmake.exe C:\fqterm
mingw32-make
REM 生成的fqterm.exe即为程序文件
```
- MSVC: 未测试

## TODO
以下是FQTerm日后需要改进和修复的地方，希望大家参与开发。
- wip: Qt4和Qt5分支的合并
- SSH: 增加host key记录和认证机制
- SSH: public key auth
- 改善终端渲染
- 使用矢量UI
- ~~Bug: Qt5分支中Ctrl按键异常 (暂时使用Mac OS的处理方法解决)~~
- ~~Bug: Qt5分支中退出时SIGSEGV(resolved)~~
- ~~Maybe something wrong with imageviewer (Qt5)~~
- 考虑代码重构
- Bug: 处理不完整GBK字符时存在一些异常，特别是在Qt5分支

