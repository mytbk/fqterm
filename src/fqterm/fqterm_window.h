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

#ifndef FQTERM_WINDOW_H
#define FQTERM_WINDOW_H

#include <QMainWindow>
#include <QCursor>
#include <QString>
#include <QProcess>
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "fqterm_param.h"
#include "fqterm_config.h"
#include "fqterm_convert.h"
#include "fqterm_session.h"

class QCloseEvent;
class QEvent;
class QKeyEvent;
class QMouseEvent;
class QMouseEvent; 
class QProgressBar;
class QWheelEvent;

namespace FQTerm {

//class QProgressDialog;
class PageViewMessage;
class FQTermConfig;
class FQTermImage;
class FQTermFrame;
class FQTermHttp;
class FQTermScreen;
class FQTermSession;
class FQTermSound;
class FQTermWindow;
class popWidget;
class zmodemDialog;
class FQTermScriptEngine;

class FQTermExternalEditor : public QObject {
  Q_OBJECT;
public:
  FQTermExternalEditor(QWidget* parent);
  ~FQTermExternalEditor();
  void start();
signals:
  void done(const QString&);
public slots:
  void stateChanged(QProcess::ProcessState state);
protected slots:
  void readDialogData();
  void closeDialog();
private:
  void execDialog();
  QString getTempFilename();
  void clearTempFileContent();
  
