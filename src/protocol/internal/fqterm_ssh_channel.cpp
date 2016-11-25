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

#include "fqterm_ssh_channel.h"
#include "fqterm_ssh_const.h"
#include "fqterm_ssh_packet.h"
#include "fqterm_trace.h"
#include <QString>
namespace FQTerm {

//==============================================================================
//FQTermSSH1Channel
//==============================================================================

FQTermSSH1Channel::FQTermSSH1Channel()
    : FQTermSSHChannel() {
  service_state_ = FQTermSSH1Channel::BEGIN_SERVICE;
}

void FQTermSSH1Channel::initChannel(FQTermSSHPacketReceiver *packet,
                                    FQTermSSHPacketSender *output,
                                    int col, int row, const QString& termtype) {
  packet_receiver_ = packet;
  packet_sender_ = output;
  packet_receiver_->disconnect(this);
  FQ_VERIFY(connect(packet_receiver_, SIGNAL(packetAvaliable(int)), this, SLOT(handlePacket(int))));
  packet_sender_->startPacket(SSH1_CMSG_REQUEST_PTY);
  // pty request is of no use in BBS, but we do this
  packet_sender_->putString(termtype.toLatin1());
  packet_sender_->putInt(row);  // FIXME: hardcoded term size.
  packet_sender_->putInt(col);
  packet_sender_->putInt(0);
  packet_sender_->putInt(0);
  packet_sender_->putByte(0);
  packet_sender_->write();
  service_state_ = FQTermSSH1Channel::REQPTY_SENT;

  is_closed_ = false;
}

void FQTermSSH1Channel::changeTermSize(int col, int row) {
  packet_sender_->startPacket(SSH1_CMSG_WINDOW_SIZE);
  packet_sender_->putInt(row);
  packet_sender_->putInt(col);
  packet_sender_->putInt(0);
  packet_sender_->putInt(0);
  packet_sender_->write();
}

void FQTermSSH1Channel::sendData(const char *data, int len) {
  packet_sender_->startPacket(SSH1_CMSG_STDIN_DATA);
  packet_sender_->putInt(len);
  packet_sender_->putRawData(data, len);
  packet_sender_->write();
}

void FQTermSSH1Channel::closeConnection(const char *reason) {
  packet_sender_->startPacket(SSH1_MSG_DISCONNECT);
  packet_sender_->putString(reason);
  packet_sender_->write();
  is_closed_ = true;
}

void FQTermSSH1Channel::handlePacket(int type) {
  switch (service_state_) {
    case BEGIN_SERVICE:
      FQ_TRACE("sshchannel", 0) << "Channel: We should not be here";
      break;
    case REQPTY_SENT:
      if (type != SSH1_SMSG_SUCCESS) {
        emit channelError(tr("Server refused pty allocation!"));
      }
      packet_sender_->startPacket(SSH1_CMSG_EXEC_SHELL);
      packet_sender_->write();
      emit channelOK();
      service_state_ = FQTermSSH1Channel::SERVICE_OK;
      //emit msg to tell window we could process input.
      break;
    case SERVICE_OK:
      switch (type) {
        case SSH1_SMSG_STDOUT_DATA:
        case SSH1_SMSG_STDERR_DATA:
          {
            const char *data = (const char *)packet_receiver_->buffer_->data() + 4;
            int len = packet_receiver_->packetDataLen() - 4;
            emit channelReadyRead(data, len);
          }
          break;
        case SSH1_SMSG_X11_OPEN:
        case SSH1_SMSG_AGENT_OPEN:
        case SSH1_MSG_PORT_OPEN:
          {
            int i = packet_receiver_->getInt();
            packet_sender_->startPacket(SSH1_MSG_CHANNEL_OPEN_FAILURE);
            packet_sender_->putInt(i);
            packet_sender_->write();
          }
          break;
        case SSH1_SMSG_EXIT_STATUS:
          packet_sender_->startPacket(SSH1_CMSG_EXIT_CONFIRMATION);
          packet_sender_->write();
          closeConnection("end");
          is_closed_ = true;
          break;
        case SSH1_SMSG_SUCCESS:
        case SSH1_SMSG_FAILURE:
          break;
        default:
          FQ_TRACE("sshchannel", 0) << "Unimplemented message: "
                                    << service_state_;
      }
  }
}


//==============================================================================
//FQTermSSH2Channel
//==============================================================================
u_int32_t FQTermSSH2Channel::generateChannelID() {
  static u_int32_t id = 0;
  return id++;
}

FQTermSSH2Channel::FQTermSSH2Channel()
    : FQTermSSHChannel(),
      col_(80),
      row_(24),
      termtype_("vt100") {
  channel_id_ = generateChannelID();
  server_channel_id_ = 0xCCCCCCCC;
  local_window_size_ = 0;
  channel_state_ = FQTermSSH2Channel::BEGIN_CHANNEL;
}

void FQTermSSH2Channel::initChannel(FQTermSSHPacketReceiver *packet,
                                    FQTermSSHPacketSender *output,
                                    int col, int row, const QString& termtype) {
  FQ_FUNC_TRACE("ssh2channel", 5);
  col_ = col;
  row_ = row;
  termtype_ = termtype;
  packet_receiver_ = packet;
  packet_sender_ = output;
  packet_receiver_->disconnect(this);
  FQ_VERIFY(connect(packet_receiver_, SIGNAL(packetAvaliable(int)), this, SLOT(handlePacket(int))));

  //    byte      SSH_MSG_CHANNEL_OPEN
  //    string    channel type in US-ASCII only
  //    uint32    sender channel
  //    uint32    initial window size
  //    uint32    maximum packet size
  //    ....      channel type specific data follows

  packet_sender_->startPacket(SSH2_MSG_CHANNEL_OPEN);
  packet_sender_->putString("session");
  packet_sender_->putInt(channel_id_);
  packet_sender_->putInt(MAX_LOCAL_WINDOW_SIZE);  // TODO: what's the best window size?
  packet_sender_->putInt(MAX_LOCAL_PACKET_SIZE);  // TODO: what's the best maximum packet size?
  packet_sender_->write();

  server_window_size_ = 0;
  server_max_packet_size_ = 0;
  local_window_size_ = MAX_LOCAL_WINDOW_SIZE;

  is_closed_ = false;
}

void FQTermSSH2Channel::changeTermSize(int col, int row) {
  //    byte      SSH_MSG_CHANNEL_REQUEST
  //    uint32    recipient channel
  //    string    "window-change"
  //    boolean   FALSE
  //    uint32    terminal width, columns
  //    uint32    terminal height, rows
  //    uint32    terminal width, pixels
  //    uint32    terminal height, pixels

  packet_sender_->startPacket(SSH2_MSG_CHANNEL_REQUEST);
  packet_sender_->putInt(server_channel_id_);
  packet_sender_->putString("window-change");
  packet_sender_->putByte(false);
  packet_sender_->putInt(col);
  packet_sender_->putInt(row);
  packet_sender_->putInt(640);  // FIXME: hard-coded screen pixels.
  packet_sender_->putInt(480);
  packet_sender_->write();
}

void FQTermSSH2Channel::sendData(const char *data, int len) {
  if (len > (int)server_window_size_ || len > (int)server_max_packet_size_) {
    FQ_TRACE("ssh2channel", 3) << "Data length is greater than server's capacity: "
                               << "data length: " << len << ", "
                               << "server window: " << server_window_size_ << ", "
                               << "server packet max size: " << server_max_packet_size_;
    return;
  }

  //    byte      SSH_MSG_CHANNEL_DATA
  //    uint32    recipient channel
  //    string    data
  packet_sender_->startPacket(SSH2_MSG_CHANNEL_DATA);
  packet_sender_->putInt(server_channel_id_);
  packet_sender_->putString(data, len);
  packet_sender_->write();

  server_window_size_ -= len;;

  FQ_TRACE("ssh2channel", 5) << len
                             << " bytes data sent, server window size left: "
                             << server_window_size_;;
}

void FQTermSSH2Channel::closeConnection(const char *reason) {
  packet_sender_->startPacket(SSH1_MSG_DISCONNECT);
  packet_sender_->putString(reason);
  packet_sender_->write();
  is_closed_ = true;
}

void FQTermSSH2Channel::requestPty() {
  if (packet_receiver_->packetType() == SSH2_MSG_CHANNEL_OPEN_FAILURE) {
    // TODO: Here the error reason in the packet is ignored.
    emit channelError(tr("Server refuces to open a channel."));
    return;
  }

  if (packet_receiver_->packetType() != SSH2_MSG_CHANNEL_OPEN_CONFIRMATION) {
    emit channelError(tr("Server error when opening a channel."));
    return;
  }

  FQ_TRACE("ssh2channel", 5) << "Channel open. Try to request a pty.";

  //    byte      SSH_MSG_CHANNEL_OPEN_CONFIRMATION
  //    uint32    recipient channel
  //    uint32    sender channel
  //    uint32    initial window size
  //    uint32    maximum packet size
  //    ....      channel type specific data follows
  packet_receiver_->consume(4);
  server_channel_id_ = packet_receiver_->getInt();
  server_window_size_ = packet_receiver_->getInt();
  server_max_packet_size_ = packet_receiver_->getInt();

  //    byte      SSH_MSG_CHANNEL_REQUEST
  //    uint32    recipient channel
  //    string    "pty-req"
  //    boolean   want_reply
  //    string    TERM environment variable value (e.g., vt100)
  //    uint32    terminal width, characters (e.g., 80)
  //    uint32    terminal height, rows (e.g., 24)
  //    uint32    terminal width, pixels (e.g., 640)
  //    uint32    terminal height, pixels (e.g., 480)
  //    string    encoded terminal modes
  packet_sender_->startPacket(SSH2_MSG_CHANNEL_REQUEST);
  packet_sender_->putInt(server_channel_id_);
  packet_sender_->putString("pty-req");
  packet_sender_->putByte(true);
  packet_sender_->putString(termtype_.toLatin1()); // TODO: hardcoded term type.
  packet_sender_->putInt(col_);   // FIXME: hardcoded screen parameters.
  packet_sender_->putInt(row_);
  packet_sender_->putInt(640);
  packet_sender_->putInt(480);
  packet_sender_->putString("");  // TODO: no modes sent.
  packet_sender_->write();
}


void FQTermSSH2Channel::requestShell() {
  if (packet_receiver_->packetType() != SSH2_MSG_CHANNEL_SUCCESS) {
    emit channelError(tr("Server refused pty allocation!"));
  }
  FQ_TRACE("ssh2channel", 5) << "Pty allocated. Now try to request a shell.";
  //    byte      SSH_MSG_CHANNEL_REQUEST
  //    uint32    recipient channel
  //    string    "shell"
  //    boolean   want reply
  packet_sender_->startPacket(SSH2_MSG_CHANNEL_REQUEST);
  packet_sender_->putInt(server_channel_id_);
  packet_sender_->putString("shell");
  packet_sender_->putByte(true);
  packet_sender_->write();
}

void FQTermSSH2Channel::checkLocalWindowSize() {
  if (local_window_size_ < MAX_LOCAL_WINDOW_SIZE/2) {
    //    byte      SSH_MSG_CHANNEL_WINDOW_ADJUST
    //    uint32    recipient channel
    //    uint32    bytes to add
    int inc = MAX_LOCAL_WINDOW_SIZE/2;
    local_window_size_ += inc;

    packet_sender_->startPacket(SSH2_MSG_CHANNEL_WINDOW_ADJUST);
    packet_sender_->putInt(server_channel_id_);
    packet_sender_->putInt(inc);
    packet_sender_->write();
  }
}

void FQTermSSH2Channel::processChannelPacket() {
  switch (packet_receiver_->packetType()) {
    case SSH2_MSG_CHANNEL_WINDOW_ADJUST:
      {
        //    byte      SSH_MSG_CHANNEL_WINDOW_ADJUST
        //    uint32    recipient channel
        //    uint32    bytes to add
        packet_receiver_->consume(4);  // channel id is already checked.
        int inc = packet_receiver_->getInt();
        server_window_size_ += inc;
        FQ_TRACE("ssh2channel", 5) << "Server window size increased from "
                                   << server_window_size_ - inc
                                   << " to " << server_window_size_;
      }
      break;
    case  SSH2_MSG_CHANNEL_DATA:
      {
        //    byte      SSH_MSG_CHANNEL_DATA
        //    uint32    recipient channel
        //    string    data
        packet_receiver_->consume(4);
        int len = packet_receiver_->getInt();
        const char *data = (const char *)packet_receiver_->buffer_->data();
        local_window_size_ -= len;
        checkLocalWindowSize();
        emit channelReadyRead(data, len);
      }
      break;
    case  SSH2_MSG_CHANNEL_EXTENDED_DATA:
      {
        //    byte      SSH_MSG_CHANNEL_DATA
        //    uint32    recipient channel
        //    string    data
        packet_receiver_->consume(4);
        int len = packet_receiver_->getInt();
        const char *data = (const char *)packet_receiver_->buffer_->data();
        local_window_size_ -= len;
        checkLocalWindowSize();
        emit channelReadyRead(data, len);
      }
      break;
    case SSH2_MSG_CHANNEL_EOF:
    case SSH2_MSG_CHANNEL_CLOSE:
      //    byte      SSH_MSG_CHANNEL_EOF
      //    uint32    recipient channel
      // FIXME: this error would cause the connection closed, while only the channel need be closed in ssh2.
	    packet_sender_->startPacket(SSH2_MSG_CHANNEL_CLOSE);
	    packet_sender_->putInt(server_channel_id_);
	    packet_sender_->write();
	    emit channelClosed();
	    break;
    case SSH2_MSG_CHANNEL_REQUEST:
      //    byte      SSH_MSG_CHANNEL_REQUEST
      //    uint32    recipient channel
      //    string    "xon-xoff"
      //    boolean   FALSE
      //    boolean   client can do

      // TODO: just ignore this message currently.
      break;
  }
}

void FQTermSSH2Channel::handlePacket(int type) {
  // first check the channel id.
  u_int32_t channel_id = ntohu32(packet_receiver_->buffer_->data());
  if (channel_id != channel_id_) {
    return;
  }

  switch (channel_state_) {
    case FQTermSSH2Channel::BEGIN_CHANNEL:
      requestPty();
      channel_state_ = FQTermSSH2Channel::REQUEST_PTY_SENT;
      break;
    case FQTermSSH2Channel::REQUEST_PTY_SENT:
      requestShell();
      channel_state_ = FQTermSSH2Channel::REQUEST_SHELL_SENT;
      break;
    case FQTermSSH2Channel::REQUEST_SHELL_SENT:
      switch (type) {
        case SSH2_MSG_CHANNEL_REQUEST:
          {
            FQ_TRACE("ssh2channel", 8) << "SSH2_MSG_CHANNEL_REQUEST isn't supported, just send back a packet with SSH2_MSG_CHANNEL_SUCCESS if reply is needed.";

            packet_receiver_->consume(4);
            u_int32_t len = ntohu32(packet_receiver_->buffer_->data());
            packet_receiver_->consume(4 + len);
            bool replyNeeded = packet_receiver_->getByte();

            if (replyNeeded) {
              packet_sender_->startPacket(SSH2_MSG_CHANNEL_SUCCESS);
              packet_sender_->putInt(server_channel_id_);
              packet_sender_->write();
            }
          }
          break;
        case SSH2_MSG_CHANNEL_WINDOW_ADJUST:
          {
            packet_receiver_->consume(4);
            u_int32_t inc = packet_receiver_->getInt();
            server_window_size_ += inc;
          }
          break;
        case SSH2_MSG_CHANNEL_SUCCESS:
          emit channelOK();
          channel_state_ = FQTermSSH2Channel::CHANNEL_OK;
          //emit a msg to tell window we could process input.
          break;
        case SSH2_MSG_CHANNEL_FAILURE:
          emit channelError(tr("Can't open a shell."));
          break;
        default:
          emit channelError(tr("Unsupported packet."));
          break;
      }
      break;
    case FQTermSSH2Channel::CHANNEL_OK:
      processChannelPacket();
      break;
  }
}

}  // namespace FQTerm

#include "fqterm_ssh_channel.moc"
