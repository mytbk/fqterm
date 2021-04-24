// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FQTERM_SSH_SOCKET_H
#define FQTERM_SSH_SOCKET_H

#include "fqterm_socket.h"
#include "internal/buffer.h"


namespace FQTerm {


class FQTermSocketPrivate;

class FQTermSSHPacketReceiver;

class FQTermSSHPacketSender;

class FQTermSSHKex;

class FQTermSSHAuth;

class FQTermSSHChannel;


class FQTermSSHSocket: public FQTermSocket {

  Q_OBJECT;
private:
  enum FQTermSSHSocketState {

    BeforeSession, SockSession
  }	ssh_socket_state_;

  FQTermSocketPrivate *private_socket_;

	buffer input_buffer; /* data to read, decrypted */
	buffer output_buffer; /* data to send, unencrypted */
	buffer socket_buffer; /* data received from the socket */


  FQTermSSHPacketReceiver *packet_receiver_;

  FQTermSSHPacketSender *packet_sender_;


  FQTermSSHKex *key_exchanger_;

  FQTermSSHAuth *authentication_;

  FQTermSSHChannel *ssh_channel_;

  bool is_channel_ok_;
  bool auth_ok_emitted_;

  QByteArray init_user_, init_passwd_;

  QString server_name_;

  int ssh_version_;
  int col_;
  int row_;
  QString termtype_;
  int chooseVersion(const QString &ver);
  unsigned long socketWriteBlock(const char *data, unsigned long len);
  void parsePacket();

private slots:
  void handlePacket(int type);
  void writeData();
  void kexOK();
  void authOK();
  void channelOK();
  void channelReadyRead(const char *data, int len);
  void socketReadyRead();
  void handleError(QString);
  
public:
  FQTermSSHSocket(int col = 80, int row = 24, const QString& termtype = "vt100", const char *sshuser = NULL, const char *sshpasswd = NULL);

  ~FQTermSSHSocket();


  void setProxy(int nProxyType,  //0-no proxy; 1-wingate; 2-sock4; 3-socks5
                bool bAuth,  // if authentation needed
                const QString &strProxyHost, quint16 uProxyPort, const QString &strProxyUsr,
                const QString &strProxyPwd);

  void connectToHost(const QString &host_name, quint16 port);

  void init(int ssh_version);

  QByteArray readBlock(unsigned long size);
  long writeBlock(const QByteArray &data);

  virtual bool readyForInput() {return is_channel_ok_;}
  virtual bool setTermSize(int col, int row);
  unsigned long bytesAvailable();

  void flush();
  void close();

};

}  // namespace FQTerm


#endif //FQTERM_SSH_SOCKET_H

