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

#ifndef FQTERM_MINI_SERVER_H
#define FQTERM_MINI_SERVER_H

#include <QThread>
#include <QByteArray>
#include <map>
#include <vector>
#include <algorithm>
#include <QString>
#include "internal/session.h"

namespace FQTerm {


class FQTermMiniServer : public QTcpServer {
  Q_OBJECT;
public:
  FQTermMiniServer( QObject * parent = 0 );
  ~FQTermMiniServer();
  void finalizeMiniServer();
protected:
  void incomingConnection(int socketDescriptor);
signals:
  void quit();
};


class FQTermMiniServerThread : public QThread{
protected:
  virtual void run();

};
//----------------------------------------------------------------------------------
// this for FSM
class FQTermUniteDecode;

//----------------------------------------------------------------------------------


class FQTermUniteSession;
class FQTermUniteState;



//use XML to describe the site.
//each session contains user info, not in the reverse way.
//each session has 1. state (edit, read, menu), 2. strategy (decided by state) 3. context (with which strategy could decide next step. i.e. inner state)
} //namespace FQTerm
#endif
