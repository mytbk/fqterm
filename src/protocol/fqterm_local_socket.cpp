#include "fqterm.h"
#include "fqterm_local_socket.h"

namespace FQTerm {

FQTermLocalSocket::FQTermLocalSocket()
{
  shell_process_ = new QProcess();
  shell_process_->setProcessChannelMode(QProcess::MergedChannels);
  FQ_VERIFY(connect(shell_process_, SIGNAL(started()), this, SIGNAL(connected())));
  FQ_VERIFY(connect(shell_process_, SIGNAL(stateChanged(QProcess::ProcessState)), this , SLOT(stateChanged(QProcess::ProcessState))));
  FQ_VERIFY(connect(shell_process_, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finished(int, QProcess::ExitStatus))));
  //FQ_VERIFY(connect(shell_process_, SIGNAL(delayedCloseFinished()), this, SIGNAL(delayedCloseFinished())));
  FQ_VERIFY(connect(shell_process_, SIGNAL(readyRead()), this, SIGNAL(readyRead())));
  //TODO: Error
  //FQ_VERIFY(connect(shell_process_, SIGNAL(error(QAbstractSocket::SocketError)), this, SIGNAL(error(QAbstractSocket::SocketError))));
  //FQ_VERIFY(connect(shell_process_, SIGNAL(socketState(int)), this, SIGNAL(socketState(int))));
}

FQTermLocalSocket::~FQTermLocalSocket()
{
  delete shell_process_;
}

void FQTermLocalSocket::connectToHost( const QString &host, quint16 port )
{
  if (shell_bin_!=NULL) {
    shell_process_->start(FQTermLocalSocket::shell_bin_->arg(QString::number(port), host),
                          QIODevice::ReadWrite | QIODevice::Unbuffered);
  } else {
    emit connectionClosed();
  }
}

void FQTermLocalSocket::close()
{
  shell_process_->close();
}

QByteArray FQTermLocalSocket::readBlock( unsigned long maxlen )
{
  return shell_process_->read(maxlen);
}

long FQTermLocalSocket::writeBlock( const QByteArray &data )
{
  int count = shell_process_->write(data);

  //char c;
//  shell_process_->getChar(&c);
//  shell_process_->ungetChar(c);
  if (bytesAvailable())
  {
    emit readyRead();
  }
  return count;
}

unsigned long FQTermLocalSocket::bytesAvailable()
{
  return shell_process_->bytesAvailable();
}

void FQTermLocalSocket::finished( int exitCode, QProcess::ExitStatus exitStatus )
{
  emit connectionClosed();
}
void FQTermLocalSocket::stateChanged(QProcess::ProcessState newState)
{
  switch(newState)
  {
  case QProcess::NotRunning:
    break;
  case QProcess::Starting:
    emit hostFound();
    //    shell_process_->write("dir\n");
    break;
  case QProcess::Running:
    break;

  }
}

}

#include "fqterm_local_socket.moc"
