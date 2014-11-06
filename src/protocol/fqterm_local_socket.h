#ifndef FQTERM_LOCAL_SOCKET_H
#define FQTERM_LOCAL_SOCKET_H

#include "fqterm_socket.h"

namespace FQTerm {

class FQTermLocalSocket: public FQTermSocket {
  Q_OBJECT;
private:
  QProcess* shell_process_;
public:
  static QString *shell_bin_;

  FQTermLocalSocket();
  ~FQTermLocalSocket();
  void flush(){}
  void setProxy(int nProxyType, bool bAuth, const QString &strProxyHost,
                quint16 uProxyPort, const QString &strProxyUsr,
                const QString &strProxyPwd){}
  void connectToHost(const QString &host, quint16 port);
  void close();
  QByteArray readBlock(unsigned long maxlen);
  long writeBlock(const QByteArray &data);
  unsigned long bytesAvailable();

public slots:
  void stateChanged(QProcess::ProcessState newState);
  void finished (int exitCode, QProcess::ExitStatus exitStatus);

};

}

#endif
