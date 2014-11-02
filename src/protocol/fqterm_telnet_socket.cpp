#include "fqterm.h"
#include "fqterm_telnet_socket.h"

namespace FQTerm {
FQTermTelnetSocket::FQTermTelnetSocket()
    : FQTermSocket() {
  private_socket_ = new FQTermSocketPrivate();
  FQ_VERIFY(connect(private_socket_, SIGNAL(connected()), this, SIGNAL(connected())));
  FQ_VERIFY(connect(private_socket_, SIGNAL(hostFound()), this, SIGNAL(hostFound())));
  FQ_VERIFY(connect(private_socket_, SIGNAL(connectionClosed()), this, SIGNAL(connectionClosed())));
  FQ_VERIFY(connect(private_socket_, SIGNAL(delayedCloseFinished()),
                    this, SIGNAL(delayedCloseFinished())));
  FQ_VERIFY(connect(private_socket_, SIGNAL(readyRead()), this, SIGNAL(readyRead())));
  FQ_VERIFY(connect(private_socket_, SIGNAL(error(QAbstractSocket::SocketError)), this, SIGNAL(error(QAbstractSocket::SocketError))));
  FQ_VERIFY(connect(private_socket_, SIGNAL(socketState(int)), this, SIGNAL(socketState(int))));
}

FQTermTelnetSocket::~FQTermTelnetSocket() {
  delete private_socket_;
}

void FQTermTelnetSocket::flush() {
  private_socket_->flush();
}

void FQTermTelnetSocket::setProxy(int nProxyType, bool bAuth,
                                  const QString &strProxyHost,
                                  quint16 uProxyPort,
                                  const QString &strProxyUsr,
                                  const QString &strProxyPwd) {
  private_socket_->setProxy(nProxyType, bAuth, strProxyHost, uProxyPort, strProxyUsr,
                            strProxyPwd);
}

void FQTermTelnetSocket::connectToHost(const QString &host, quint16 port) {
  private_socket_->connectToHost(host, port);
}

void FQTermTelnetSocket::close() {
  private_socket_->close();
}

QByteArray FQTermTelnetSocket::readBlock(unsigned long maxlen) {
  return private_socket_->readBlock(maxlen);
}

long FQTermTelnetSocket::writeBlock(const QByteArray &data) {
  return private_socket_->writeBlock(data);
}

unsigned long FQTermTelnetSocket::bytesAvailable() {
  return private_socket_->bytesAvailable();
}

}