  static const QString textEditName_;
  bool started_;
  QProcess* editorProcess_;
};

class FQTermWindow : public QMainWindow,
                      public FQTermScriptEventListener {
  friend class FQTermScreen;

  Q_OBJECT;
 public:
  FQTermWindow(FQTermConfig *, FQTermFrame *frame, FQTermParam param, int addr = -1, QWidget
              *parent = 0, const char *name = 0, Qt::WindowFlags wflags = Qt::Window);
  ~FQTermWindow();

  void connectHost();
  bool isConnected();

  void disconnect();
  //redraw the dirty lines
  void refreshScreen();
  //repaint a dirty rectangle.
  void repaintScreen();
  //force a repaint by sending a resize event
  void forcedRepaintScreen();
  void viewMessages();
  void toggleAutoReply();
  void toggleAntiIdle();
  void toggleAutoReconnect();
  void setFont(bool isEnglish);
  void saveSetting(bool ask = true);

  void runScript(const QString & filename);
  int externInput(const QByteArray &);
  int externInput(const QString &);
  void getHttpHelper(const QString &, bool);

  void openUrl(QString url);
  void openUrlImpl(QString url);

  FQTermSession * getSession() const { return session_; }
  FQTermScreen * getScreen() const {return screen_;}
  FQTermConfig * getConfig() const { return config_; }
        
  QPoint getUrlStartPoint() const { return urlStartPoint_; }
  QPoint getUrlEndPoint() const { return urlEndPoint_; }

  //FQTermScriptEventListener
private:
  virtual bool postQtScriptCallback(const QString& func, const QScriptValueList & args = QScriptValueList());
#ifdef HAVE_PYTHON
  virtual bool postPythonCallback(const QString& func, PyObject* pArgs);
#endif //HAVE_PYTHON

public:
  virtual long windowID() {return long(this);}
  //end FQTermScriptEventListener

signals:
  void resizeSignal(FQTermWindow*);
  void refreshOthers(FQTermWindow*);
  void blinkTheTab(FQTermWindow*, bool);
  void connectionClosed(FQTermWindow*);
 public slots:
  // ui
  void copy();
  void paste();
  void openAsUrl();
  void searchIt();
  void shareIt();
  void externalEditor();
  void fastPost();
  void copyArticle();
  void setting();
  void setColor();
  void runScript();
  void stopScript();
  
  void sendParsedString(const char*);
  void showIP(bool show = true);

  void beep();
  void startBlink();
  void stopBlink();
  void onTitleSet(const QString& title);

  void connectionClosed();

  void messageAutoReplied();

  void pasteHelper(bool);

  QByteArray parseString(const QByteArray &, int *len = 0);

  //  void sendMouseState(int, Qt::KeyboardModifier, Qt::KeyboardModifier, const
  //                    QPoint &);


 protected slots:
  void setFont();
  void recreateMenu();
  
  //refresh screen & reset cursor position
  void sessionUpdated();

  void requestUserPwd(QString *userName, QString *password, bool *isOK);

  void TelnetState(int);
  void ZmodemState(int, int, const char *);
  void showSessionErrorMessage(QString);

  void blinkTab();

  void externalEditorDone(const QString& str);

  //http menu
  void previewLink();
  void saveLink();
  void openLink();
  void copyLink();
  void previewImage(const QString &filename, bool raiseViewer);
  void httpPreviewImage(const QString &filename, bool raiseViewer, bool done);
  void startHttpDownload(FQTermHttp *, const QString &filedesp);

  void httpDone(QObject*);
 
  // decode
  // void setMouseMode(bool);
  void articleCopied(int e, const QString content);


 protected:
  bool event(QEvent*);

  void resizeEvent(QResizeEvent *);
  void mouseDoubleClickEvent(QMouseEvent*);
  void mouseMoveEvent(QMouseEvent*);
  void mousePressEvent(QMouseEvent*);
  void mouseReleaseEvent(QMouseEvent*);
  void wheelEvent(QWheelEvent*);
  void enterEvent(QEvent*);
  void leaveEvent(QEvent*);
  void changeEvent(QEvent*);
  void closeEvent(QCloseEvent*);
  void keyPressEvent(QKeyEvent*);
  //void focusInEvent (QFocusEvent *);

 private:
  FQTermFrame *frame_;
  FQTermScreen *screen_;
  FQTermSession *session_;
  QString allMessages_;
  // before calling repaintScreen(), pls
  // fill this rectangle.
  QRect clientRect_;
  QPoint urlEndPoint_;
  QPoint urlStartPoint_;

  QMenu *menu_;
  QMenu *urlMenu_;

  static char directions_[][5];
  QCursor cursors_[9];

  FQTermConvert encodingConverter_;

  // mouse select
  QPoint lastMouseCell_;
  bool isSelecting_;

  // address setting
  int addressIndex_;

  // url rect
  QRect urlRectangle_;

  //ip rect
  QRect ipRectangle_;
 
  // play sound
  FQTermSound *sound_;


  FQTermConfig *config_;
  zmodemDialog *zmodemDialog_;

  //osd
  PageViewMessage *pageViewMessage_;

  popWidget *popWindow_;
  QTimer *tabBlinkTimer_;

  bool isMouseClicked_;
  bool blinkStatus_;

  bool isUrlUnderLined_;

  FQTermScriptEngine *script_engine_;  
  FQTermExternalEditor *externalEditor_;
 private:
  void addMenu();

  void setCursorPosition(const QPoint& mousePosition);
  //set cursor type according to the content
  //show ip location info if openUrlCheck is set
  void setCursorType(const QPoint& mousePosition);


  void enterMenuItem();
  void processLClick(const QPoint& cellClicked);

  void startSelecting(const QPoint& mousePosition);
  void onSelecting(const QPoint& mousePosition);
  void finishSelecting(const QPoint& mousePosition);

  void updateSetting(const FQTermParam& param);

  void sendKey(const int key, const Qt::KeyboardModifiers modifier,
               const QString &text);

  bool scriptKeyEvent(QKeyEvent *keyevent);
  bool scriptMouseEvent(QMouseEvent *mouseevent);
  bool scriptWheelEvent(QWheelEvent *wheelevent);

  void writePasting(const QString& content);


signals: 
  //these 2 signals are connected to corresponding slots to 
  //make write thread safe.
  int writeStringSignal(const QString& str);
  int writeRawStringSignal(const QString& str);
public slots:
  //for script
  int writeString(const QString& str) {return externInput(str);}
  int writeRawString(const QString& str);
public:
  void writeString_ts(const QString& str) {emit writeStringSignal(str);}
  void writeRawString_ts(const QString& str) {emit writeRawStringSignal(str);}


    
//python support
#ifdef HAVE_PYTHON
public:
  QString getPythonErrorMessage() {
    return pythonErrorMessage_;
  }
  void runPythonScript();
  void runPythonScriptFile(const QString&);
  
protected:
	bool pythonCallback(const QString &, PyObject*);
	int runPythonFile(const QString& file);
  void initPython(const QString& file);
  void finalizePython();
  //void sendMouseState(int, ButtonState, ButtonState, const QPoint&);
private:
	PyObject *pModule, *pDict;
  bool pythonScriptLoaded_;
  QString pythonErrorMessage_;
#endif

};

}  // namespace FQTerm

#endif  // FQTERM_WINDOW_H

