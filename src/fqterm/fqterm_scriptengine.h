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

#ifndef FQTERM_SCRIPTENGINE_H
#define FQTERM_SCRIPTENGINE_H

#include "fqterm.h"
#include "articledialog.h"
#include "defineescape.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QScriptEngine>
#include <QScriptValue>
#include <QList>
#include <map>
class QString;
class QTimer;
namespace FQTerm {
class FQTermWindow;
class FQTermSession;
class FQTermScreen;
class FQTermBuffer;
class ArticleCopyThread;
class FQTermScriptEngine : public QObject {
  Q_OBJECT;
public:
  FQTermScriptEngine(FQTermWindow* parent);
  ~FQTermScriptEngine();
  QScriptEngine* getQtEngine() {return engine_;}
  void runScript(const QString& filename);
  void stopScript();
  void finalizeScript();

public:
  bool scriptCallback(const QString& func, const QScriptValueList& args);
public slots: //script apis
  //ui functions.
  void msgBox(const QString& msg);
  bool yesnoBox(const QString& msg);
  QString askDialog(const QString& title, const QString& question,
                    const QString& defText);
  
  //bbs ui functions
  int caretX();
  int caretY();
  QString getText(int row);
  QString getTextAt(int row, int column, int len);
  //if we get from a column that contains the second part
  //of a character with width == 2, we could still get it.
  QString getFullTextAt(int row, int column, int len);
  QString getAttrText(int row);
  QString getAttrTextAt(int row, int column, int len);
  QString getFullAttrText(int row, int column, int len);
  void sendString(const QString& str);
  void sendParsedString(const QString& str);
  void serverRedraw();
  void clientRedraw();
  int columns();
  int rows();
  bool isConnected();
  void disconnect();
  void reconnect();
  QString FileDialog();
  QString getBBSCodec();
  QString getAddress();
  int getPort();
  int getProtocol();
  QString getReplyKey();
  QString getURL();
  QString getIP();
  void previewImage(const QString& url);
  void sleep(int ms);
  QString copyArticle();
  void openUrl(const QString & url);
  QString getSelect(bool color_copy);
  QList<int> mapToChar(int screenX, int screenY);
  //next 2 functions are map screen to char for x, y.
  int charX(int screen_x);
  int charY(int screen_y);
  int screenX(int char_x);
  int screenY(int char_y);
  //mouse position is given in screen coordinate.
  int mouseX() {return charX(mouseSX());}
  int mouseY() {return charY(mouseSY());}

  int mouseSX();
  int mouseSY();

  void setMenuRect(int row, int col, int len);

  bool importFile(const QString& filename);

  bool isAntiIdle();
  bool isAutoReply();
  //auxiliary functions.
  //should be move to some other class.
  //qt script provides so poor extensions to js.
  void writeFile(const QString& filename, const QString& str);
  void appendFile(const QString& filename, const QString& str);
  QString readFile(const QString& filename);
  QStringList readFolder(const QString& path);
  void artDialog(const QString &content);
  QString platform();
  bool makePath(const QString& path);
  QString newLine();

  //Timer
  int setInterval(int ms, const QScriptValue& func);
  void clearInterval(int id);
  int setTimeout(int ms, const QScriptValue& func);
  void clearTimeout(int id);

  int getUIEventInterval();
  void setUIEventInterval(int ms);
private:
  int createTimer(int ms, const QScriptValue& func, bool singleShot);
  void destroyTimer(int id);

private slots:
  void articleCopied(int state, const QString content);
private:
  QScriptEngine* engine_;
  FQTermWindow* window_;
  FQTermSession* session_;
  FQTermScreen* screen_;
  FQTermBuffer* buffer_;
  ArticleCopyThread* articleCopyThread_;
  bool articleCopied_;
  QString articleText_;
  std::map<int, QTimer*> timerTable_;
  int timerIDCount_;
};


}//namespace FQTerm
#endif //FQTERM_SCRIPTENGINE_H
