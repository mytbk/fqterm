/***************************************************************************
 *   fqterm, a terminal emulator for both BBS and *nix.                    *
 *   Copyright (C) 2008 fqterm development group.                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.               *
 ***************************************************************************/



#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <QClipboard>
#include <QDesktopServices>
#include <QDialog>
#include <QFileDialog>
#include <QFontDialog>
#include <QFontMetrics>
#include <QInputDialog>
#include <QImageReader>
#include <QMenu>
#include <QMessageBox>
#include <QStatusBar>
#include <QTextEdit>
#include <QTimer>
#include <QUrl>
#include <QtScript/QScriptValue>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QTemporaryFile>
#include <QRegExp>
#include <QNetworkProxy>
#include "addrdialog.h"
#include "articledialog.h"
#include "msgdialog.h"
#include "osdmessage.h"
#include "popwidget.h"
#include "progressBar.h"
#include "common.h"
#include "fqterm_buffer.h"
#include "fqterm_canvas.h"
#include "fqterm_config.h"
#include "fqterm_convert.h"
#include "fqterm_filedialog.h"
#include "fqterm_frame.h"
#include "fqterm_http.h"
#include "fqterm_ip_location.h"
#include "fqterm_param.h"
#include "fqterm_path.h"
#include "fqterm_screen.h"
#include "fqterm_session.h"
#include "fqterm_sound.h"
#include "fqterm_window.h"
#include "fqterm_wndmgr.h"
#include "imageviewer.h"
#include "sshlogindialog.h"
#include "statusBar.h"
#include "zmodemdialog.h"
#ifdef HAVE_PYTHON
#include <Python.h>
#include "fqterm_python.h"
#endif //HAVE_PYTHON
#include "fqterm_scriptengine.h"


