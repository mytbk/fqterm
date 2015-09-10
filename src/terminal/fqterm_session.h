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

#ifndef FQTERM_SESSION_H
#define FQTERM_SESSION_H

#include <vector>

#include <QPoint>
#include <QRect>
#include <QObject>
#include <QThread>
#include <QString>
#include <QWaitCondition>
#include <QList>
#include <QtScript/QScriptValue>
#include <QReadWriteLock>
#include "fqterm_param.h"
#include "fqterm_config.h"
#include "fqterm_convert.h"
#ifdef HAVE_PYTHON
#include <Python.h>
#endif //HAVE_PYTHON

class QRect;
class QTimer;

namespace FQTerm {

class FQTermScriptEventListener {
public:
  bool postScriptCallback(const QString& func, 
#ifdef HAVE_PYTHON
                          PyObject* pArgs, 
#endif //HAVE_PYTHON
                          const QScriptValueList & args = QScriptValueList())
  {
    bool res = postQtScriptCallback(func, args);
#ifdef HAVE_PYTHON
    res = postPythonCallback(func, pArgs) || res;
#endif //HAVE_PYTHON
    return res;
  }
  virtual long windowID() = 0;
  virtual ~FQTermScriptEventListener() {}
private:
  virtual bool postQtScriptCallback(const QString& func, const QScriptValueList & args = QScriptValueList()) = 0;
#ifdef HAVE_PYTHON
  virtual bool postPythonCallback(const QString& func, PyObject* pArgs) = 0;
#endif //HAVE_PYTHON
};
 
struct LineColorInfo {
  bool hasBackgroundColor;
  bool hasForegroundColor;
  bool uniBackgroundColor;
  bool uniForegroundColor;
  QList<unsigned char> backgroundColorIndex;
  QList<unsigned char> foregroundColorIndex;
};

class FQTermConfig;
class FQTermBuffer;
class FQTermTextLine;
class FQTermTelnet;
class FQTermDecode;
class FQTermZmodem;
class ArticleCopyThread;

class FQTermSession: public QObject {
  Q_OBJECT;
 public:
  enum CursorType {
    kHome = 0,
    kEnd = 1,
    kPageUp = 2,
    kPageDown = 3,
    kUp = 4,
    kDown = 5,
    kLeft = 6,
    kRight = 7,
    kNormal = 8
  };

  FQTermSession(FQTermConfig *, FQTermParam);
  ~FQTermSession();

  enum PageState {
    Undefined = -1,
    Menu = 0,
    MailMenu = 1,
    ArticleList = 2,
    EliteArticleList = 3,
    BoardList = 4,
    FriendMailList = 5,
    Message = 6,
    Read = 7,
    Edit = 8,
    TOP10 = 9,
  };

  enum ProtocolIndex {
    Http = 0,
    Https = 1,
    Mms = 2,
    Rstp = 3,
    Ftp = 4,
    Mailto = 5,
    Telnet = 6,
    ProtocolSupported = 7
  };

  static const QString endOfUrl[];

  static const QString protocolPrefix[];
  PageState getPageState();

  // Set current cursor postion to pt,
  // return whether the selection rectangle is changed.
  // the output parameter rc be a rectangle including both
  // the new and the old selection region.
  bool setCursorPos(const QPoint &pt, QRect &rc);
  CursorType getCursorType(const QPoint &);

  // Get the menu char detected in getSelectRect().
  char getMenuChar();

  // Get current selection rectangle.
  // also detect the menu char if in kMenu page state.
  QRect detectMenuRect();
  QRect getMenuRect() {return menuRect_;}
  void setMenuRect(int row, int col, int len);
  // detect whether the given line or point is contained by current selection rectangle.
  bool isSelectedMenu(int line);
  bool isSelectedMenu(const QPoint &);
  //selection.
  void setSelect(const QPoint &pt1, const QPoint &pt2);
  void clearSelect();

  bool isUrl(QRect &, QRect &);
  bool isIP(QRect &, QRect &);
  QString getUrl();
  QString getIP();



  // Set current screen start line to help detect cursor type
  // and select rectangle corresponding to current cursor postion.
  void setScreenStart(int);



  // Set a line of buffer to have been changed from start to end.
  void clearLineChanged(int index);
  void setLineAllChanged(int index);

  bool isAntiIdle();
  void setAntiIdle(bool antiIdle);
  void leaveIdle();

  bool isAutoReply();
  void setAutoReply(bool autoReply);
  QString getMessage();

  void setAutoReconnect(bool autoReconnect);


  bool readyForInput();
  void setTermSize(int col, int row);

