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

注意：除 master 之外的分支都会在 master 的基础上 rebase 并用 ``git push --force`` 推上 GitHub.

## 安装

从0.9.9版本开始，Windows 版本的二进制包使用 GitHub releases 发布，请到 https://github.com/mytbk/fqterm/releases 下载 Windows 版本的二进制包。

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
cmake ../fqterm
make
# 以root身份安装，以下用sudo获取root权限
sudo make install
```

在 macOS (Sierra) 中编译，利用 HomeBrew 安装依赖。目前支持并不完善。
```shell
brew install openssl qt5
mkdir build
cd build
CMAKE_PREFIX_PATH=/usr/local/opt/qt5/lib/cmake \
OPENSSL_ROOT_DIR=/usr/local/opt/openssl \
cmake ../fqterm
make
make install
```

## Windows版本
- 在Linux下用MXE交叉编译，运行时错误，可能原因是MXE的Qt缺少语言Codec
- MinGW: 发布的Windows二进制包是在Windows 7下用MinGW编译的(静态连接的OpenSSL,Qt4,TDM GCC,构建方式请见项目wiki),编译方法如下  
```
REM 假设源码在C:\fqterm, OpenSSL在C:\openssl, Qt在C:\Qt\4.8.6.static
mkdir build
cd build
cmake -G "MinGW Makefiles" -DOPENSSL_ROOT_DIR=C:\openssl -DCMAKE_CXX_FLAGS=-mwindows -DCMAKE_BUILD_TYPE=Release -DQT_QMAKE_EXECUTABLE=C:\Qt\4.8.6.static\bin\qmake.exe C:\fqterm
mingw32-make
REM 生成的fqterm.exe即为程序文件
```
- MSVC: 经过测试可以使用MSVC2019，Qt5.15.2编译，方法如下
```
REM 假设Qt安装在C:\Qt目录
mkdir build
cd build
cmake.exe -DCMAKE_BUILD_TYPE=RelWithDebInfo -G"NMake Makefiles" -DCMAKE_PREFIX_PATH=C:\qt\5.15.2\msvc2019_64 -DOPENSSL_ROOT_DIR=C:\Qt\Tools\OpenSSL\Win_x64 C:\fqterm
nmake
```

也可以使用JOM加快编译速度

```
REM 假设Qt安装在C:\Qt目录
mkdir build
cd build
cmake.exe -DCMAKE_BUILD_TYPE=RelWithDebInfo -G"NMake Makefiles JOM" -DCMAKE_PREFIX_PATH=C:\qt\5.15.2\msvc2019_64 -DOPENSSL_ROOT_DIR=C:\Qt\Tools\OpenSSL\Win_x64 -DCMAKE_MAKE_PROGRAM=C:\Qt\Tools\QtCreator\bin\jom\jom.exe C:\fqterm
jom
```

## TODO
以下是FQTerm日后需要改进和修复的地方，希望大家参与开发。
- SSH: 建议使用系统的ssh，可参考 [doc/SSH.md](doc/SSH.md). 自带的SSH实现正在重构。
- 改善终端渲染
- 使用矢量UI
- ~~Bug: Qt5分支中Ctrl按键异常 (暂时使用Mac OS的处理方法解决)~~
- ~~Maybe something wrong with imageviewer (Qt5)~~ 准备删除imageviewer功能
- 考虑代码重构
- Bug: 处理不完整GBK字符时存在一些异常，特别是在Qt5分支

