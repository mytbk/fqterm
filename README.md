## FQTerm
FQTerm是为Telnet BBS设计的终端模拟器(俗称Term)，支持Telnet,SSH协议。    
本项目源自QTerm-Qt3,从2008年起，由Curvlet和dp2重写并移植至Qt4.    
现在FQTerm的原开发组成员已经停止了该项目的维护，现由archlinux(mytbk@GitHub)网友维护并移植至Qt5。    

## 文档
目前FQTerm还没有特别完整的文档，现在正在进行文档编写。用Markdown编写的文档请见[doc目录](doc/).

## 如何贡献源码
fork本项目，然后自己开发，提交自己的更改，再创建pull request.    
详情请看GitHub的帮助:
- https://help.github.com/articles/fork-a-repo
- https://help.github.com/articles/using-pull-requests

## 安装
Windows用户可以直接到[fosshub页面](http://code.fosshub.com/FQTerm/downloads)下载预编译的包。

鉴于此前SourceForge的一些事件，本项目的Windows预编译包已迁移至FossHub，原有的[SourceForge](http://sourceforge.net/projects/fqterm/files/windows/)上的包会保留。

ArchLinux用户可以使用[archlinuxcn仓库](https://wiki.archlinux.org/index.php/Unofficial_user_repositories#archlinuxcn)安装``fqterm-git``包，或者从AUR安装[fqterm-git](https://aur.archlinux.org/packages/fqterm-git/)或Qt5版本[fqterm-qt5-git](https://aur.archlinux.org/packages/fqterm-qt5-git/).

手动编译大致步骤如下:    
解决依赖: Qt(Qt5版本需要qt5-script,qt5-multimedia,qt5-tools), alsa(Linux)    
获取代码：      
```
git clone https://github.com/mytbk/fqterm.git
```

然后开始编译：      

```
# 假设FQTerm源码目录为fqterm
mkdir build
cd build
# 用cmake生成Makefile,默认为Qt4版本，Qt5版本请添加-DUSE_QT5=1参数
cmake ../fqterm
make
# 以root身份安装，以下用sudo获取root权限
sudo make install
```

在 macOS (Sierra) 中编译建议使用 Qt5 分支，利用 HomeBrew 安装依赖。目前支持并不完善。
```shell
brew install openssl qt5
mkdir build
cd build
CMAKE_PREFIX_PATH=/usr/local/opt/qt5/lib/cmake \
OPENSSL_ROOT_DIR=/usr/local/opt/openssl \
cmake ../fqterm -DUSE_QT5=1
make
make install
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
- SSH: 增加host key记录和认证机制
- SSH: public key auth
- 改善终端渲染
- 使用矢量UI
- ~~Bug: Qt5分支中Ctrl按键异常 (暂时使用Mac OS的处理方法解决)~~
- ~~Bug: Qt5分支中退出时SIGSEGV(resolved)~~
- ~~Maybe something wrong with imageviewer (Qt5)~~
- 考虑代码重构
- Bug: 处理不完整GBK字符时存在一些异常，特别是在Qt5分支

