#include "fqterm_socket.h"

namespace FQTerm {

class FQTermTelnetSocket: public FQTermSocket {
 private:
  FQTermSocketPrivate *private_socket_;

 public:
  FQTermTelnetSocket();

  ~FQTermTelnetSocket();

  void flush();
  void setProxy(int nProxyType, bool bAuth, const QString &strProxyHost,
                quint16 uProxyPort, const QString &strProxyUsr,
                const QString &strProxyPwd);
  void connectToHost(const QString &host, quint16 port);
  void close();
  QByteArray readBlock(unsigned long maxlen);
  long writeBlock(const QByteArray &data);
  unsigned long bytesAvailable();
};


}
