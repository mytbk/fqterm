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

#include "fqterm_mini_server.h"
#include "fqterm_trace.h"
#include "fqterm_path.h"
#include "fqterm.h"
#include <QByteArray>
#include <QTimer>
#include <QFile>
#include <QApplication>
#include <queue>
namespace FQTerm {


void FQTermMiniServerThread::run()
{
  setTerminationEnabled();
  FQTermMiniServer miniServer;
  //miniServer.setMaxPendingConnections(1);
  miniServer.listen(QHostAddress::LocalHost, 35172);
  exec();
  miniServer.close();
  miniServer.finalizeMiniServer();
}

FQTermMiniServer::FQTermMiniServer(QObject * parent)
  : QTcpServer(parent) {
}


FQTermMiniServer::~FQTermMiniServer() {
}

void FQTermMiniServer::incomingConnection( int socketDescriptor ) {
  FQTermUniteSession* session = new FQTermUniteSession(socketDescriptor);
  FQ_VERIFY(connect(this, SIGNAL(quit()), session, SLOT(quit()), Qt::QueuedConnection));
  session->start();
}

void FQTermMiniServer::finalizeMiniServer() {
  emit quit();
}








} //namespace FQTerm

#include "fqterm_mini_server.moc"
