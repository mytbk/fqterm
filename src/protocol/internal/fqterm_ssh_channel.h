// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FQTERM_SSH_CHANNEL_H
#define FQTERM_SSH_CHANNEL_H

#include "fqterm_ssh_types.h"

#include <QObject>

namespace FQTerm {

class FQTermSSHPacketReceiver;
class FQTermSSHPacketSender;

class FQTermSSHChannel: public QObject {
  Q_OBJECT;
protected:
  bool is_closed_;

  FQTermSSHPacketReceiver *packet_receiver_;
  FQTermSSHPacketSender *packet_sender_;

public:
  FQTermSSHChannel(){}
  virtual ~FQTermSSHChannel(){}
  virtual void initChannel(FQTermSSHPacketReceiver *packet,
                           FQTermSSHPacketSender *output, 
                           int col, int row, const QString& termtype) = 0;
  virtual void closeConnection(const char *reason) = 0;

  // TODO: it seems this function isn't used.
  virtual void changeTermSize(int col, int row) = 0;
  
  virtual void sendData(const char *data, int len) = 0;

public slots:
  virtual void handlePacket(int type) = 0;
signals:
  void channelOK();
  void channelReadyRead(const char *data, int len);
  void channelError(QString);
  void channelClosed();
};

class FQTermSSH1Channel: public FQTermSSHChannel {
  Q_OBJECT;
private:
  enum FQTermSSH1ChannelState {
    BEGIN_SERVICE, REQPTY_SENT, 
    //REQCMD_SENT,
    SERVICE_OK
  }	service_state_;

public:
  FQTermSSH1Channel();
  virtual void initChannel(FQTermSSHPacketReceiver *packet, FQTermSSHPacketSender *output,
                          int col, int row, const QString& termtype);
  virtual void closeConnection(const char *reason);
  virtual void changeTermSize(int col, int row);
  virtual void sendData(const char *data, int len);

public slots:
  virtual void handlePacket(int type);
signals:
  void channelReadyRead(const char *data, int len);
};


class FQTermSSH2Channel: public FQTermSSHChannel {
  Q_OBJECT;
private:
  static u_int32_t generateChannelID();

private:
  int col_;
  int row_;
  QString termtype_;
  enum FQTermSSH2ChannelState {
    BEGIN_CHANNEL, REQUEST_PTY_SENT, REQUEST_SHELL_SENT, CHANNEL_OK
  }	channel_state_;

  u_int32_t channel_id_;

  enum {MAX_LOCAL_WINDOW_SIZE = 0x100000, MAX_LOCAL_PACKET_SIZE = 0x4000};
  
  u_int32_t local_window_size_;

  u_int32_t server_channel_id_;
  u_int32_t server_window_size_; // connection packet window size;
  u_int32_t server_max_packet_size_; // max size of each packet sent to server.

  void requestPty();
  void requestShell();
  void processChannelPacket();
  void checkLocalWindowSize();

public:
  FQTermSSH2Channel();
  virtual void initChannel(FQTermSSHPacketReceiver *packet, FQTermSSHPacketSender *output,
                          int col, int row, const QString& termtype);
  virtual void closeConnection(const char *reason);
  virtual void changeTermSize(int col, int row);
  virtual void sendData(const char *data, int len);

public slots:
  virtual void handlePacket(int type);
signals:
  void channelReadyRead(const char *data, int len);
};

}  // namespace FQTerm

#endif  // FQTERM_SSH_CHANNEL_H
