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

#include "state.h"
#include "fqterm.h"
#include <QApplication>
#include <algorithm>
#include "statebuilder.h"
namespace FQTerm{

void FQTermUniteSessionContext::write(const QByteArray& str) {
  session_->write(str);
}

FQTermUniteSessionContext::FQTermUniteSessionContext(FQTermUniteSession* session) : QObject(session), session_(session) {
  stateTable_[WELCOME] = new FQTermWelcomeState(this);
  stateTable_[EXITING] = new FQTermExitState(this);
  stateTable_[READING] = new FQTermReadingState(this);
  stateTable_[HELP] = new FQTermHelpState(this);
  setCurrentState(WELCOME);
}

void FQTermUniteSessionContext::processInput(const QByteArray& input) {
  inputBuffer_ += input;
  while(true) {
    int consumed = currentState_->processInput(inputBuffer_);
    inputBuffer_ = inputBuffer_.mid(consumed);
    if (!consumed || inputBuffer_.size() == 0) {
      break;
    }
  }
}

void FQTermUniteSessionContext::quit() {
  session_->quit();
}



FQTermUniteState* FQTermUniteSessionContext::setCurrentState(int state, bool init /*= true*/) {
  StateTableIterator it = stateTable_.find(state);
  if (it != stateTable_.end()) {
    currentState_ = it->second;
    if (init)
      currentState_->initialize();
  }
  return currentState_;
}



FQTermUniteSession::FQTermUniteSession(int socketDescriptor ) : context_(NULL) {
  moveToThread(this);
  socket_ = new QTcpSocket;
  FQ_VERIFY(connect(socket_, SIGNAL(readyRead()),
    this, SLOT(readyRead()),Qt::DirectConnection));
  FQ_VERIFY(connect(socket_, SIGNAL(disconnected()),
    this, SLOT(disconnected()),Qt::DirectConnection));
  FQ_VERIFY(connect(socket_, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
    this, SLOT(stateChanged(QAbstractSocket::SocketState)),Qt::DirectConnection));
  FQ_VERIFY(connect(socket_, SIGNAL(error(QAbstractSocket::SocketError)),
    this, SLOT(error(QAbstractSocket::SocketError)),Qt::DirectConnection));

  //  FQ_VERIFY(connect(socket_, SIGNAL(connected()),
  //    this, SLOT(welcome()),Qt::DirectConnection));

  socket_->setSocketDescriptor(socketDescriptor);
  socket_->moveToThread(this);
  setTerminationEnabled();

  StateBuilder sb;
  sb.BuildState();
}


FQTermUniteSession::~FQTermUniteSession() {
  socket_->close();
  delete socket_;
  socket_ = NULL;
}

void FQTermUniteSession::welcome()
{
  context_ = new FQTermUniteSessionContext(this);
}

void FQTermUniteSession::readyRead() {
  if (context_) {
    context_->processInput(socket_->readAll());
  }
}

void FQTermUniteSession::write( const QByteArray& output ) {
  socket_->write(output);
}

void FQTermUniteSession::run() {
  welcome();
  exec();
  disconnected();
}


void FQTermUniteSession::disconnected() {
  if (!isRunning())
    return;
  quit();
  moveToThread(QApplication::instance()->thread());
  deleteLater();  
}

void FQTermUniteSession::stateChanged( QAbstractSocket::SocketState socketState ) {
}

void FQTermUniteSession::error(QAbstractSocket::SocketError socketError) {
}


}

#include "session.moc"