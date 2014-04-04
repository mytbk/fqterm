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

#include <QString>

#include "fqterm_ssh_auth.h"
#include "fqterm_ssh_packet.h"
#include "fqterm_ssh_const.h"
#include "fqterm_trace.h"

namespace FQTerm {

//==============================================================================
//FQTermSSH1PasswdAuth
//==============================================================================

FQTermSSH1PasswdAuth::FQTermSSH1PasswdAuth(const char *sshuser,
                                           const char *sshpasswd)
    : FQTermSSHPasswdAuth(sshuser, sshpasswd) {
  is_tried_ = false;
  ssh_pw_auth_state_ = FQTermSSH1PasswdAuth::BEFORE_AUTH;
}

void FQTermSSH1PasswdAuth::initAuth(FQTermSSHPacketReceiver *packet,
                                    FQTermSSHPacketSender *output) {
  packet_receiver_ = packet;
  packet_sender_ = output;
  packet_receiver_->disconnect(this);
  FQ_VERIFY(connect(packet_receiver_, SIGNAL(packetAvaliable(int)),
                    this, SLOT(handlePacket(int))));
  packet_sender_->startPacket(SSH1_CMSG_USER);
  user_name_.clear();
  passwd_.clear();

  if (!default_user_.isEmpty() && !default_passwd_.isEmpty()) {
    user_name_ = default_user_;
    passwd_ = default_passwd_;
  }
  
  while (user_name_.isEmpty()) {
    bool isOK = false;
    emit requestUserPwd(&user_name_, &passwd_, &isOK);
    // SSHLoginDialog login(&d_user, &d_passwd);
    if (!isOK) {
      emit authError(tr("UserCancel"));
      return ;
    }
  }
  
  packet_sender_->putString(user_name_.toLatin1());
  packet_sender_->write();
  ssh_pw_auth_state_ = USER_SENT;
  is_tried_ = false;
}

void FQTermSSH1PasswdAuth::handlePacket(int type) {
  switch (ssh_pw_auth_state_) {
    case BEFORE_AUTH:
      FQ_TRACE("sshauth", 0) << "Auth: We should not be here.";
      break;
    case USER_SENT:
      if (type == SSH1_SMSG_SUCCESS) {
        ssh_pw_auth_state_ = AUTH_OK;
        emit authOK();
        break;
      }
      if (type != SSH1_SMSG_FAILURE) {
        emit authError(tr("Strange response from server"));
        break;
      }
      if (is_tried_) {
        bool isOK = false;
        emit requestUserPwd(&user_name_, &passwd_, &isOK);
        // SSHLoginDialog login(&d_user, &d_passwd);
        if (!isOK) {
          emit authError(tr("User canceled"));
          break;
        }
        is_tried_ = false;
      }
      packet_sender_->startPacket(SSH1_CMSG_AUTH_PASSWORD);
      packet_sender_->putString(passwd_.toLatin1());
      packet_sender_->write();
      is_tried_ = true;
      break;
    case AUTH_OK:
      break;
    default:
      return ;
  }
}

//==============================================================================
//FQTermSSH2PasswdAuth
//==============================================================================

FQTermSSH2PasswdAuth::FQTermSSH2PasswdAuth(const char *sshuser,
                                           const char *sshpasswd)
    : FQTermSSHPasswdAuth(sshuser, sshpasswd) {
  is_tried_ = false;
  ssh_pw_auth_state_ = FQTermSSH2PasswdAuth::SERVICE_ACCEPTED;
}

void FQTermSSH2PasswdAuth::initAuth(FQTermSSHPacketReceiver *packet,
                                    FQTermSSHPacketSender *output) {
  packet_receiver_ = packet;
  packet_sender_ = output;
  packet_receiver_->disconnect(this);
  FQ_VERIFY(connect(packet_receiver_, SIGNAL(packetAvaliable(int)),
                    this, SLOT(handlePacket(int))));

  packet_sender_->startPacket(SSH2_MSG_SERVICE_REQUEST);
  packet_sender_->putString("ssh-userauth");
  packet_sender_->write();

  ssh_pw_auth_state_ = FQTermSSH2PasswdAuth::SERVICE_ACCEPTED;
}

void FQTermSSH2PasswdAuth::handlePacket(int type) {
  switch (ssh_pw_auth_state_) {
    case FQTermSSH2PasswdAuth::SERVICE_ACCEPTED:
      sendUserPasswd();
      ssh_pw_auth_state_ = FQTermSSH2PasswdAuth::USER_PASSWD_SENT;
      break;
    case FQTermSSH2PasswdAuth::USER_PASSWD_SENT:
      if (check()) {
        FQ_TRACE("ssh2passwdauth", 3) << "Auth OK.";

        emit authOK();
        ssh_pw_auth_state_ = FQTermSSH2PasswdAuth::AUTH_OK;
      }
      break;
    case FQTermSSH2PasswdAuth::AUTH_OK:
      break;
    default:
      return ;
  }
}

bool FQTermSSH2PasswdAuth::check() {
  switch(packet_receiver_->packetType()) {
    case SSH2_MSG_USERAUTH_SUCCESS:
      return true;
      break;
    case SSH2_MSG_USERAUTH_BANNER:
      // TODO: just ignore banner messages.
      break;    
    case SSH2_MSG_USERAUTH_FAILURE:
      emit authError(tr("Authentication failed!"));
      break;
    default:
      emit authError(tr("Unexpected packet"));
  }

  return false;
}

void FQTermSSH2PasswdAuth::sendUserPasswd() {
  if (packet_receiver_->packetType() != SSH2_MSG_SERVICE_ACCEPT) {
    emit authError(tr("Expect a SSH2_MSG_SERVICE_ACCEPT packet"));
    return;
  }

  u_char *service_name = (u_char *)packet_receiver_->getString();

  if (std::string((char *)service_name) != std::string("ssh-userauth")) {
    emit authError(tr("Error when sending username and password."));
    return;
  }

  delete[] service_name;

  //  byte      SSH_MSG_USERAUTH_REQUEST
  //  string    user name in ISO-10646 UTF-8 encoding [RFC3629]
  //  string    service name in US-ASCII
  //  string    method name in US-ASCII
  //  ....      method specific fields
  user_name_.clear();
  passwd_.clear();

  if (!default_user_.isEmpty() && !default_passwd_.isEmpty()) {
    user_name_ = default_user_;
    passwd_ = default_passwd_;
  }
  
  while (user_name_.isEmpty()) {
    bool isOK = false;
    emit requestUserPwd(&user_name_, &passwd_, &isOK);
    if (!isOK) {
      emit authError(tr("UserCancel"));
      return ;
    }
  }

  packet_sender_->startPacket(SSH2_MSG_USERAUTH_REQUEST);
  packet_sender_->putString(user_name_.toLatin1());
  packet_sender_->putString("ssh-connection");
  packet_sender_->putString("password");
  packet_sender_->putByte(false); 
  packet_sender_->putString(passwd_.toLatin1());
  packet_sender_->write();
}

}  // namespace FQTerm

#include <fqterm_ssh_auth.moc>