namespace FQTerm {

char FQTermWindow::directions_[][5] =  {
  // 4
  "\x1b[1~",  // 0 HOME
  "\x1b[4~",  // 1 END
  "\x1b[5~",  // 2 PAGE UP
  "\x1b[6~",  // 3 PAGE DOWN
  // 3
  "\x1b[A",  // 4 UP
  "\x1b[B",  // 5 DOWN
  "\x1b[D",  // 6 LEFT
  "\x1b[C" // 7 RIGHT
};
/*
class FAQ: public QObject {
  Q_OBJECT;
 public slots:
  FAQ() {a = 100;}

  int get() {return a;}
  
 private:
  int a;
};
*/

//constructor
FQTermWindow::FQTermWindow(FQTermConfig *config, FQTermFrame *frame, FQTermParam param,
                           int addressIndex, QWidget *parent,
                           const char *name, Qt::WindowFlags wflags)
    : QMainWindow(parent, wflags),
      frame_(frame),
      isSelecting_(false),
      addressIndex_(addressIndex),
      sound_(NULL),
      isMouseClicked_(false),
      blinkStatus_(true),
      isUrlUnderLined_(false),
#ifdef HAVE_PYTHON
      // the system wide script
      pythonScriptLoaded_(false),
      // python thread module
      pModule(NULL),
      pDict(NULL),
#endif
      script_engine_(NULL),
      externalEditor_(NULL) {
  // isMouseX11_ = false;
  session_ = new FQTermSession(config, param);
  screen_ = new FQTermScreen(this, session_);

  config_ = config;
  zmodemDialog_ = new zmodemDialog(this);

  externalEditor_ = new FQTermExternalEditor(this);

  pageViewMessage_ = new PageViewMessage(this);
  tabBlinkTimer_ = new QTimer;

  setWindowIcon(QIcon("noicon"));
  setWindowTitle(param.name_);
  setMouseTracking(true);
  setFocusProxy(screen_);
  setCentralWidget(screen_);
  addMenu();
  setStatusBar(NULL);


  FQ_VERIFY(connect(pageViewMessage_, SIGNAL(hideAt(const QRect&)),
     screen_, SLOT(widgetHideAt(const QRect&))));

  FQ_VERIFY(connect(frame_, SIGNAL(bossColor()),
    screen_, SLOT(bossColor())));

  FQ_VERIFY(connect(frame_, SIGNAL(fontAntiAliasing(bool)),
    screen_, SLOT(setFontAntiAliasing(bool))));

  FQ_VERIFY(connect(frame_, SIGNAL(changeLanguage()),
                    this, SLOT(recreateMenu())));
  FQ_VERIFY(connect(frame_, SIGNAL(bossColor()),
                    screen_, SLOT(bossColor())));
  FQ_VERIFY(connect(frame_, SIGNAL(updateScroll()),
                    screen_, SLOT(updateScrollBar())));

  FQ_VERIFY(connect(screen_, SIGNAL(inputEvent(const QString&)),
                    session_, SLOT(handleInput(const QString&))));

  FQ_VERIFY(connect(session_, SIGNAL(zmodemStateChanged(int, int, const char*)),
                    this, SLOT(ZmodemState(int, int, const char *))));
  FQ_VERIFY(connect(zmodemDialog_, SIGNAL(canceled()),
                    session_, SLOT(cancelZmodem())));
  FQ_VERIFY(connect(session_, SIGNAL(connectionClosed()),
                    this, SLOT(connectionClosed())));
  FQ_VERIFY(connect(session_, SIGNAL(startAlert()), this, SLOT(startBlink())));
  FQ_VERIFY(connect(session_, SIGNAL(stopAlert()), this, SLOT(stopBlink())));
  // FQ_VERIFY(connect(session_->decoder_, SIGNAL(mouseMode(bool)),
  //                   this, SLOT(setMouseMode(bool))));
  FQ_VERIFY(connect(session_, SIGNAL(articleCopied(int, const QString)),
                    this, SLOT(articleCopied(int, const QString))));
  FQ_VERIFY(connect(session_, SIGNAL(requestUserPwd(QString*, QString*, bool*)),
                    this, SLOT(requestUserPwd(QString *, QString *, bool *))));
  //connect telnet signal to slots
  // QVERIFY(connect(session_->telnet_, SIGNAL(readyRead(int)),
  //                 this, SLOT(readReady(int))));
  FQ_VERIFY(connect(session_, SIGNAL(sessionUpdated()),
                    this, SLOT(sessionUpdated())));
  FQ_VERIFY(connect(session_, SIGNAL(bellReceived()), this, SLOT(beep())));
  FQ_VERIFY(connect(session_, SIGNAL(onTitleSet(const QString&)), this, SLOT(onTitleSet(const QString&))));

  FQ_VERIFY(connect(session_, SIGNAL(messageAutoReplied()),
                    this, SLOT(messageAutoReplied())));
  FQ_VERIFY(connect(session_, SIGNAL(telnetStateChanged(int)),
                    this, SLOT(TelnetState(int))));
  FQ_VERIFY(connect(session_, SIGNAL(errorMessage(QString)),
                    this, SLOT(showSessionErrorMessage(QString))));

  FQ_VERIFY(connect(tabBlinkTimer_, SIGNAL(timeout()), this, SLOT(blinkTab())));

  FQ_VERIFY(connect(externalEditor_, SIGNAL(done(const QString&)), this, SLOT(externalEditorDone(const QString&))));

  FQ_VERIFY(connect(this, SIGNAL(writeStringSignal(const QString&)), this, SLOT(writeString(const QString&)), Qt::QueuedConnection));
  FQ_VERIFY(connect(this, SIGNAL(writeRawStringSignal(const QString&)), this, SLOT(writeRawString(const QString&)), Qt::QueuedConnection));

#if defined(WIN32)
  popWindow_ = new popWidget(this, frame_);
#else
  popWindow_ = new popWidget(this);
#endif

  const QString &resource_dir = getPath(RESOURCE);

  cursors_[FQTermSession::kHome] = QCursor(
      QPixmap(resource_dir + "cursor/home.xpm"));
  cursors_[FQTermSession::kEnd] = QCursor(
      QPixmap(resource_dir + "cursor/end.xpm"));
  cursors_[FQTermSession::kPageUp] = QCursor(
      QPixmap(resource_dir + "cursor/pageup.xpm"));
  cursors_[FQTermSession::kPageDown] = QCursor(
      QPixmap(resource_dir + "cursor/pagedown.xpm"));
  cursors_[FQTermSession::kUp] = QCursor(
      QPixmap(resource_dir + "cursor/prev.xpm"));
  cursors_[FQTermSession::kDown] = QCursor(
      QPixmap(resource_dir + "cursor/next.xpm"));
  cursors_[FQTermSession::kLeft] = QCursor(
      QPixmap(resource_dir + "cursor/exit.xpm"), 0, 10);
  cursors_[FQTermSession::kRight] = QCursor(
      QPixmap(resource_dir + "cursor/hand.xpm"));
  cursors_[FQTermSession::kNormal] = Qt::ArrowCursor;

  script_engine_ = new FQTermScriptEngine(this);
  if  (session_->param().isAutoLoadScript_
       && !session_->param().autoLoadedScriptFileName_.isEmpty()) {
    const QString &filename = session_->param().autoLoadedScriptFileName_;
#ifdef HAVE_PYTHON
    if (filename.trimmed().endsWith(".py", Qt::CaseInsensitive)){
      initPython(filename);
    } else {
      initPython("");
#endif
      runScript(filename);    
#ifdef HAVE_PYTHON
    }
  } else {
    initPython("");
#endif //HAVE_PYTHON
  }
  session_->setScriptListener(this);
}

FQTermWindow::~FQTermWindow() {
#ifdef HAVE_PYTHON
  finalizePython();
#endif //HAVE_PYTHON
  script_engine_->finalizeScript();
  //  delete telnet_;
  delete session_;
  delete popWindow_;
  delete tabBlinkTimer_;
  delete menu_;
  delete urlMenu_;
  delete screen_;
  delete pageViewMessage_;
  //delete script_engine_;
}

void FQTermWindow::addMenu() {
  urlMenu_ = new QMenu(screen_);
  urlMenu_->addAction(tr("Preview image"), this, SLOT(previewLink()));
  urlMenu_->addAction(tr("Open link"), this, SLOT(openLink()));
  urlMenu_->addAction(tr("Save As..."), this, SLOT(saveLink()));
  urlMenu_->addAction(tr("Copy link address"), this, SLOT(copyLink()));
  urlMenu_->addSeparator();
  urlMenu_->addAction(tr("Share Selected Text and URL!"), this, SLOT(shareIt()));
  const QString &resource_dir = getPath(RESOURCE);

  menu_ = new QMenu(screen_);
#if defined(__APPLE__)
  // Please note that on MacOSX Qt::CTRL corresponds to Command key (apple key),
  // while Qt::Meta corresponds to Ctrl key.
  QKeySequence copy_shortcut(tr("Ctrl+C"));
  QKeySequence paste_shortcut(tr("Ctrl+V"));
#else
  QKeySequence copy_shortcut(tr("Ctrl+Insert"));
  QKeySequence paste_shortcut(tr("Shift+Insert"));
#endif
  menu_->addAction(QPixmap(resource_dir + "pic/copy.png"), tr("Copy"),
                   this, SLOT(copy()), copy_shortcut);
  menu_->addAction(QPixmap(resource_dir + "pic/paste.png"), tr("Paste"),
                   this, SLOT(paste()), paste_shortcut);

  menu_->addAction(QPixmap(resource_dir+"pic/get_article_fulltext.png"), tr("Copy Article"),
                   this, SLOT(copyArticle()));
  menu_->addSeparator();

  QMenu *fontMenu = new QMenu(menu_);
  fontMenu->setTitle(tr("Font"));
  fontMenu->setIcon(QPixmap(resource_dir + "pic/change_fonts.png"));
  for (int i = 0; i < 2; ++i) {
    QAction *act = fontMenu->addAction(
        FQTermParam::getLanguageName(bool(i)) + tr(" Font"),
        this, SLOT(setFont()));
    act->setData(i);
  }
  menu_->addMenu(fontMenu);

  menu_->addAction(QPixmap(resource_dir + "pic/ansi_color.png"), tr("Color"),
                   this, SLOT(setColor()));
  menu_->addSeparator();
  menu_->addAction(QPixmap(resource_dir + "pic/preferences.png"), tr("Setting"),
                   this, SLOT(setting()));
  menu_->addSeparator();
  menu_->addAction(tr("Open Selected As Url"), this, SLOT(openAsUrl()));
  menu_->addAction(tr("Search Selected Text!"), this, SLOT(searchIt()));
  menu_->addAction(tr("Share Selected Text and URL!"), this, SLOT(shareIt()));
}

void FQTermWindow::recreateMenu() {
  delete urlMenu_;
  delete menu_;
  addMenu();
}

//close event received
void FQTermWindow::closeEvent(QCloseEvent *clse) {
  bool toClose = true;
  if (isConnected() && FQTermPref::getInstance()->openWarnOnClose_) {
    QMessageBox mb(tr("FQTerm"),
                   tr("Still connected, do you really want to exit?"),
                   QMessageBox::Warning, QMessageBox::Yes|QMessageBox::Default,
                   QMessageBox::No | QMessageBox::Escape, 0, this);
    if (mb.exec() != QMessageBox::Yes) {
      toClose = false;
    }
  }
  if (toClose) {
    session_->close();
  } else {
    clse->ignore();
    return;
  }
  //We no longer save setting on close from r1036.
  //saveSetting();
}

void FQTermWindow::blinkTab() {
  emit blinkTheTab(this, blinkStatus_);
  blinkStatus_ = !blinkStatus_;
}

/* ------------------------------------------------------------------------ */
/*                                                                         */
/*                           Mouse & Key                                    */
/*                                                                          */
/* ------------------------------------------------------------------------ */
void FQTermWindow::enterEvent(QEvent*){}

void FQTermWindow::leaveEvent(QEvent*) {
  // TODO: code below doesn't work
  // QPoint temp(0, 0);
  // session_->setSelect(temp, temp, session_->isRectangleCopy_);
  // QApplication::postEvent(screen_, new QPaintEvent(QRect(0, 0, 0, 0));
  // &paintEvent);
}

void FQTermWindow::changeEvent(QEvent *e) {
  if (e->type() == QEvent::WindowStateChange) {
    QWindowStateChangeEvent *event = (QWindowStateChangeEvent*)(e);
    Qt::WindowStates oldState = event->oldState();
    if (!isMaximized()) {
      emit refreshOthers(this);
    }
    if ( (oldState & Qt::WindowMinimized && !isMinimized()) ||
         (!(oldState & Qt::WindowActive) && isActiveWindow())) {
      forcedRepaintScreen();
    }
    if (oldState & Qt::WindowMaximized || isMaximized()) {
      emit resizeSignal(this);
    }
  }
}

void FQTermWindow::resizeEvent(QResizeEvent *qResizeEvent)
{
  emit resizeSignal(this);
}

bool FQTermWindow::event(QEvent *qevent) {
  bool res = false;
  QKeyEvent *keyEvent;
  switch(qevent->type()) {
#ifdef __linux__ 
    case QEvent::ShortcutOverride:    
       keyEvent = (QKeyEvent *)(qevent);   
       if(keyEvent->key() == Qt::Key_W &&    
          keyEvent->modifiers() == Qt::ControlModifier) {    
         keyEvent->accept();   
         res = true;   
       }   
       break;
#endif
    case QEvent::KeyPress:
      keyEvent = (QKeyEvent *)(qevent);
      if (keyEvent->key() == Qt::Key_Tab ||
          keyEvent->key() == Qt::Key_Backtab) {
        // Key_Tab and Key_Backtab are special, if we don't process them here,
        // the default behavoir is to move focus. see QWidget::event.
        keyPressEvent(keyEvent);
        keyEvent->accept();
        res = true;
      }
      break;
    default:
      break;
  }

  res = res || QMainWindow::event(qevent);

  if (qevent->type() == QEvent::HoverMove
      || qevent->type() == QEvent::MouseMove
      || qevent->type() == QEvent::Move) {
    if (res) {
      FQ_TRACE("wndEvent", 10) << "Accept event: " << qevent->type()
                               << " " << getEventName(qevent->type()) << ".";
    } else {
      FQ_TRACE("wndEvent", 10) << "Ignore event: " << qevent->type()
                               << " " << getEventName(qevent->type()) << ".";
    }
  } else {
    if (res) {
      FQ_TRACE("wndEvent", 9) << "Accept event: " << qevent->type()
                              << " " << getEventName(qevent->type()) << ".";
    } else {
      FQ_TRACE("wndEvent", 9) << "Ignore event: " << qevent->type()
                              << " " << getEventName(qevent->type()) << ".";
    }
  }
  return res;
}

void FQTermWindow::mouseDoubleClickEvent(QMouseEvent *mouseevent) {
  if (scriptMouseEvent(mouseevent))
    return;
}

void FQTermWindow::mousePressEvent(QMouseEvent *mouseevent) {
  if (scriptMouseEvent(mouseevent))
    return;
  // stop  the tab blinking
  stopBlink();

  // Left Button for selecting
  if (mouseevent->button() & Qt::LeftButton && !(mouseevent->modifiers())) {
    startSelecting(mouseevent->pos());
  }

  // Right Button
  if ((mouseevent->button() & Qt::RightButton)) {
    if (mouseevent->modifiers() & Qt::ControlModifier) {
      // on Url
      if (!session_->getUrl().isEmpty()) {
        previewLink();
      }
      return ;
    }

#ifdef __APPLE__
    bool additional_modifier = (mouseevent->modifiers() & !Qt::MetaModifier);
#else
    bool additional_modifier = mouseevent->modifiers();
#endif

    if (!(additional_modifier)) {
      // on Url
      if (!session_->getUrl().isEmpty()) {
        urlMenu_->popup(mouseevent->globalPos());
      } else {
        menu_->popup(mouseevent->globalPos());
      }
      return ;
    }
  }
  // Middle Button for paste
  if (mouseevent->button() &Qt::MidButton && !(mouseevent->modifiers())) {
    if (isConnected())
      // on Url
      if (!session_->getUrl().isEmpty()) {
        previewLink();
      } else {
        pasteHelper(false);
      }
    return ;
  }

  // If it is a click, there should be a press event and a release event.
  isMouseClicked_ = true;

  // xterm mouse event
  //session_->sendMouseState(0, me->button(), me->state(), me->pos());
}


void FQTermWindow::mouseMoveEvent(QMouseEvent *mouseevent) {
  bool mouseEventConsumed = scriptMouseEvent(mouseevent);
  QPoint position = mouseevent->pos();
  if (!mouseEventConsumed)
  {

    // selecting by leftbutton
    if ((mouseevent->buttons() &Qt::LeftButton) && isSelecting_) {
      onSelecting(position);
    }
  }
  if (!(session_->param().isSupportMouse_ && isConnected())) {
    return;
  }

  setCursorPosition(position);
  setCursorType(position);


  if (!mouseEventConsumed)
  {
    if (!isUrlUnderLined_ && session_->urlStartPoint() != session_->urlEndPoint()) {
      isUrlUnderLined_ = true;
      urlStartPoint_ = session_->urlStartPoint();
      urlEndPoint_ = session_->urlEndPoint();
	    clientRect_ = QRect(QPoint(0, urlStartPoint_.y()), QSize(session_->getBuffer()->getNumColumns(), urlEndPoint_.y() - urlStartPoint_.y() + 1));
	    repaintScreen();
	  
    } else if (isUrlUnderLined_ && (session_->urlStartPoint() != urlStartPoint_ || session_->urlEndPoint() != urlEndPoint_)) {
      clientRect_ = QRect(QPoint(0, urlStartPoint_.y()), QSize(session_->getBuffer()->getNumColumns(), urlEndPoint_.y() - urlStartPoint_.y() + 1));
      urlStartPoint_ = QPoint();
      urlEndPoint_ = QPoint();
	    repaintScreen();
	    isUrlUnderLined_ = false;

    }
  }
}

static bool isSupportedImage(const QString &name) {
  static QList<QByteArray> image_formats =
      QImageReader::supportedImageFormats();

  return image_formats.contains(name.section(".", -1).toLower().toUtf8());
}

void FQTermWindow::mouseReleaseEvent(QMouseEvent *mouseevent) {
  if (scriptMouseEvent(mouseevent))
    return;
  if (!isMouseClicked_) {
    return ;
  }
  isMouseClicked_ = false;
  // other than Left Button ignored
  if (!(mouseevent->button() & Qt::LeftButton) ||
      (mouseevent->modifiers() & Qt::KeyboardModifierMask)) {
    // no local mouse event
    //session_->sendMouseState(3, me->button(), me->state(), me->pos());
    return ;
  }

  // Left Button for selecting
  QPoint currentMouseCell = screen_->mapToChar(mouseevent->pos());
  if (currentMouseCell != lastMouseCell_ && isSelecting_) {
    finishSelecting(mouseevent->pos());
    return ;
  }
  isSelecting_ = false;

  if (!session_->param().isSupportMouse_ || !isConnected()) {
    return ;
  }

  // url
  QString url = session_->getUrl();
  if (!url.isEmpty()) {
    if (isSupportedImage(url)) {
      previewLink();      
    } else {
      openUrl(url);
    }
    return ;
  }

  processLClick(currentMouseCell);
}

void FQTermWindow::wheelEvent(QWheelEvent *wheelevent) {
  if (scriptWheelEvent(wheelevent)) return;
  int j = wheelevent->delta() > 0 ? 4 : 5;
  if (!(wheelevent->modifiers())) {
    if (FQTermPref::getInstance()->isWheelSupported_ && isConnected()) {
      session_->writeStr(directions_[j]);
    }
    return ;
  }
  //session_->sendMouseState(j, Qt::NoButton, we->state(), we->pos());
}

//keyboard input event
void FQTermWindow::keyPressEvent(QKeyEvent *keyevent) {
  if (keyevent->isAutoRepeat() && !session_->getBuffer()->isAutoRepeatMode()) {
    FQ_TRACE("sendkey", 5)
        << "The terminal is set to not allow repeated key events.";
    keyevent->accept();
    return;
  }
  if (scriptKeyEvent(keyevent))
    return;

  keyevent->accept();

  if (!isConnected()) {
    if (keyevent->key() == Qt::Key_Return || keyevent->key() == Qt::Key_Enter) {
      session_->reconnect();
    } else if (keyevent->key() == Qt::Key_Space) {
        emit(connectionClosed(this));
    }
    return ;
  }

  // stop  the tab blinking
  stopBlink();
  if (!session_->readyForInput()) {
    return;
  }
  // message replying
  session_->leaveIdle();

  Qt::KeyboardModifiers modifier = QApplication::keyboardModifiers();
  int key = keyevent->key();
  sendKey(key, modifier, keyevent->text());
}
/*
void FQTermWindow::focusInEvent (QFocusEvent *event) {
  QMainWindow::focusInEvent(event);
  screen_->setFocus(Qt::OtherFocusReason);
}
*/

//connect slot
void FQTermWindow::connectHost() {
  pageViewMessage_->display(tr("Not connected"), screen_->mapToPixel(QPoint(1, 1)));
  session_->setProxy(session_->param().proxyType_,
                     session_->param().isAuthentation_,
                     session_->param().proxyHostName_,
                     session_->param().proxyPort_,
                     session_->param().proxyUserName_,
                     session_->param().proxyPassword_);
  session_->connectHost(session_->param().hostAddress_, session_->param().port_);
  config_->setItemValue("global", "lastaddrindex", QString("%1").arg(addressIndex_));
}

bool FQTermWindow::isConnected() {
  return session_->isConnected();
}

/* ------------------------------------------------------------------------ */
/*                                                                         */
/*                           Telnet State                                   */
/*                                                                          */
/* ------------------------------------------------------------------------ */

void FQTermWindow::sessionUpdated() {
  refreshScreen();
  //send a mouse move event to make mouse-related change
  QMouseEvent* me = new QMouseEvent(
      QEvent::MouseMove, mapFromGlobal(QCursor::pos()),
      QCursor::pos(), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
  QApplication::postEvent(this, me);
}

void FQTermWindow::requestUserPwd(QString *user, QString *pwd, bool *isOK) {
  SSHLoginDialog login(user, pwd, this);
  *isOK = (login.exec() == QDialog::Accepted);
}

void FQTermWindow::ZmodemState(int type, int value, const char *status) {
  QString strMsg;
  //to be completed
  switch (type) {
    case RcvByteCount:
    case SndByteCount:
      zmodemDialog_->setProgress(value);
      return;
    case RcvTimeout:
      /* receiver did not respond, aborting */
      strMsg = QString(tr("Time out!"));
      break;
    case SndTimeout:
      /* value is # of consecutive send timeouts */
      strMsg = QString(tr("Time out after trying %1 times")).arg(value);
      break;
    case RmtCancel:
      /* remote end has cancelled */
      strMsg = QString(tr("Canceled by remote peer %1")).arg(status);
      break;
    case ProtocolErr:
      /* protocol error has occurred, val=hdr */
      strMsg = QString(
          tr("Unhandled header %1 at state %2")).arg(value).arg(status);
      break;
    case RemoteMessage:
      /* message from remote end */
      strMsg = QString(tr("Msg from remote peer: %1")).arg(status);
      break;
    case DataErr:
      /* data error, val=error count */
      strMsg = QString(tr("Data errors %1")).arg(value);
      break;
    case FileErr:
      /* error writing file, val=errno */
      strMsg = QString(tr("Falied to write file"));
      break;
    case FileBegin:
      /* file transfer begins, str=name */
      FQ_TRACE("window", 1) << "Starting file " << status;
      zmodemDialog_->setFileInfo(session_->bbs2unicode(status), value);
      zmodemDialog_->setProgress(0);
      zmodemDialog_->clearErrorLog();
      zmodemDialog_->show();
      zmodemDialog_->setModal(true);
      return;
    case FileEnd:
      /* file transfer ends, str=name */
      zmodemDialog_->hide();
      return;
    case FileSkip:
      /* file being skipped, str=name */
      strMsg = QString(tr("Skipping file %1")).arg(status);
      break;
  }
  zmodemDialog_->addErrorLog(strMsg);
}

// telnet state slot
void FQTermWindow::TelnetState(int state) {
  switch (state) {
    case TSRESOLVING:
      pageViewMessage_->display(tr("Resolving host name"), screen_->mapToPixel(QPoint(1, 1)));
      break;
    case TSHOSTFOUND:
      pageViewMessage_->display(tr("Host found"), screen_->mapToPixel(QPoint(1, 1)));
      break;
    case TSHOSTNOTFOUND:
      pageViewMessage_->display(tr("Host not found"), screen_->mapToPixel(QPoint(1, 1)));
      break;
    case TSCONNECTING:
      pageViewMessage_->display(tr("Connecting..."), screen_->mapToPixel(QPoint(1, 1)));
      break;
    case TSHOSTCONNECTED:
      pageViewMessage_->display(tr("Connected"), screen_->mapToPixel(QPoint(1, 1)));
      frame_->updateMenuToolBar();
      break;
    case TSPROXYCONNECTED:
      pageViewMessage_->display(tr("Connected to proxy"), screen_->mapToPixel(QPoint(1, 1)));
      break;
    case TSPROXYAUTH:
      pageViewMessage_->display(tr("Proxy authentation"), screen_->mapToPixel(QPoint(1, 1)));
      break;
    case TSPROXYFAIL:
      pageViewMessage_->display(tr("Proxy failed"), screen_->mapToPixel(QPoint(1, 1)));
      break;
    case TSREFUSED:
      pageViewMessage_->display(tr("Connection refused"), screen_->mapToPixel(QPoint(1, 1)));
      break;
    case TSREADERROR:
      pageViewMessage_->display(tr("Error when reading from server"), screen_->mapToPixel(QPoint(1, 1)), 
                                PageViewMessage::Error);
      break;
    case TSCLOSED:
      pageViewMessage_->display(tr("Connection closed"), screen_->mapToPixel(QPoint(1, 1)));
      break;
    case TSCLOSEFINISH:
      pageViewMessage_->display(tr("Connection close finished"), screen_->mapToPixel(QPoint(1, 1)));
      break;
    case TSCONNECTVIAPROXY:
      pageViewMessage_->display(tr("Connect to host via proxy"), screen_->mapToPixel(QPoint(1, 1)));
      break;
    case TSEGETHOSTBYNAME:
      pageViewMessage_->display(
          tr("Error in gethostbyname"), screen_->mapToPixel(QPoint(1, 1)), PageViewMessage::Error);
      break;
    case TSEINIWINSOCK:
      pageViewMessage_->display(
          tr("Error in startup winsock"), screen_->mapToPixel(QPoint(1, 1)), PageViewMessage::Error);
      break;
    case TSERROR:
      pageViewMessage_->display(
          tr("Error in connection"), screen_->mapToPixel(QPoint(1, 1)), PageViewMessage::Error);
      break;
    case TSPROXYERROR:
      pageViewMessage_->display(tr("Error in proxy"), screen_->mapToPixel(QPoint(1, 1)), PageViewMessage::Error);
      break;
    case TSWRITED:
      break;
  }
}

void FQTermWindow::showSessionErrorMessage(QString reason) {
  if (reason == tr("User Cancel")) {
    return;
  }
  QMessageBox::critical(this, tr("Session error"), reason);
}

/* ------------------------------------------------------------------------ */
/*                                                                         */
/*                           UI Slots                                       */
/*                                                                          */
/* ------------------------------------------------------------------------ */


void FQTermWindow::copy() {
  QClipboard *clipboard = QApplication::clipboard();

  QString selected_text = session_->getBuffer()->getTextSelected(
      session_->param().isRectSelect_,
      session_->param().isColorCopy_,
      parseString((const char*)session_->param().escapeString_.toLatin1()));

  QByteArray cstrText = session_->unicode2bbs(selected_text);

  // TODO_UTF16: avoid this replacement.
  if (!FQTermPref::getInstance()->escapeString_.isEmpty()) {
    cstrText.replace(
      parseString((const char*)session_->param().escapeString_.toLatin1()),
      parseString(FQTermPref::getInstance()->escapeString_.toLatin1()));
  }
  selected_text = session_->bbs2unicode(cstrText);

  // TODO_UTF16: there are three modes: Clipboard, Selection, FindBuffer.
  clipboard->setText(selected_text, QClipboard::Clipboard);
  clipboard->setText(selected_text, QClipboard::Selection);
}

void FQTermWindow::paste() {
  pasteHelper(true);
}

void FQTermWindow::writePasting(const QString& content)
{
  QByteArray cstrText = session_->unicode2bbs(content);

  // TODO_UTF16: avoid this replacement.
  if (!FQTermPref::getInstance()->escapeString_.isEmpty()) {
    cstrText.replace(
      parseString(FQTermPref::getInstance()->escapeString_.toLatin1()),
      parseString((const char*)session_->param().escapeString_.toLatin1()));
  }

  if (session_->param().isAutoWrap_) {
    // convert to unicode for word wrap
    QString strText;
    strText = session_->bbs2unicode(cstrText);
    // insert '\n' as needed
    for (uint i = 0; (long)i < strText.length(); i++) {
      uint j = i;
      uint k = 0, l = 0;
      while ((long)j < strText.length() && strText.at(j) != QChar('\n')) {
        if (FQTermPref::getInstance()->widthToWrapWord_ - (l - k) >= 0
          && FQTermPref::getInstance()->widthToWrapWord_ - (l - k) < 2) {
            strText.insert(j, QChar('\n'));
            k = l;
            j++;
            break;
        }
        // double byte or not
        if (strText.at(j).row() == '\0') {
          l++;
        } else {
          l += 2;
        }
        j++;
      }
      i = j;
    }

    cstrText = session_->unicode2bbs(strText);
  }

  session_->write(cstrText, cstrText.length());
}

void FQTermWindow::pasteHelper(bool clip) {
  if (!isConnected()) {
    return ;
  }

  // TODO_UTF16: there are three modes: Clipboard, Selection, FindBuffer.
  QClipboard *clipboard = QApplication::clipboard();

  // TODO_UTF16: what's the termFrame_->clipboardEncodingID_?
  /*
  if (termFrame_->clipboardEncodingID_ == 0) {
  if (clip) {
  cstrText = U2G(clipboard->text(QClipboard::Clipboard));
  }
  else {
  cstrText = U2G(clipboard->text(QClipboard::Selection));
  }

  if (session_->param().serverEncodingID_ == 1) {
  char *str = encodingConverter_.G2B(cstrText, cstrText.length());
  cstrText = str;
  delete [] str;
  }
  } else {
  if (clip) {
  cstrText = U2B(clipboard->text(QClipboard::Clipboard));
  } else {
  cstrText = U2B(clipboard->text(QClipboard::Selection));
  }

  if (session_->param().serverEncodingID_ == 0) {
  char *str = encodingConverter_.B2G(cstrText, cstrText.length());
  cstrText = str;
  delete [] str;
  }
  }
  */

  QString clipStr;

  if (clip) {
    clipStr = clipboard->text(QClipboard::Clipboard);
  } else {
    clipStr = clipboard->text(QClipboard::Selection);
  }

  writePasting(clipStr);
}

void FQTermWindow::openAsUrl() {
  QString selected_text = session_->getBuffer()->getTextSelected(
    session_->param().isRectSelect_,
    session_->param().isColorCopy_,
    parseString((const char*)session_->param().escapeString_.toLatin1()));
  openUrl(selected_text);
}

void FQTermWindow::searchIt()
{
  script_engine_->runScript(getPath(RESOURCE) + "script/search.js");
  QScriptValueList qvl;
  qvl << FQTermPref::getInstance()->searchEngine_;
  if (!script_engine_->scriptCallback("searchSelected", qvl)) {
    //fall back to google.
    QString selected_text = session_->getBuffer()->getTextSelected(
      session_->param().isRectSelect_,
      session_->param().isColorCopy_,
      parseString((const char*)session_->param().escapeString_.toLatin1()));
    QString searchUrl = "http://www.google.com/search?client=fqterm&rls=en&q=" + selected_text + "&sourceid=fqterm";
    QByteArray url = QUrl(searchUrl).toEncoded();
    openUrlImpl(url);
  }
}

void FQTermWindow::shareIt()
{
  script_engine_->runScript(getPath(RESOURCE) + "script/weiboshare.js");
}

void FQTermWindow::fastPost()
{
  QClipboard *clipboard = QApplication::clipboard();
  QString clipStr = clipboard->text(QClipboard::Clipboard);
  QString lineEnd("\n");
  QString postTitle = clipStr.left(clipStr.indexOf(lineEnd));

  sendParsedString("^p");
  writeRawString(postTitle);
  writeRawString(QString("\n\n"));
  writePasting(clipStr);
  sendParsedString("^W\n");
}


void FQTermWindow::externalEditor() {
  externalEditor_->start();
}


void FQTermWindow::externalEditorDone(const QString& str) {
  QByteArray rawStr = session_->unicode2bbs_smart(str);
  session_->write(rawStr, rawStr.length());
}


void FQTermWindow::copyArticle() {
  if (!isConnected()) {
    return ;
  }

  session_->copyArticle();
}

void FQTermWindow::setColor() {
  addrDialog set(this, session_->param());

  set.setCurrentTabIndex(addrDialog::Display);
  int res = set.exec();
  if (res != 0) {
    updateSetting(set.param());
    if (res == 2)
      saveSetting(false);
  }

}

void FQTermWindow::disconnect() {
  session_->close();

  connectionClosed();
}

void FQTermWindow::showIP(bool show) {
  pageViewMessage_->hide();
  if (!show) {
    return;
  }
  QString country, city;
  QString url = session_->getIP();
  FQTermIPLocation *ipLocation = FQTermIPLocation::getInstance();

  QRect screenRect = screen_->rect();
  QPoint messagePos;
  QRect globalIPRectangle = QRect(screen_->mapToRect(ipRectangle_));
  QFontMetrics fm(qApp->font());
  int charHeight = fm.height();

  int midLine = (screenRect.top() + screenRect.bottom()) / 2;
  int ipMidLine = (globalIPRectangle.top() + globalIPRectangle.bottom()) / 2;
  if (ipMidLine < midLine) {
    // "There is Plenty of Room at the Bottom." -- Feyman, 1959
    messagePos.setY(globalIPRectangle.bottom() + 0.618 * charHeight);
  } else {
    messagePos.setY(globalIPRectangle.top()
                    - 0.618 * charHeight
                    - pageViewMessage_->size().height());
  }

  QString displayText;
  if (ipLocation == NULL) {
    displayText = tr("IP database file does NOT exist");
  } else if (!ipLocation->getLocation(url, country, city)) {
    displayText = tr("Invalid IP");
  } else {
    displayText = country + " " + city;
  }

  QRect messageSize(pageViewMessage_->displayCheck(displayText,
                                                   PageViewMessage::Info));
  PageViewMessage::Alignment ali;
  if (messageSize.width() + globalIPRectangle.left() >= screenRect.right()) {
    //"But There is No Room at the Right" -- Curvelet, 2007
    messagePos.setX(globalIPRectangle.right());
    ali = PageViewMessage::TopRight;
  } else {
    messagePos.setX(globalIPRectangle.left());
    ali = PageViewMessage::TopLeft;
  }

  pageViewMessage_->display(
      displayText,
      PageViewMessage::Info,
      0, messagePos, ali);
}

void FQTermWindow::runScript() {
  // get the previous dir
  QStringList fileList;
  FQTermFileDialog fileDialog(config_);

  fileList = fileDialog.getOpenNames("Choose a Javascript file", "JavaScript File (*.js)");

  if (!fileList.isEmpty() && fileList.count() == 1) {
    runScript(fileList.at(0));
  }
}

void FQTermWindow::stopScript() {
  script_engine_->stopScript();
}

void FQTermWindow::viewMessages() {
  msgDialog msg(this);

  QByteArray dlgSize =
//      frame_->config()->getItemValue("global", "msgdialog").toLatin1();
    config_->getItemValue("global", "msgdialog").toLatin1();
  const char *dsize = dlgSize.constData();
  if (!dlgSize.isEmpty()) {
    int x, y, cx, cy;
    sscanf(dsize, "%d %d %d %d", &x, &y, &cx, &cy);
    msg.resize(QSize(cx, cy));
    msg.move(QPoint(x, y));
  } else {
    msg.resize(QSize(300, 500));
    msg.move(QPoint(20,20));
  }

  msg.setMessageText(allMessages_);
  msg.exec();

  QString strSize = QString("%1 %2 %3 %4").arg(msg.x()).arg(msg.y()).arg
                    (msg.width()).arg(msg.height());
//  frame_->config()->setItemValue("global", "msgdialog", strSize);
    config_->setItemValue("global", "msgdialog", strSize);
}

void FQTermWindow::setting() {
  addrDialog set(this, session_->param());
  int res = set.exec();
  if (res != 0) {
    updateSetting(set.param());
    if (res == 2)
      saveSetting(false);
  }
}

void FQTermWindow::toggleAntiIdle() {
  session_->setAntiIdle(!session_->isAntiIdle());
}

void FQTermWindow::toggleAutoReply() {
  session_->setAutoReply(!session_->isAutoReply());
}

void FQTermWindow::toggleAutoReconnect() {
  session_->setAutoReconnect(!session_->param().isAutoReconnect_);
}

void FQTermWindow::connectionClosed() {
  stopBlink();

  pageViewMessage_->display(tr("Connection closed"), screen_->mapToPixel(QPoint(1, 1)));

  frame_->updateMenuToolBar();

  setCursor(cursors_[FQTermSession::kNormal]);

  refreshScreen();

  if (!getSession()->param().isAutoReconnect_ && getSession()->param().isAutoCloseWin_) {
    emit(connectionClosed(this));
  }
}

/* ------------------------------------------------------------------------ */
/*                                                                         */
/*                           Events                                         */
/*                                                                          */
/* ------------------------------------------------------------------------ */

void FQTermWindow::articleCopied(int e, const QString content) {
  if (e == DAE_FINISH) {
//    articleDialog article(frame_->config(), this);
    articleDialog article(config_, this);

    // Fix focus-losing bug.
    // dsize should be a pointer to a non-temprary object.
    QByteArray dlgSize =
//        frame_->config()->getItemValue("global", "articledialog").toLatin1();
      config_->getItemValue("global", "articledialog").toLatin1();

    const char *dsize = dlgSize.constData();

    if (!dlgSize.isEmpty()) {
      int x, y, cx, cy;
      sscanf(dsize, "%d %d %d %d", &x, &y, &cx, &cy);
      article.resize(QSize(cx, cy));
      article.move(QPoint(x, y));
    } else {
      article.resize(QSize(300, 500));
      article.move(20,20);
    }
    article.articleText_ = content;

    article.ui_.textBrowser->setPlainText(article.articleText_);
    article.exec();
    QString strSize = QString("%1 %2 %3 %4").arg(article.x()).arg(article.y())
                      .arg(article.width()).arg(article.height());
//    frame_->config()->setItemValue("global", "articledialog", strSize);
    config_->setItemValue("global", "articledialog", strSize);
  } else if (e == DAE_TIMEOUT) {
    QMessageBox::warning(this, "timeout", "download article timeout, aborted");
#ifdef HAVE_PYTHON
  } else if (e == PYE_ERROR) {
    QMessageBox::warning(this, "Python script error", pythonErrorMessage_);
  } else if (e == PYE_FINISH) {
    QMessageBox::information(this, "Python script finished",
                             "Python script file executed successfully");
#endif //HAVE_PYTHON          
  }
}

/* ------------------------------------------------------------------------ */
/*                                                                         */
/*                           Aux Func                                       */
/*                                                                          */
/* ------------------------------------------------------------------------ */

QByteArray FQTermWindow::parseString(const QByteArray &cstr, int *len) {
  QByteArray parsed = "";

  if (len != 0) {
    *len = 0;
  }

  for (uint i = 0; (long)i < cstr.length(); i++) {
    if (cstr.at(i) == '^') {
      i++;
      if ((long)i < cstr.length()) {
        parsed += FQTERM_CTRL(cstr.at(i));
        if (len != 0) {
          *len =  *len + 1;
        }
      }
    } else if (cstr.at(i) == '\\') {
        i++;
        if ((long)i < cstr.length()) {
          if (cstr.at(i) == 'n') {
            parsed += CHAR_CR;
          } else if (cstr.at(i) == 'r') {
            parsed += CHAR_LF;
          } else if (cstr.at(i) == 't') {
            parsed += CHAR_TAB;
          }
          if (len != 0) {
            *len =  *len + 1;
          }
        }
      } else {
        parsed += cstr.at(i);
        if (len != 0) {
          *len =  *len + 1;
        }
      }
  }

  return parsed;
}

void FQTermWindow::messageAutoReplied() {
  pageViewMessage_->display("You have messages", PageViewMessage::Info, 0);
}

void FQTermWindow::saveSetting(bool ask /* = true*/) {
  if (addressIndex_ == -1) {
    return ;
  }

  //save these options silently
  FQTermConfig pConf(getPath(USER_CONFIG) + "address.cfg");
  FQTermParam param;
  loadAddress(&pConf, addressIndex_, param);
  param.isAutoCopy_ = session_->param().isAutoCopy_;
  param.isAutoWrap_ = session_->param().isAutoWrap_;
  param.isRectSelect_ = session_->param().isRectSelect_;
  param.isColorCopy_ = session_->param().isColorCopy_;
  saveAddress(&pConf, addressIndex_, param);
  pConf.save(getPath(USER_CONFIG) + "address.cfg");

  if (param == session_->param()) {
    return;
  }
  
  QMessageBox mb("FQTerm", "Setting changed do you want to save it?",
                 QMessageBox::Warning, QMessageBox::Yes | QMessageBox::Default,
                 QMessageBox::No | QMessageBox::Escape, 0, this);
  if (!ask || mb.exec() == QMessageBox::Yes) {
    saveAddress(&pConf, addressIndex_, session_->param());
    pConf.save(getPath(USER_CONFIG) + "address.cfg");
  }
}

int FQTermWindow::externInput(const QByteArray &cstrText) {
  QByteArray cstrParsed = parseString(cstrText);
  return session_->write(cstrParsed, cstrParsed.length());
}

int FQTermWindow::externInput(const QString &cstrText) {
  return externInput(session_->unicode2bbs_smart(cstrText));
}

int FQTermWindow::writeRawString(const QString& str) {
  QByteArray rawStr = session_->unicode2bbs_smart(str);
  return session_->write(rawStr, rawStr.length());
}

void FQTermWindow::sendParsedString(const char *str) {
  int length;
  QByteArray cstr = parseString(str, &length);
  session_->write(cstr, length);
}


bool FQTermWindow::postQtScriptCallback(const QString& func, const QScriptValueList & args) {
  return script_engine_->scriptCallback(func, args);
}

#ifdef HAVE_PYTHON
bool FQTermWindow::postPythonCallback( const QString& func, PyObject* pArgs )
{
  return pythonCallback(func, pArgs);
}
#endif //HAVE_PYTHON

// void FQTermWindow::setMouseMode(bool on) {
//   isMouseX11_ = on;
// }

//  /* 0-left 1-middle 2-right 3-relsase 4/5-wheel
//   *
//   */
//  //void FQTermWindow::sendMouseState( int num,
//  //    Qt::ButtonState btnstate, Qt::ButtonState keystate, const QPoint& pt )
//  void FQTermWindow::sendMouseState(int num, Qt::KeyboardModifier btnstate,
//                           Qt::KeyboardModifier keystate, const QPoint &pt) {
//    /*
//      if(!m_bMouseX11)
//      return;
//
//      QPoint ptc = screen_->mapToChar(pt);
//
//      if(btnstate&Qt::LeftButton)
//      num = num==0?0:num;
//      else if(btnstate&Qt::MidButton)
//      num = num==0?1:num;
//      else if(btnstate&Qt::RightButton)
//      num = num==0?2:num;
//
//      int state = 0;
//      if(keystate&Qt::ShiftModifier)
//      state |= 0x04;
//      if(keystate&Qt::MetaModifier)
//      state |= 0x08;
//      if(keystate&Qt::ControlModifier)
//      state |= 0x10;
//
//      // normal buttons are passed as 0x20 + button,
//      // mouse wheel (buttons 4,5) as 0x5c + button
//      if(num>3) num += 0x3c;
//
//      char mouse[10];
//      sprintf(mouse, "\033[M%c%c%c",
//      num+state+0x20,
//      ptc.x()+1+0x20,
//      ptc.y()+1+0x20);
//      m_pTelnet->write( mouse, strlen(mouse) );
//    */
//  }

/* ------------------------------------------------------------------------ */
/*                                                                         */
/*                           Python Func                                    */
/*                                                                          */
/* ------------------------------------------------------------------------ */
#ifdef HAVE_PYTHON


void FQTermWindow::runPythonScript() {
  QStringList fileList;
  FQTermFileDialog fileDialog(config_);

  fileList = fileDialog.getOpenNames("Choose a Python file", "Python File (*.py)");

  if (!fileList.isEmpty() && fileList.count() == 1) {
    runPythonScriptFile(fileList.at(0));
  }
}

void FQTermWindow::runPythonScriptFile(const QString& file) {
  QString str(QString("%1").arg(long(this)));
	char* lp = fq_strdup(str.toUtf8().data());
	char *argv[2]={lp,NULL};
    // get the global python thread lock
    PyEval_AcquireLock();
    
    PyInterpreterState * mainInterpreterState = frame_->getPythonHelper()->getPyThreadState()->interp;

    // create a thread state object for this thread
    PyThreadState * myThreadState = PyThreadState_New(mainInterpreterState);
    PyThreadState_Swap(myThreadState);


    PySys_SetArgv(1, argv);

    runPythonFile(file);

    //Clean up this thread's python interpreter reference
    PyThreadState_Swap(NULL);
    PyThreadState_Clear(myThreadState);
    PyThreadState_Delete(myThreadState);
    PyEval_ReleaseLock(); 
    free((void*)lp);
}

bool FQTermWindow::pythonCallback(const QString & func, PyObject* pArgs) {
	if(!pythonScriptLoaded_) {
		Py_DECREF(pArgs);
		return false;
	};
	
	bool done = false;
	// get the global lock
  PyEval_AcquireLock();
	// get a reference to the PyInterpreterState
      
	//Python thread references
      
	PyInterpreterState * mainInterpreterState = frame_->getPythonHelper()->getPyThreadState()->interp;
	// create a thread state object for this thread
	PyThreadState * myThreadState = PyThreadState_New(mainInterpreterState);
	PyThreadState_Swap(myThreadState);
    
	PyObject *pF = PyString_FromString(func.toUtf8());
	PyObject *pFunc = PyDict_GetItem(pDict, pF);
 	Py_DECREF(pF);

	if (pFunc && PyCallable_Check(pFunc)) 
	{
		PyObject *pValue = PyObject_CallObject(pFunc, pArgs);
		Py_DECREF(pArgs);
		if (pValue != NULL) 
		{
      if (PyBool_Check(pValue) && pValue == Py_True)
			  done = true;
			Py_DECREF(pValue);
		}
		else 
		{
			//QMessageBox::warning(this,"Python script error", getException());
      printf("%p: Python script error\n", this, getException().toUtf8().data());
		}
  }
	else 
	{
		PyErr_Print();
		printf("Cannot find python %s callback function\n", func.toUtf8().data());
	}
      
	// swap my thread state out of the interpreter
	PyThreadState_Swap(NULL);
	// clear out any cruft from thread state object
	PyThreadState_Clear(myThreadState);
   // delete my thread state object
	PyThreadState_Delete(myThreadState);
	// release the lock
	PyEval_ReleaseLock();

	if (func == "autoReply")
		startBlink();

	return done;
}


int FQTermWindow::runPythonFile( const QString& file )
{
  QString buffer("def work_thread():\n"
                 "\ttry:\n\t\texecfile('");
  buffer += QString(file).replace("\\", "\\\\");

  /* Have to do it like this. PyRun_SimpleFile requires you to pass a
   * stdio file pointer, but Vim and the Python DLL are compiled with
   * different options under Windows, meaning that stdio pointers aren't
   * compatible between the two. Yuk.
   *
   * Put the string "execfile('file')" into buffer. But, we need to
   * escape any backslashes or single quotes in the file name, so that
   * Python won't mangle the file name.
 * ---- kafa
   */
  
  /* Put in the terminating "')" and a null */
  buffer += "',{})\n\0";

  buffer += QString("\texcept:\n"
      "\t\texc, val, tb = sys.exc_info()\n"
      "\t\tlines = traceback.format_exception(exc, val, tb)\n"
      "\t\terr = string.join(lines)\n"
      "\t\tprint err\n"
      "\t\tf=open('%1','w')\n"
      "\t\tf.write(err)\n"
      "\t\tf.close()\n").arg(getErrOutputFile(this));

  buffer += QString("\t\tfqterm.formatError(%1)\n").arg((long)this);
  buffer += "\t\texit\n";

  /* Execute the file */
	PyRun_SimpleString("import thread,string,sys,traceback,fqterm");
	PyRun_SimpleString(buffer.toUtf8());
	PyRun_SimpleString(	"thread.start_new_thread(work_thread,())\n");

	return 0;
}

void FQTermWindow::initPython(const QString& file) {

	// get the global python thread lock
  PyEval_AcquireLock();

  // get a reference to the PyInterpreterState
  PyInterpreterState * mainInterpreterState = frame_->getPythonHelper()->getPyThreadState()->interp;



  // create a thread state object for this thread
  PyThreadState * myThreadState = PyThreadState_New(mainInterpreterState);
  PyThreadState_Swap(myThreadState);
  
  Py_InitModule4("fqterm", fqterm_methods,
    NULL,(PyObject*)NULL,PYTHON_API_VERSION);
    
	PyImport_AddModule("fqterm");

	if(!file.isEmpty() )
	{
    QFileInfo info(file);
    PyRun_SimpleString("import sys"); 
    PyRun_SimpleString(QString("sys.path.append(\"%1/\")").arg(info.absolutePath()).toUtf8().data()); 
		PyObject *pName = PyString_FromString(info.baseName().toUtf8().data());
		pModule = PyImport_Import(pName);
    //PyErr_Print();
		Py_DECREF(pName);
		if (pModule != NULL) 
			  pDict = PyModule_GetDict(pModule);
		else
		{
			//printf("Failed to PyImport_Import\n");
		}

		if(pDict != NULL )
			pythonScriptLoaded_ = true;
		else
		{
			//printf("Failed to PyModule_GetDict\n");
		}
	}
	
    //Clean up this thread's python interpreter reference
    PyThreadState_Swap(NULL);
    PyThreadState_Clear(myThreadState);
    PyThreadState_Delete(myThreadState);
    PyEval_ReleaseLock();
    
}

void FQTermWindow::finalizePython()
{
  // get the global python thread lock
	PyEval_AcquireLock();

	// get a reference to the PyInterpreterState
	PyInterpreterState * mainInterpreterState = frame_->getPythonHelper()->getPyThreadState()->interp;
      
	// create a thread state object for this thread
	PyThreadState * myThreadState = PyThreadState_New(mainInterpreterState);
	PyThreadState_Swap(myThreadState);
      
	//Displose of current python module so we can reload in constructor.
	if(pModule!=NULL)
		Py_DECREF(pModule);

	//Clean up this thread's python interpreter reference
	PyThreadState_Swap(NULL);
	PyThreadState_Clear(myThreadState);
	PyThreadState_Delete(myThreadState);
	PyEval_ReleaseLock();
}

#endif //HAVE_PYTHON

/* ------------------------------------------------------------------------ */
/*                                                                         */
/*                           HTTP Func                                      */
/*                                                                          */
/* ------------------------------------------------------------------------ */

void FQTermWindow::openLink() {
  QString url = session_->getUrl();
  if (!url.isEmpty()) {
	openUrlImpl(url);
  }
}

void FQTermWindow::saveLink() {
  QString url = session_->getUrl();
  if (!url.isEmpty())
    getHttpHelper(url, false);
}

void FQTermWindow::openUrl(QString url)
{
  QString caption = tr("URL Dialog");
  QString strUrl = url;
  QTextEdit *textEdit = new QTextEdit();
  textEdit->setPlainText(strUrl);
  //textEdit->setFocus();
  textEdit->setTabChangesFocus(true);
  textEdit->moveCursor(QTextCursor::End);
  

  QPushButton *okButton = new QPushButton(tr("&Open URL"));
  QPushButton *cancelButton = new QPushButton(tr("&Cancel"));
//  okButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
//  cancelButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
  

  QGridLayout *layout = new QGridLayout;
  layout->addWidget(textEdit, 0, 0, 1, -1);
  layout->addWidget(okButton, 1, 3);
  layout->addWidget(cancelButton, 1, 4);

  QDialog dlg(this);

  QFontMetrics fm(qApp->font());
  

  connect(okButton, SIGNAL(pressed()), &dlg, SLOT(accept()));
  connect(cancelButton, SIGNAL(pressed()), &dlg, SLOT(reject()));

  
  dlg.setWindowTitle(caption);
  dlg.setLayout(layout);
  dlg.resize(fm.width(QString(30, 'W')), fm.height() * (fm.width(strUrl) / fm.width(QString(30, 'W')) + 5));
  textEdit->clearFocus();
  okButton->setFocus(Qt::TabFocusReason);
  if (dlg.exec()) {
	  openUrlImpl(textEdit->toPlainText());
  }
}


void FQTermWindow::openUrlImpl(QString url)
{
    const QString &httpBrowser = FQTermPref::getInstance()->httpBrowser_;
    if (httpBrowser.isNull() || httpBrowser.isEmpty()) {
#if QT_VERSION >= 0x040400
	    QDesktopServices::openUrl(QUrl::fromEncoded(url.toLocal8Bit()));
#else
      QDesktopServices::openUrl(url);
#endif
    } else {
	    runProgram(httpBrowser, url);
    }
}


void FQTermWindow::previewLink() {
  getHttpHelper(session_->getUrl(), true);
}

void FQTermWindow::copyLink() {
  QString strUrl;
  strUrl = session_->bbs2unicode(session_->getUrl().toLatin1());

  QClipboard *clipboard = QApplication::clipboard();

  clipboard->setText(strUrl, QClipboard::Selection);
  clipboard->setText(strUrl, QClipboard::Clipboard);
}

void FQTermWindow::getHttpHelper(const QString &url, bool preview) {

  const QString &strPool = FQTermPref::getInstance()->poolDir_;

  FQTermHttp *http = new FQTermHttp(config_, this, strPool, session_->param().serverEncodingID_);
  if (getSession()->param().proxyType_ != 0) {
    QString host = getSession()->param().proxyHostName_;
    int port = getSession()->param().proxyPort_;
    bool auth = getSession()->param().isAuthentation_;
    QString user = auth ? getSession()->param().proxyUserName_ : QString();
    QString pass = auth ? getSession()->param().proxyPassword_ : QString();
    switch (getSession()->param().proxyType_)
    {
    case 1:
    case 2:
      //no support in qt.
      FQ_TRACE("network", 0) << "proxy type not supported by qt, download will not use proxy.";
      break;
    case 3:
      http->setProxy(QNetworkProxy(QNetworkProxy::Socks5Proxy, host, port, user, pass));
      break;
    case 4:
      http->setProxy(QNetworkProxy(QNetworkProxy::HttpProxy, host, port, user, pass));
      break;
    }
  }
  FQTerm::StatusBar::instance()->newProgressOperation(http).setDescription(tr("Waiting header...")).setAbortSlot(http, SLOT(cancel())).setMaximum(100);
  FQTerm::StatusBar::instance()->resetMainText();
  FQTerm::StatusBar::instance()->setProgress(http, 0);

  FQ_VERIFY(connect(http, SIGNAL(headerReceived(FQTermHttp *, const QString &)),
                    this, SLOT(startHttpDownload(FQTermHttp*,const QString&))));

  FQ_VERIFY(connect(http,SIGNAL(done(QObject*)),this,SLOT(httpDone(QObject*))));

  FQ_VERIFY(connect(http, SIGNAL(message(const QString &)),
                    pageViewMessage_, SLOT(showText(const QString &))));

  FQ_VERIFY(connect(http, SIGNAL(done(QObject*)),
                    FQTerm::StatusBar::instance(),
                    SLOT(endProgressOperation(QObject*))));

  FQ_VERIFY(connect(http, SIGNAL(percent(int)),
                    FQTerm::StatusBar::instance(), SLOT(setProgress(int))));

  FQ_VERIFY(connect(http, SIGNAL(previewImage(const QString &, bool, bool)),
                    this, SLOT(httpPreviewImage(const QString &, bool, bool))));


  http->getLink(url, preview);
}

void FQTermWindow::startHttpDownload(
    FQTermHttp *pHttp, const QString &filedesp) {
  FQTerm::StatusBar::instance()->newProgressOperation(pHttp).setDescription
      (filedesp).setAbortSlot(pHttp, SLOT(cancel())).setMaximum(100);

  FQTerm::StatusBar::instance()->resetMainText();
}

void FQTermWindow::httpDone(QObject *pHttp) {
  pHttp->deleteLater();
}

void FQTermWindow::httpPreviewImage(const QString &filename, bool raiseViewer, bool done) 
{
  if (config_->getItemValue("preference", "image").isEmpty() || done) {
    previewImage(filename, raiseViewer);
  }
}

void FQTermWindow::previewImage(const QString &filename, bool raiseViewer) {
//  QString strViewer = frame_->config()->getItemValue("preference", "image");
  QString strViewer = config_->getItemValue("preference", "image");

  if (strViewer.isEmpty()) {
    frame_->viewImages(filename, raiseViewer);
  } else if(raiseViewer) {
    runProgram(strViewer, filename);
  }

}

void FQTermWindow::beep() {
  if (session_->param().isBeep_) {
    if (session_->param().isBuzz_) {
      frame_->buzz(this);
    }
    if (FQTermPref::getInstance()->beepSoundFileName_.isEmpty() ||
      FQTermPref::getInstance()->openBeep_ == 3) {
      qApp->beep();
    }
    else {
      sound_ = NULL;

      switch (FQTermPref::getInstance()->beepMethodID_) {
        case 0:
          sound_ =
              new FQTermSystemSound(FQTermPref::getInstance()->beepSoundFileName_);
          break;
        case 1:
          sound_ =
              new FQTermExternalSound(FQTermPref::getInstance()->beepPlayerName_,
                FQTermPref::getInstance()->beepSoundFileName_);
          break;
      }

      if (sound_) {
        sound_->start();
      }
    }
  }

  QString strMsg = session_->getMessage();
  if (!strMsg.isEmpty()) {
    allMessages_ += strMsg + "\n\n";
  }

  session_->setSendingMessage();
}


void FQTermWindow::onTitleSet(const QString& title) {
  setWindowTitle(title);
}


void FQTermWindow::startBlink() {
  if (FQTermPref::getInstance()->openTabBlinking_) {
    if (!tabBlinkTimer_->isActive()) {
      tabBlinkTimer_->start(500);
    }
  }
}

void FQTermWindow::stopBlink() {
  if (tabBlinkTimer_->isActive()) {
    tabBlinkTimer_->stop();
    emit blinkTheTab(this, true);
  }
}

void FQTermWindow::setCursorType(const QPoint& mousePosition) {
  QRect rcOld;
  bool isUrl = false;
  if (FQTermPref::getInstance()->openUrlCheck_) {
    showIP(session_->isIP(ipRectangle_, rcOld));
    if (session_->isUrl(urlRectangle_, rcOld)) {
      setCursor(Qt::PointingHandCursor);
      isUrl = true;
    }
  }
  if (!isUrl) {
    FQTermSession::CursorType cursorType =
        session_->getCursorType(screen_->mapToChar(mousePosition));
    setCursor(cursors_[cursorType]);
  }
}

void FQTermWindow::setCursorPosition(const QPoint& mousePosition) {
  // set cursor pos, repaint if state changed
  QRect rc;
  if (session_->setCursorPos(screen_->mapToChar(mousePosition), rc)) {
    screen_->repaint(screen_->mapToRect(rc));

    //re-check whether the cursor is still on the selected rectangle -- dp
    QPoint localPos = mapFromGlobal(QCursor::pos()); //local.
    if (!session_->getMenuRect().contains(screen_->mapToChar(localPos)))
    {
      QRect dummyRect;
      rc = session_->getMenuRect();
      session_->setCursorPos(screen_->mapToChar(localPos), dummyRect);
      screen_->repaint(screen_->mapToRect(rc));
      //restore the state.
      session_->setCursorPos(screen_->mapToChar(mousePosition), dummyRect); 
    }
  }
}

void FQTermWindow::refreshScreen() {
  screen_->setPaintState(FQTermScreen::NewData);
  screen_->update();
}

void FQTermWindow::repaintScreen() {

  screen_->setPaintState(FQTermScreen::Repaint);
  if (clientRect_ != QRect()) {
    screen_->repaint(screen_->mapToRect(clientRect_));
  } else {
    screen_->repaint();
  }
  clientRect_ = QRect();
//  screen_->update(clientRect_);
}

void FQTermWindow::enterMenuItem() {
  char cr = CHAR_CR;
  QRect rc = session_->getMenuRect();
  FQTermSession::PageState ps = session_->getPageState();
  switch (ps) {
    case FQTermSession::Menu:
    case FQTermSession::MailMenu:
    case FQTermSession::TOP10:
      if (!rc.isEmpty()) {
        char cMenu = session_->getMenuChar();
        session_->write(&cMenu, 1);
        session_->write(&cr, 1);
      }
      break;
    case FQTermSession::ArticleList:
    case FQTermSession::BoardList:
    case FQTermSession::FriendMailList:
    case FQTermSession::EliteArticleList:
      if (!rc.isEmpty()) {
        int n = rc.y() - session_->getBuffer()->getCaretLine();
        // scroll lines
        int cursorType = n>0?FQTermSession::kDown:FQTermSession::kUp;
        n = qAbs(n);
        while (n) {
          session_->write(directions_[cursorType], 4);
          n--;
        }
      }
      session_->write(&cr, 1);
      break;
    default:
      // TODO: process other PageState.
      break;
  }
}

void FQTermWindow::processLClick(const QPoint& cellClicked) {
  int cursorType = session_->getCursorType(cellClicked);
  switch(cursorType)
  {
    case FQTermSession::kHome:
    case FQTermSession::kEnd:
    case FQTermSession::kPageUp:
    case FQTermSession::kPageDown:
      session_->writeStr(directions_[cursorType]);
      break;
    case FQTermSession::kUp:
    case FQTermSession::kDown:
    case FQTermSession::kLeft:
      session_->writeStr(directions_[cursorType]);
      break;
    case FQTermSession::kRight:  //Hand
      enterMenuItem();
      break;
    case FQTermSession::kNormal:
      char cr = CHAR_CR;
      session_->write(&cr, 1);
      break;
  }
}

void FQTermWindow::startSelecting(const QPoint& mousePosition) {
  // clear the selected before
  session_->clearSelect();
  sessionUpdated();

  // set the selecting flag
  isSelecting_ = true;
  lastMouseCell_ = screen_->mapToChar(mousePosition);
}

void FQTermWindow::onSelecting(const QPoint& mousePosition) {
  if (mousePosition.y() < childrenRect().top()) {
    screen_->scrollLine(-1);
  }
  if (mousePosition.y() > childrenRect().bottom()) {
    screen_->scrollLine(1);
  }

  QPoint curMouseCell = screen_->mapToChar(mousePosition);
  session_->setSelect(lastMouseCell_, curMouseCell);
  sessionUpdated();
}

void FQTermWindow::finishSelecting(const QPoint& mousePosition) {
  QPoint currentMouseCell = screen_->mapToChar(mousePosition);
  session_->setSelect(currentMouseCell, lastMouseCell_);
  refreshScreen();
  if (session_->param().isAutoCopy_) {
    copy();
  }
  isSelecting_ = false;
}

void FQTermWindow::sendKey(const int keyCode, const Qt::KeyboardModifiers modifier,
                           const QString &t) {
  if (keyCode == Qt::Key_Shift || keyCode == Qt::Key_Control
      || keyCode == Qt::Key_Meta || keyCode == Qt::Key_Alt) {
    return;
  } 
  
  int key = keyCode;
  QString text = t;
  
#ifdef __APPLE__
  // On Mac, pressing Command Key (Apple Key) will set the ControlModifier flag.
  bool alt_on = ((modifier & Qt::ControlModifier) || (modifier & Qt::AltModifier));
  bool ctrl_on = (modifier & Qt::MetaModifier);
#else
  bool alt_on = ((modifier & Qt::MetaModifier) || (modifier & Qt::AltModifier));
  bool ctrl_on = (modifier & Qt::ControlModifier);
#endif
  bool shift_on = (modifier & Qt::ShiftModifier);
  
#ifdef __APPLE__
  // Bullshit! Qt4.4.0 on Mac generates weird key evnets.
  // fix them here.

  if (ctrl_on) {
    static const char char_map[][2] = {
      {'@', '\x00'}, {'A', '\x01'}, {'B', '\x02'}, {'C', '\x03'}, {'D', '\x04'},
      {'E', '\x05'}, {'F', '\x06'}, {'G', '\x07'}, {'H', '\x08'}, {'J', '\x0a'},
      {'K', '\x0b'}, {'L', '\x0c'}, {'N', '\x0e'}, {'O', '\x0f'}, {'P', '\x10'},
      {'Q', '\x11'}, {'R', '\x12'}, {'S', '\x13'}, {'T', '\x14'}, {'U', '\x15'},
      {'V', '\x16'}, {'W', '\x17'}, {'X', '\x18'}, {'Y', '\x19'}, {'Z', '\x1a'},
      {'\\', '\x1c'}, {']', '\x1d'}, {'^', '\x1e'}, {'_', '\x1f'}
    };

    const int num = sizeof(char_map)/2;

    int expected_key = key;
        
    for (int i = 0; i < num; ++i) {
      const char *c = char_map[i];
      if (c[0] == key) {
        expected_key = c[1];
        break;
      }
    }
   
    if (expected_key != key) {
      FQ_TRACE("sendkey", 5) << "translate the key code from "
                             << (int)key << "(" << (char)key << ")"
                             << " to "
                             << (int)expected_key
                             << "(" << (char)expected_key << ")"
                             << ", and change the text appropriately." ;
      key = expected_key;
      text.clear();
      text.append((char) expected_key);
    }
  }

  if (!ctrl_on && alt_on && !shift_on) {
    if (text.size() == 0 && 0 <= key && key <= 128) {
      FQ_TRACE("sendkey", 5)
          << "add appropriate text according to the key code.";
      text.append((char)key);
    }
  }
  
  if (!ctrl_on && alt_on && shift_on) {
    // fix the key code when both shift key and meta/alt key are pressed.
    static const char char_map[][2] = {
      {'`', '~'}, {'1', '!'}, {'2', '@'}, {'3', '#'}, {'4', '$'},
      {'5', '%'}, {'6', '^'}, {'7', '&'}, {'8', '*'}, {'9', '('}, 
      {'0', ')'}, {'-', '_'}, {'=', '+'}, {'[', '{'}, {']', '}'}, 
      {'\\', '|'}, {';', ':'}, {'\'', '"'}, {',', '<'},{'.', '>'},
      {'/', '?'}
    };
    const int num = sizeof(char_map)/2;

    int expected_key = key;
        
    for (int i = 0; i < num; ++i) {
      const char *c = char_map[i];
      if (c[0] == key) {
        expected_key = c[1];
        break;
      }
    }

    if (expected_key != key) {
      FQ_TRACE("sendkey", 5) << "translate the key code from "
                             << (int)key << "(" << (char)key << ")"
                             << " to "
                             << (int)expected_key
                             << "(" << (char)expected_key << ")";
      key = expected_key;
    }
        
    if (text.size() == 0) {
      text.append(key);
    } else {
      text[0] = key;
    }
    FQ_TRACE("sendkey", 5)
        << "set the text for this key event as the keycode.";
  }      
#endif

  FQ_TRACE("sendkey", 5) << "(alt " << (alt_on? " on": "off")
                         << ", ctrl " << (ctrl_on? " on": "off")
                         << ", shift " << (shift_on? " on": "off") << ")"
                         << " key: " << key
                         << " text: " << (text.size() == 0 ? "NULL" : text)
                         << " (in hex: "
                         << dumpHexString << text.toStdString()
                         << dumpNormalString << ")";
  
  if (session_->getBuffer()->isAnsiMode()) {
    if (session_->getBuffer()->isCursorMode()) {
      switch(key) {
        case Qt::Key_Up:
          session_->writeStr("\x1bOA");
          return;
        case Qt::Key_Down:
          session_->writeStr("\x1bOB");
          return;
        case Qt::Key_Left:
          session_->writeStr("\x1bOD");
          return;
        case Qt::Key_Right:
          session_->writeStr("\x1bOC");
          return;
      }
    } else {
      switch(key) {
        case Qt::Key_Up:
          session_->writeStr("\x1b[A");
          return;
        case Qt::Key_Down:
          session_->writeStr("\x1b[B");
          return;
        case Qt::Key_Left:
          session_->writeStr("\x1b[D");
          return;
        case Qt::Key_Right:
          session_->writeStr("\x1b[C");
          return;
      }
    }
  } else {
    // VT52 mode
    switch(key) {
      case Qt::Key_Up:
        // Do NOT change the parameter to "\x1bA".
        session_->writeStr("\x1b" "A");
        return;
      case Qt::Key_Down:
        // Do NOT change the parameter to "\x1bB".        
        session_->writeStr("\x1b" "B");
        return;
      case Qt::Key_Left:
        // Do NOT change the parameter to "\x1bD".        
        session_->writeStr("\x1b" "D");
        return;
      case Qt::Key_Right:
        // Do NOT change the parameter to "\x1bC".        
        session_->writeStr("\x1b" "C");
        return;
    }
  }

  if (!session_->getBuffer()->isNumericMode() &&
      (modifier & Qt::KeypadModifier)) {
    switch(key) {
      case Qt::Key_0:
      case Qt::Key_1:
      case Qt::Key_2:
      case Qt::Key_3:
      case Qt::Key_4:
      case Qt::Key_5:
      case Qt::Key_6:
      case Qt::Key_7:
      case Qt::Key_8:
      case Qt::Key_9:
        {
          char tmp[] = "\x1bOp";
          tmp[2] = 'p' + key - Qt::Key_0;
          session_->writeStr(tmp);
        }
        return;
      case Qt::Key_Plus:
        session_->writeStr("\x1bOl");
        return;
      case Qt::Key_Minus:
        session_->writeStr("\x1bOm");
        return;
      case Qt::Key_Period:
        session_->writeStr("\x1bOn");
        return;
      case Qt::Key_Asterisk:
        session_->writeStr("\x1bOM");
        return;
    }
  }

  switch (key) {
    case Qt::Key_Home:
      session_->writeStr(directions_[0]);
      return;
    case Qt::Key_End:
      session_->writeStr(directions_[1]);
      return;
    case Qt::Key_PageUp:
      session_->writeStr(directions_[2]);
      return;
    case Qt::Key_PageDown:
      session_->writeStr(directions_[3]);
      return;
    case Qt::Key_Up:
      session_->writeStr(directions_[4]);
      return;
    case Qt::Key_Down:
      session_->writeStr(directions_[5]);
      return;
    case Qt::Key_Left:
      session_->writeStr(directions_[6]);
      return;
    case Qt::Key_Right:
      session_->writeStr(directions_[7]);
      return;
    case Qt::Key_F1:
      session_->writeStr("\x1b[11~");
      return;
    case Qt::Key_F2:
      session_->writeStr("\x1b[12~");
      return;
    case Qt::Key_F3:
      session_->writeStr("\x1b[13~");
      return;
    case Qt::Key_F4:
      session_->writeStr("\x1b[14~");
      return;
    case Qt::Key_F5:
      session_->writeStr("\x1b[15~");
      return;
    case Qt::Key_F6:
      session_->writeStr("\x1b[17~");
      return;
    case Qt::Key_F7:
      session_->writeStr("\x1b[18~");
      return;
    case Qt::Key_F8:
      session_->writeStr("\x1b[19~");
      return;
    case Qt::Key_F9:
      session_->writeStr("\x1b[20~");
      return;
    case Qt::Key_F10:
      session_->writeStr("\x1b[21~");
      return;
    case Qt::Key_F11:
      session_->writeStr("\x1b[23~");
      return;
    case Qt::Key_F12:
      session_->writeStr("\x1b[24~");
      return;
  }
  
  if (text.length() > 0) {
    if (alt_on) {
      // Use ESC to emulate Alt-XX key.
      session_->writeStr("\x1b");
    }

    if (ctrl_on) {
      switch(key) {
        case Qt::Key_At:     // Ctrl-@
        case Qt::Key_Space:  // Ctrl-SPACE
          // send NULL
          session_->write("\x0", 1);
          return;
        case Qt::Key_AsciiCircum:  // Ctrl-^
        case Qt::Key_AsciiTilde:   // Ctrl-~
        case Qt::Key_QuoteLeft:    // Ctrl-`
          // send RS
          session_->write("\x1e", 1);
          return;
        case Qt::Key_Underscore: // Ctrl-_
        case Qt::Key_Question:   // Ctrl-?
          // send US
          session_->write("\x1f", 1);            
          return;
      }
    }
    
    if (key == Qt::Key_Backspace) {
      if (shift_on) {
        session_->writeStr("\x1b[3~");
      } else {
        if (session_->param().backspaceType_ ==  0)
          session_->writeStr("\x08");
        else
          session_->writeStr("\x7f");
      }
      return;
    }
    
    if (key == Qt::Key_Delete) {
      if (shift_on) {
        if (session_->param().backspaceType_ ==  0)
          session_->writeStr("\x08");
        else
          session_->writeStr("\x7f");
      } else {
        session_->writeStr("\x1b[3~");
      }
      return;
    }

    if (key == Qt::Key_Return) {
      if (session_->getBuffer()->isNewLineMode()) {
        session_->write("\r\n", 2);
      } else {
        session_->write("\r", 1);
      }
      return;
    }
    
    QByteArray cstrTmp = session_->unicode2bbs_smart(text);
    session_->write(cstrTmp, cstrTmp.length());
  }
  return;
}

void FQTermWindow::forcedRepaintScreen() {
  QResizeEvent *re = new QResizeEvent(screen_->size(), screen_->size());
  QApplication::postEvent(screen_, re);
}

void FQTermWindow::updateSetting(const FQTermParam& param) {
  session_->updateSetting(param);
  screen_->setTermFont(true,
                       QFont(param.englishFontName_,
                             param.englishFontSize_));
  screen_->setTermFont(false,
                       QFont(param.otherFontName_,
                             param.otherFontSize_));
  screen_->setSchema();

  setWindowTitle(param.name_);

  forcedRepaintScreen();
}

void FQTermWindow::setFont()
{
  bool isEnglish =
       ((QAction*)(sender()))->data().toBool();
  setFont(isEnglish);
}

void FQTermWindow::setFont(bool isEnglish) {
  bool ok;
  QString& fontName = isEnglish?session_->param().englishFontName_:session_->param().otherFontName_;
  int& fontSize = isEnglish?session_->param().englishFontSize_:session_->param().otherFontSize_;
  QFont font(fontName, fontSize);
  font = QFontDialog::getFont(&ok, font, this, tr("Font Selector")
#ifdef __APPLE__
, QFontDialog::DontUseNativeDialog
#endif
  );
  if (ok == true) {
    if (FQTermPref::getInstance()->openAntiAlias_) {
      font.setStyleStrategy(QFont::PreferAntialias);
    }
    screen_->setTermFont(isEnglish, font);
    fontName = font.family();
    fontSize = font.pointSize();
    forcedRepaintScreen();
  }
}


void FQTermWindow::runScript(const QString &filename) {
  script_engine_->runScript(filename);
}

//-----------
static int translateQtModifiers(Qt::KeyboardModifiers modifiers) {
  int state = 0;
  if (modifiers &Qt::AltModifier) {
    state |= SBS_ALT;
  }
  if (modifiers &Qt::ControlModifier) {
    state |= SBS_CTRL;
  }
  if (modifiers &Qt::ShiftModifier) {
    state |= SBS_SHIFT;
  }
  return state;
}

static int translateQtButton(Qt::MouseButtons btnstate) {
  int state = 0;
  if(btnstate&Qt::LeftButton)
    state |= SBS_LEFT_BUTTON;
  if(btnstate&Qt::RightButton)
    state |= SBS_RIGHT_BUTTON;
  if(btnstate&Qt::MidButton)
    state |= SBS_MID_BUTTON;
  return state;
}

bool FQTermWindow::scriptKeyEvent( QKeyEvent *keyevent ){
  bool res = false;
  int state = 0;
  state |= translateQtModifiers(keyevent->modifiers());

  res = postScriptCallback(SFN_KEY_EVENT
#ifdef HAVE_PYTHON
                           ,Py_BuildValue("liii", this, SKET_KEY_PRESS, state, keyevent->key())
#endif //HAVE_PYTHON
                           ,QScriptValueList() << SKET_KEY_PRESS << state << keyevent->key());
  return res;
}

bool FQTermWindow::scriptMouseEvent(QMouseEvent *mouseevent){
  int type = SMET_UNKOWN;
  switch (mouseevent->type())
  {
  case QEvent::MouseButtonPress:
    type = SMET_MOUSE_PRESS;
    break;
  case QEvent::MouseButtonRelease:
    type = SMET_MOUSE_RELEASE;
    break;
  case QEvent::MouseButtonDblClick:
    type = SMET_MOUSE_DBCLICK;
    break; 
  case QEvent::MouseMove:
    type = SMET_MOUSE_MOVE;
    break;
  }

  int delta = 0;
  int state=0;

  state |= translateQtButton(mouseevent->button());
  state |= translateQtModifiers(mouseevent->modifiers());

  QPoint ptc = screen_->mapToChar(mouseevent->pos());
  ptc.setY(ptc.y() - screen_->getBufferStart());
  int res = postScriptCallback(SFN_MOUSE_EVENT
#ifdef HAVE_PYTHON
                               ,Py_BuildValue("liiiii", this, type, state, ptc.x(), ptc.y(),delta)
#endif //HAVE_PYTHON
                               ,QScriptValueList() << type << state << ptc.x() << ptc.y() << delta);
  return res;
}

bool FQTermWindow::scriptWheelEvent( QWheelEvent *wheelevent ) {
  int type = SMET_WHEEL;
  int delta = 0;
  int state=0;
  state |= translateQtModifiers(wheelevent->modifiers());
  QPoint ptc = screen_->mapToChar(wheelevent->pos());
  ptc.setY(ptc.y() - screen_->getBufferStart());
  int res = postScriptCallback(SFN_MOUSE_EVENT
#ifdef HAVE_PYTHON
                               ,Py_BuildValue("liiiii", this, type, state, ptc.x(), ptc.y(),wheelevent->delta())
#endif //HAVE_PYTHON
                               ,QScriptValueList() << type << state << ptc.x() << ptc.y() << wheelevent->delta());
  return res;
}

const QString FQTermExternalEditor::textEditName_ =  "external editor input";

FQTermExternalEditor::FQTermExternalEditor(QWidget* parent) 
: QObject(parent),
  editorProcess_(NULL),
  started_(false) {
  editorProcess_ = new QProcess(this);
  FQ_VERIFY(connect(editorProcess_, SIGNAL(stateChanged(QProcess::ProcessState)), 
                    this, SLOT(stateChanged(QProcess::ProcessState))));
  clearTempFileContent();
}  

FQTermExternalEditor::~FQTermExternalEditor() {
  editorProcess_->kill();
  QFile::remove(getTempFilename());
  delete editorProcess_;
}

void FQTermExternalEditor::execDialog() {
  QDialog *dialog = new QDialog((QWidget *)parent());
  QGridLayout *layout = new QGridLayout(dialog);
  QTextEdit *text = new QTextEdit(dialog);
  text->setObjectName(textEditName_);
  layout->addWidget(text);
  QBoxLayout  *rowLayout = new QBoxLayout(QBoxLayout::LeftToRight, dialog);
  layout->addLayout(rowLayout, 1, 0, Qt::AlignHCenter);
  QPushButton *ok = new QPushButton(tr("OK"), dialog);
  QPushButton *cancel = new QPushButton(tr("Cancel"), dialog);
  rowLayout->addWidget(ok);
  rowLayout->addWidget(cancel);
  FQ_VERIFY(connect(ok, SIGNAL(clicked()),
    dialog, SLOT(accept())));
  FQ_VERIFY(connect(cancel, SIGNAL(clicked()),
    dialog, SLOT(reject())));
  FQ_VERIFY(connect(dialog, SIGNAL(accepted()),
    this, SLOT(readDialogData())));
  FQ_VERIFY(connect(dialog, SIGNAL(rejected()),
    this, SLOT(closeDialog())));
  dialog->setModal(true);
  dialog->exec();
}

void FQTermExternalEditor::readDialogData() {
  QDialog *dialog = (QDialog*)sender();
  QTextEdit* edit = (QTextEdit*)dialog->findChild<QTextEdit*>(textEditName_);
  if (edit) {
    emit done(edit->toPlainText());
  }
  closeDialog();
}

void FQTermExternalEditor::closeDialog() {
  sender()->deleteLater();
}

void FQTermExternalEditor::start() {
  if (FQTermPref::getInstance()->externalEditor_.isEmpty()) {
    execDialog();
    return;
  }
  if (started_)
    return;
  started_ = true;
  if (FQTermPref::getInstance()->externalEditorArg_.isEmpty()) {
    editorProcess_->start(FQTermPref::getInstance()->externalEditor_, QStringList() << getTempFilename());
  } else {
    FQ_TRACE("editor", 5) << FQTermPref::getInstance()->externalEditorArg_.arg(getTempFilename());
    editorProcess_->start(FQTermPref::getInstance()->externalEditor_ + " " + FQTermPref::getInstance()->externalEditorArg_.arg(getTempFilename()));
  }
}


QString FQTermExternalEditor::getTempFilename() {
    #ifndef WIN32
    return "/tmp/.fqterm_tmp.txt";
    #else
    return getPath(USER_CONFIG) + "tmp_do_not_use.txt";
    #endif
}

void FQTermExternalEditor::clearTempFileContent() {
  QFile file(getTempFilename());
  file.open(QIODevice::Truncate | QIODevice::WriteOnly);
  file.write("\xEF\xBB\xBF");
  file.flush();
  file.close();
}

void FQTermExternalEditor::stateChanged( QProcess::ProcessState state ) {
  //FQ_TRACE("editor", 0) << "stateChanged: " << state << editorProcess_->readAllStandardOutput() << editorProcess_->readAllStandardError();
  if (!started_ || state != QProcess::NotRunning)
    return;
  QFile file(getTempFilename());
  file.open(QIODevice::ReadOnly);
  QString result = U82U(file.readAll());
  file.close();
#if !defined(WIN32) && !defined(__APPLE__) 
  if (result.endsWith(OS_NEW_LINE)) {
    result.resize(result.size() - 1);
  }
#endif
  emit done(result);
  clearTempFileContent();
  started_ = false;
}
} // namespace FQTerm
#include "fqterm_window.moc"
