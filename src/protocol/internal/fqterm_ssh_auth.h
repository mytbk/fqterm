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
