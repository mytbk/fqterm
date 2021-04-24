// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FQTERM_SSH_AUTH_H
#define FQTERM_SSH_AUTH_H

#include <QObject>

class QString;

namespace FQTerm {

class FQTermSSHPacketReceiver;
class FQTermSSHPacketSender;

class FQTermSSHAuth: public QObject {

  Q_OBJECT;
protected:
  QString user_name_;
  FQTermSSHPacketReceiver *packet_receiver_;
  FQTermSSHPacketSender *packet_sender_;

public:
  FQTermSSHAuth(const char *sshuser = NULL)
    : user_name_(sshuser) {
  }

  ~FQTermSSHAuth() {}

  virtual void initAuth(FQTermSSHPacketReceiver *packet,
                        FQTermSSHPacketSender *output) = 0;
public slots:
  virtual void handlePacket(int type) = 0;

signals:
  void requestUserPwd(QString *user, QString *pwd, bool *isOK);
  void authOK();
  void authError(QString);
};

class FQTermSSHPasswdAuth: public FQTermSSHAuth {
  Q_OBJECT;
protected:
  QString passwd_;
  bool is_tried_;

  QString default_user_;
  QString default_passwd_;
  
public:
  FQTermSSHPasswdAuth(const char *sshuser, const char *sshpasswd)
    : FQTermSSHAuth(sshuser),
      passwd_(sshpasswd),
      default_user_(sshuser),
      default_passwd_(sshpasswd) {
  }
};

class FQTermSSH1PasswdAuth: public FQTermSSHPasswdAuth {
  Q_OBJECT;
private:
  enum FQTermSSH1PasswdAuthState {
    BEFORE_AUTH, USER_SENT, PASS_SENT, AUTH_OK
  }	ssh_pw_auth_state_;

public:
  FQTermSSH1PasswdAuth(const char *sshuser, const char *sshpasswd);

public slots:
  void handlePacket(int type);
  void initAuth(FQTermSSHPacketReceiver *packet, FQTermSSHPacketSender *output);
};

class FQTermSSH2PasswdAuth: public FQTermSSHPasswdAuth {
  Q_OBJECT;
private:
  enum FQTermSSH2PasswdAuthState {
    SERVICE_ACCEPTED, USER_PASSWD_SENT, PASS_SENT, AUTH_OK
  }	ssh_pw_auth_state_;

  void sendUserPasswd();
  bool check();

public:
  FQTermSSH2PasswdAuth(const char *sshuser, const char *sshpasswd);

public slots:
  void handlePacket(int type);
  void initAuth(FQTermSSHPacketReceiver *packet, FQTermSSHPacketSender *output);
};

}  // namespace FQTerm

#endif  // FQTERM_SSH_AUTH_H
