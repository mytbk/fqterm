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

#include "fqterm_ssh_socket.h"
#include "fqterm_ssh_packet.h"
#include "fqterm_ssh1_packet.h"
#include "fqterm_ssh2_packet.h"
#include "fqterm_ssh_kex.h"
#include "fqterm_ssh2_kex.h"
#include "fqterm_ssh_const.h"
#include "fqterm_ssh_auth.h"
#include "fqterm_ssh_channel.h"
#include "fqterm_trace.h"
#include "fqterm_path.h"
#include "ssh_known_hosts.h"
#include <QString>
namespace FQTerm {

#define V1STR	"SSH-1.5-FQTermSSH\n"
#define V2STR	"SSH-2.0-FQTermSSH\n"
#define SSH_V1_C	"SSH-1.5-FQTermSSH"
#define SSH_V2_C	"SSH-2.0-FQTermSSH"

FQTermSSHSocket::FQTermSSHSocket(int col, int row, const QString& termtype, const char *sshuser, const char *sshpasswd)
  : termtype_(termtype) {
  col_ = col;
  row_ = row;
  init_user_ = sshuser;
  init_passwd_ = sshpasswd;

  private_socket_ = new FQTermSocketPrivate();

  buffer_init(&input_buffer);
  buffer_init(&output_buffer);
  buffer_init(&socket_buffer);
  packet_receiver_ = NULL;
  packet_sender_ = NULL;
  key_exchanger_ = NULL;
  authentication_ = NULL;
  ssh_channel_ = NULL;
  ssh_version_ = 1;
  is_channel_ok_ = false;
  auth_ok_emitted_ = false;

  FQ_VERIFY(connect(private_socket_, SIGNAL(hostFound()), this, SIGNAL(hostFound())));
  FQ_VERIFY(connect(private_socket_, SIGNAL(connected()), this, SIGNAL(connected())));
  FQ_VERIFY(connect(private_socket_, SIGNAL(connectionClosed()), this, SIGNAL(connectionClosed())));
  FQ_VERIFY(connect(private_socket_, SIGNAL(delayedCloseFinished()), this, SIGNAL(delayedCloseFinished())));
  FQ_VERIFY(connect(private_socket_, SIGNAL(error(QAbstractSocket::SocketError )), this, SIGNAL(error(QAbstractSocket::SocketError ))));
  FQ_VERIFY(connect(private_socket_, SIGNAL(socketState(int)), this, SIGNAL(socketState(int))));

  FQ_VERIFY(connect(private_socket_, SIGNAL(readyRead()), this, SLOT(socketReadyRead())));
}

FQTermSSHSocket::~FQTermSSHSocket() {
  delete private_socket_;
  buffer_deinit(&input_buffer);
  buffer_deinit(&output_buffer);
  buffer_deinit(&socket_buffer);
  delete packet_receiver_;
  delete packet_sender_;
  delete key_exchanger_;
  delete authentication_;
  delete ssh_channel_;
}

void FQTermSSHSocket::init(int ssh_version)
{
	// Actually we could reuse these buffers, sender/receivers, and etc.
	// but in that case reset methods should be added to all these classes.
	// Guys lazy as me won't do that.

	buffer_clear(&input_buffer);
	buffer_clear(&output_buffer);
	buffer_clear(&socket_buffer);
	delete packet_receiver_;
	delete packet_sender_;
	delete key_exchanger_;
	delete authentication_;
	delete ssh_channel_;

	is_channel_ok_ = false;
	auth_ok_emitted_ = false;

	packet_sender_ = new FQTermSSHPacketSender(ssh_version);
	if (ssh_version == 1) {
		packet_receiver_ = new FQTermSSH1PacketReceiver;
		key_exchanger_ = new FQTermSSH1Kex(SSH_V1_C, server_name_.toLatin1().constData());
		authentication_ = new FQTermSSH1PasswdAuth(init_user_, init_passwd_);
		ssh_channel_ = new FQTermSSH1Channel;
	} else {
		packet_receiver_ = new FQTermSSH2PacketReceiver;
		key_exchanger_ = new FQTermSSH2Kex(SSH_V2_C, server_name_.toLatin1().constData());
		authentication_ = new FQTermSSH2PasswdAuth(init_user_, init_passwd_);
		ssh_channel_ = new FQTermSSH2Channel;
	}
	FQ_VERIFY(connect(packet_receiver_, SIGNAL(packetAvaliable(int)), this, SLOT(handlePacket(int))));
	FQ_VERIFY(connect(packet_receiver_, SIGNAL(packetError(QString)), this, SLOT(handleError(QString))));
	FQ_VERIFY(connect(packet_sender_, SIGNAL(dataToWrite()), this, SLOT(writeData())));
	FQ_VERIFY(connect(key_exchanger_, SIGNAL(kexOK()), this, SLOT(kexOK())));
	FQ_VERIFY(connect(key_exchanger_, SIGNAL(kexError(QString)), this, SLOT(handleError(QString))));
	FQ_VERIFY(connect(key_exchanger_, SIGNAL(reKex()), packet_receiver_, SLOT(resetEncryption())));
	FQ_VERIFY(connect(key_exchanger_, SIGNAL(reKex()), packet_sender_, SLOT(resetEncryption())));
	FQ_VERIFY(connect(authentication_, SIGNAL(requestUserPwd(QString *, QString *, bool *)),
				this, SIGNAL(requestUserPwd(QString *, QString *, bool *))));
	FQ_VERIFY(connect(authentication_, SIGNAL(authOK()), this, SLOT(authOK())));
	FQ_VERIFY(connect(authentication_, SIGNAL(authError(QString)), this, SLOT(handleError(QString))));
	FQ_VERIFY(connect(ssh_channel_, SIGNAL(channelOK()), this, SLOT(channelOK())));
	FQ_VERIFY(connect(ssh_channel_, SIGNAL(channelReadyRead(const char *, int)),
				this, SLOT(channelReadyRead(const char *, int))));
	if (ssh_version == 2) {
		FQ_VERIFY(connect(ssh_channel_, SIGNAL(channelError(QString)),
					this, SLOT(handleError(QString))));
		FQ_VERIFY(connect(ssh_channel_, SIGNAL(channelClosed()),
					this, SIGNAL(connectionClosed())));
	}
	key_exchanger_->initKex(packet_receiver_, packet_sender_);

	conn_info.proto = PROTO_SSH;
	conn_info.ssh_proto_info.proto_version = ssh_version;
}

void FQTermSSHSocket::kexOK()
{
	FQ_TRACE("sshsocket", 3) << "Key exchange completed!";
	conn_info.ssh_proto_info.c2s_cipher = packet_sender_->cipher->name;
	conn_info.ssh_proto_info.s2c_cipher = packet_receiver_->cipher->name;
	if (packet_sender_->mac)
		conn_info.ssh_proto_info.c2s_mac = packet_sender_->mac->name;
	if (packet_receiver_->mac)
		conn_info.ssh_proto_info.s2c_mac = packet_receiver_->mac->name;

	if (ssh_version_ == 2) {
		int nhosts;
		struct ssh_host *hosts;
		const char *hosts_file;
#ifdef WIN32
		hosts_file = (getPath(USER_CONFIG) + "known_hosts").toLatin1().constData();
#else
		hosts_file = ssh_hosts_filename();
#endif
		hosts = parse_hosts_file(hosts_file, &nhosts);
		int idx = find_ssh_host(hosts, nhosts, conn_info.hostname, conn_info.port);
		FQTermSSH2Kex *kex = dynamic_cast<FQTermSSH2Kex *> (key_exchanger_);
		if (idx >=0 && key_matches(&hosts[idx], kex->K_S(), kex->K_S_len()))
			conn_info.ssh_proto_info.key_matches = 1;
		else
			conn_info.ssh_proto_info.key_matches = 0;
	}

	key_exchanger_->hostKeyHash(conn_info.ssh_proto_info.hash);
	authentication_->initAuth(packet_receiver_, packet_sender_);
}

void FQTermSSHSocket::authOK() {
  FQ_TRACE("sshsocket", 3) << "Auth completed!";
  ssh_channel_->initChannel(packet_receiver_, packet_sender_, col_, row_, termtype_);
}

void FQTermSSHSocket::channelOK() {
  FQ_TRACE("sshsocket", 3) << "Channel established!";
  is_channel_ok_ = true;
  //auth_ok_emitted_ = false;
}

void FQTermSSHSocket::channelReadyRead(const char *data, int len)
{
	buffer_append(&input_buffer, (const uint8_t*)data, len);
	emit readyRead();
}

unsigned long FQTermSSHSocket::socketWriteBlock(const char *data, unsigned long len) {
  QByteArray to_write(data, len);
  return private_socket_->writeBlock(to_write);
}

void FQTermSSHSocket::socketReadyRead() {
  if (!auth_ok_emitted_ && is_channel_ok_) {
    auth_ok_emitted_ = true;
    emit sshAuthOK();
  }
  unsigned long size;
  
  switch (ssh_socket_state_) {
    case BeforeSession:
      {
        QByteArray str;
        int version;
        size = private_socket_->bytesAvailable();
        str = private_socket_->readBlock(size);

        server_name_ = QString(str).trimmed(); // remove the newline
        version = chooseVersion(str.data());

        FQ_TRACE("sshsocket", 3) << "SSH server: " << server_name_;
        FQ_TRACE("sshsocket", 3) << "SSH version chosen: " << version;

        if (version == 1) {
		bool isok;
		emit warnInsecure(tr("You are using the insecure SSH1 connection, continue?"), &isok);
		if (!isok) {
			handleError(tr("Closed by user because of the insecure SSH1 connection."));
			return;
		}
          init(1);
          ssh_version_ = 1;
          socketWriteBlock(V1STR, strlen(V1STR));
        } else if (version == 2) {
          init(2);
          ssh_version_ = 2;
          socketWriteBlock(V2STR, strlen(V2STR));
        } else {
          handleError(tr("Unknown SSH version. "
                      "Check if you set the right server and port."));
          return ;
        }
        ssh_socket_state_ = SockSession;
        private_socket_->flush();
        break;
      }
    case SockSession:
      parsePacket();
  }
}

void FQTermSSHSocket::parsePacket() {
  unsigned long size;
  QByteArray data;
  size = private_socket_->bytesAvailable();
  data = private_socket_->readBlock(size);

  buffer_append(&socket_buffer, (const uint8_t*)data.data(), size);
  packet_receiver_->parseData(&socket_buffer);
}

int FQTermSSHSocket::chooseVersion(const QString &ver) {
  QString version = ver.mid(ver.indexOf("-") + 1);
  version = version.left(version.indexOf("-"));

  if (version == "2.0" || version == "1.99")
    return 2;
  else if (version == "1.3" || version == "1.5")
    return 1;
  else
    return -1;
}

void FQTermSSHSocket::connectToHost(const QString &host_name, quint16 port)
{
	ssh_socket_state_ = BeforeSession;
	private_socket_->connectToHost(host_name, port);
	strncpy(conn_info.hostname, host_name.toLatin1().constData(),
			sizeof(conn_info.hostname));
	conn_info.port = port;
}

void FQTermSSHSocket::writeData()
{
	socketWriteBlock((const char*)buffer_data(&packet_sender_->data_to_send),
			buffer_len(&packet_sender_->data_to_send));
	private_socket_->flush();
}

void FQTermSSHSocket::handlePacket(int type) {
  if (ssh_version_ == 1) {
    switch (type) {
      case SSH1_MSG_DISCONNECT:
        char *reason;
        reason = (char*)packet_receiver_->getString();
        FQ_TRACE("sshsocket", 1) << "Disconnect because: " << reason;
        delete [] reason;
        break;
      default:
        return ;
    }
  } else {
    // ssh_version_ == 2;
    switch (type) {
      case SSH2_MSG_DISCONNECT:
        char *reason;
        packet_receiver_->consume(4);
        reason = (char*)packet_receiver_->getString();
        FQ_TRACE("sshsocket", 1) << "Disconnect because: " << reason;
        delete[] reason;
        break;
      default:
        return ;
    }
  }
}

unsigned long FQTermSSHSocket::bytesAvailable()
{
	return buffer_len(&input_buffer);
}

QByteArray FQTermSSHSocket::readBlock(unsigned long size)
{
	QByteArray data(size, 0);
	buffer_get(&input_buffer, (uint8_t*)data.data(), size);
	return data;
}

long FQTermSSHSocket::writeBlock(const QByteArray &data)
{
	if (!is_channel_ok_) return 0;

	unsigned long size = data.size();
	buffer_append(&output_buffer, (const uint8_t*)data.data(), size);
	return size;
}

void FQTermSSHSocket::flush()
{
	if (!is_channel_ok_) return;

	size_t size = buffer_len(&output_buffer);
	ssh_channel_->sendData((const char *)buffer_data(&output_buffer), size);
	buffer_consume(&output_buffer, size);
}

void FQTermSSHSocket::close() {
  private_socket_->close();
}

void FQTermSSHSocket::handleError(QString reason) {
  close();

  emit errorMessage(reason);
  emit connectionClosed();
}

void FQTermSSHSocket::setProxy(int nProxyType, bool bAuth,
                               const QString &strProxyHost,
                               quint16 uProxyPort,
                               const QString &strProxyUsr,
                               const QString &strProxyPwd) {
  private_socket_->setProxy(nProxyType, bAuth, strProxyHost,
                            uProxyPort, strProxyUsr, strProxyPwd);
}

bool FQTermSSHSocket::setTermSize(int col, int row) {
  if (ssh_channel_ && is_channel_ok_)
    ssh_channel_->changeTermSize(col, row);
  return true;
}
}  // namespace FQTerm

#include "fqterm_ssh_socket.moc"
