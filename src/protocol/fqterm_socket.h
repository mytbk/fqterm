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

#ifndef FQTERM_SOCKET_H
#define FQTERM_SOCKET_H

// _OS_X_ not defined if i dont include it
#include <QGlobalStatic>

// different
#if defined(Q_OS_WIN32) || defined(_OS_WIN32_)

#include <winsock2.h>

#elif defined(Q_OS_BSD4) || defined(_OS_FREEBSD_)   \
  || defined(Q_OS_MACX) || defined(Q_OS_DARWIN)

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#else

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#endif

#include <QAbstractSocket>
#include <QString>
#include <QStringList>
#include <QProcess>

class QTcpSocket;
namespace FQTerm {
/*
 * Socket with proxy support. 
 *
 */
class FQTermSocketPrivate: public QObject {

  Q_OBJECT;
 public:
  FQTermSocketPrivate(QObject *parent_ = 0);

  ~FQTermSocketPrivate();

  void flush();
  void setProxy(int nProxyType, bool bAuth, const QString &strProxyHost,
                quint16 uProxyPort, const QString &strProxyUsr,
                const QString &strProxyPwd);
  void connectToHost(const QString &hostname, quint16 portnumber);
  void close();
  QByteArray readBlock(unsigned long maxlen);
  long writeBlock(const QByteArray &data);
  unsigned long bytesAvailable();

 signals:
  void connected();
  void hostFound();
  void connectionClosed();
  void delayedCloseFinished();
  void readyRead();
  void error(QAbstractSocket::SocketError);
  void socketState(int);

 protected slots:
  void socketConnected();
  void socketReadyRead();

 protected:
  // socks5 function
  void socks5_connect();
  void socks5_auth();
  void socks5_reply(const QByteArray &, int);

 private:
  // proxy
  int proxy_type;
  QString proxy_host;
  QString proxy_usr;
  quint16 proxy_port;
  QString proxy_pwd;
  QString host;
  quint16 port;
  int proxy_state;
  bool bauth;

  struct sockaddr_in addr_host;

  QTcpSocket *m_socket;
};

// Virtual base class for FQTermTelnetSocket and FQTermSSHSocket
class FQTermSocket: public QObject {

  Q_OBJECT;
 public:
  FQTermSocket(QObject *parent = 0): QObject(parent) {}

  virtual ~FQTermSocket() {}

  virtual void flush() = 0;
  virtual void setProxy(int nProxyType, bool bAuth,
                        const QString &strProxyHost,
                        quint16 uProxyPort,
                        const QString &strProxyUsr,
                        const QString &strProxyPwd) =	0;
  virtual void connectToHost(const QString &host, quint16 port) = 0;
  virtual void close() = 0;
  virtual QByteArray readBlock(unsigned long maxlen) = 0;
  virtual long writeBlock(const QByteArray &data) = 0;
  virtual unsigned long bytesAvailable() = 0;
  virtual bool readyForInput() {return true;}
  virtual bool setTermSize(int col, int row) {return 0;}
 signals:
  void sshAuthOK();
  void connected();
  void hostFound();
  void connectionClosed();
  void delayedCloseFinished();
  void readyRead();
  void error(QAbstractSocket::SocketError);
  void errorMessage(QString);
  void socketState(int);
  void requestUserPwd(QString *user, QString *pwd, bool *isOK);
};


}  // namespace FQTerm

#endif  // FQTERM_SOCKET_H
