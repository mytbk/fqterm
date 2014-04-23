fqterm
======
This is a terminal simulator developed for telnet based online forum.

Originally forked from Qterm-Qt3, rewritten and ported to Qt4 by Curvlet and dp2 since 2008.

Now I'm porting FQTerm to Qt5.

TODO
====
- ~~Bug fix: Unable to use Ctrl key (FIXED using the code for Mac OS)~~
- Bug fix: SIGSEGV on exit
- Maybe something wrong with imageviewer
- Reconstruct the code

Install
=======
First clone the code, assume it's in fqterm/ directory, then run the following.

```
mkdir build
cd build
cmake ../fqterm
make
as-root make install
```

