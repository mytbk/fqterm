如何添加一个新的功能
=====================

在FQTerm添加新的功能，可以参考 `3ddf947c <https://github.com/mytbk/fqterm/commit/3ddf947c8da82a6d2e40688583143005b632942e>`__, 它的功能是记录收到的（解密后的）数据包。以下以此为例，讲解添加新功能的方法。

首先在  src/common/fqterm_shortcuthelper.h 添加一个新项，在这里是 LOGRAW. 之后在 src/common/fqterm_shortcuthelper.cpp 中用 initShortcutDescriptionTableEntry 和 retranslateAction 函数注册此功能，包括它的文字描述、快捷键，图标等。如果这个功能需要图标，请在 res/pic 下建立一个图标文件，具体见 initShortcutDescriptionTableEntry 函数的源码。

在主窗口的源文件 src/fqterm/fqterm_frame.cpp 中，用::

  toolBarMdiConnectTools_->addAction(getAction(FQTermShortcutHelper::LOGRAW));

以及::

  FQTERM_ADDACTION(spec, LOGRAW, this, logRaw);

分别在工具栏和菜单中添加此功能。用 ``getAction(FQTermShortcutHelper::LOGRAW)->setEnabled(enable);`` 启用此功能。

此外， ``void FQTermFrame::updateMenuToolBar()`` 过程在窗口切换等操作发生时会被调用，由于 LOGRAW 功能是针对单个窗口的功能，在窗口切换时要改变它的选择状态，于是要在此过程中添加::

  getAction(FQTermShortcutHelper::LOGRAW)->setChecked(window->getSession()->isLogging());

此后，实现 ``void FQTermFrame::logRaw()`` 函数完成新功能的实现。
