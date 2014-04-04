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
#ifndef FQTERM_UNITE_SESSION
#define FQTERM_UNITE_SESSION

#include <QObject>
#include <QThread>
#include <QTcpServer>
#include <QTcpSocket>
#include <map>
namespace FQTerm {

class FQTermUniteState;
class FQTermUniteSession;
class FQTermUniteSessionContext : public QObject {
  Q_OBJECT;
public:
  FQTermUniteSessionContext(FQTermUniteSession* session);
  void write(const QByteArray& str);
  void processInput(const QByteArray& input);
  void quit();
  enum STATE{WELCOME = 0, EXITING = 1, READING = 2, HELP = 3};
  FQTermUniteState* setCurrentState(int state, bool init = true);

  int row() {return 24;}
  int column() {return 80;}
private:
  FQTermUniteSession* session_;
  FQTermUniteState* currentState_;
  QByteArray inputBuffer_;
  typedef std::map<int, FQTermUniteState*> StateTable;
  typedef std::map<int, FQTermUniteState*>::iterator StateTableIterator;
  StateTable stateTable_;
};


class FQTermUniteSession : public QThread {
  Q_OBJECT;
public:
  FQTermUniteSession(int socketDescriptor);
  ~FQTermUniteSession();
  void write(const QByteArray& output);

protected:
  virtual void run();
  protected slots:
    void readyRead();
    void disconnected();
    void stateChanged(QAbstractSocket::SocketState socketState);
    void error(QAbstractSocket::SocketError socketError);
    void welcome();
private:
  QTcpSocket* socket_;
  FQTermUniteSessionContext* context_;

};

} //namespace FQTerm

#endif //FQTERM_UNITE_USER