  QWaitCondition& getWaitCondition() {return waitCondition_;};
  //this function will do
  //1. convert unicode to bbs encoding
  //2. if there are some chars express same meaning in simplify/traditional
  //Chinese, auto covert them by considering ime/bbs encoding.
  QByteArray unicode2bbs_smart(const QString &);
  QString bbs2unicode(const QByteArray &text);
  QByteArray unicode2bbs(const QString &text);

  // Write data raw data
  int write(const char *data, int len);
  int writeStr(const char *str);

  // type: 0-no proxy; 1-wingate; 2-sock4; 3-socks5
  // needAuth: if authentation needed
  void setProxy(int type, bool needAuth,
    const QString &hostname, quint16 portNumber,
    const QString &username, const QString &password);
  // User close the connection
  void close();

  bool isConnected() {return isConnected_;}
  void setSendingMessage(bool sending = true) {isSendingMessage_ = sending;}
  const QPoint& urlStartPoint() {return urlStartPoint_;}
  const QPoint& urlEndPoint() {return urlEndPoint_;}

  FQTermParam& param() {return param_;}
  void updateSetting(const FQTermParam& p);
  QReadWriteLock& getBufferLock() {return bufferWriteLock_;}

  // raw data logging
  void startLogging();
  void stopLogging(bool);
  bool isLogging();

 public:


 public slots:
  FQTermBuffer *getBuffer() const;
  void connectHost(const QString &hostname, quint16 portnumber);
  void reconnect();
  void disconnect();

  void cancelZmodem();
  void changeTelnetState(int state);

  void handleInput(const QString &text);

  void copyArticle();

 signals:
  void messageAutoReplied();
  void articleCopied(int state, const QString content);
  void sessionUpdated();
  void connectionClosed();
  void bellReceived();
  void onTitleSet(const QString&);
  void startAlert();
  void stopAlert();

  void requestUserPwd(QString *user, QString *pwd, bool *isOK);

  void telnetStateChanged(int state);
  void zmodemStateChanged(int type, int value, const char *status);

  void errorMessage(QString);

private slots:
  void readReady(int size, int raw_size);
  void onIdle();
  void onAutoReply();
  void onEnqReceived();
  void onSSHAuthOK();
  void setMouseMode(bool on);

private:
  void sendMouseState(int, Qt::KeyboardModifier, Qt::KeyboardModifier, const QPoint &);
  void getLineColorInfo(const FQTermTextLine * line, LineColorInfo * colorInfo);
  bool isIllChar(char);
  void detectPageState();
  QString expandUrl(const QPoint& pt, QPair<int, int>& range);
  bool checkUrl(QRect &, QRect &, bool);
  QByteArray parseString(const QByteArray &cstr, int *len = 0);
  void finalizeConnection();
  bool isPageComplete();
  void autoReplyMessage();
  void reconnectProcess();
  void doAutoLogin();

private:
  //this read-write lock will be locked as a writer's lock if the buffer is being changed
  //so for a reader who wants to ensure thread-safe, he should lock
  //this lock as a reader's lock.
  mutable QReadWriteLock bufferWriteLock_;

  FQTermParam param_;
  FQTermConvert encodingConverter_;
  bool isTelnetLogining_;
  bool isSSHLogining_;

  bool isIdling_;
  bool isMouseX11_;
  bool isConnected_;
  bool isSendingMessage_;

  QRect urlRect_;
  QPoint urlStartPoint_;
  QPoint urlEndPoint_;
  QString url_;
  QString ip_;
  char menuChar_;
  QRect menuRect_;
  PageState pageState_;
  QPoint cursorPoint_;
  int screenStartLineNumber_;

  FQTermZmodem *zmodem_;
  FQTermDecode *decoder_;
  FQTermTelnet *telnet_;
  FQTermBuffer *termBuffer_;

  QTimer *idleTimer_;
  QTimer *autoReplyTimer_;

  QWaitCondition waitCondition_;
  ArticleCopyThread *acThread_;

  std::vector<char> telnet_data_;
  std::vector<char> raw_data_;
  // raw data logging
  bool isLogging_;
  QByteArray *logData;
  
  int reconnectRetry_;
public:
  void setScriptListener(FQTermScriptEventListener* pythonListener) {
    scriptListener_ = pythonListener;
  }
private:
  FQTermScriptEventListener* scriptListener_;
};

class ArticleCopyThread: public QThread {
  Q_OBJECT;
 public:
  ArticleCopyThread(FQTermSession &bbs, QWaitCondition &waitCondition, QReadWriteLock &bufferLock);

  ~ArticleCopyThread();

 signals:
  void articleCopied(int state, const QString content);
  void writeSession(const QString&);
 protected:
  virtual void run();
 private:
  FQTermSession &session_;
  QWaitCondition &waitCondition_;

  QReadWriteLock &lock_;
};

}  // namespace FQTerm

#endif // FQTERM_SESSION_H
