FQTerm
======
This is a terminal simulator developed for telnet based online forum.

Originally forked from Qterm-Qt3, rewritten and ported to Qt4 by Curvlet and dp2 since 2008.

Now I'm porting FQTerm to Qt5.

TODO
====
- ~~Bug fix: Unable to use Ctrl key (FIXED using the code for Mac OS)~~
- Bug fix: SIGSEGV on exit (Qt5)
- Maybe something wrong with imageviewer (Qt5)
- Reconstruct the code
- Some stinky things with wide characters (Qt5)

Install
=======
First clone the code.
```
git clone https://github.com/mytbk/fqterm.git
```
If you want to use the Qt5 branch, just add ```--branch Qt5``` as
follows.
```
git clone https://github.com/mytbk/fqterm.git --branch Qt5
```
Then build FQTerm.

```
mkdir build
cd build
cmake ../fqterm
make
as-root make install
```

Windows Builds
==============
- Cross compile on Linux with MXE: builds but fails at runtime
- MinGW on Windows: builds and runs with shared Qt 4.8.6
- MSVC: not tested
