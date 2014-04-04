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

#ifndef FQTERMCONST_H
#define FQTERMCONST_H

namespace FQTerm {

#define SSH_CIPHER_SSH2					-3
#define SSH_CIPHER_ILLEGAL				-2
#define SSH_CIPHER_NOT_SET				-1
#define SSH_CIPHER_NONE					0
#define SSH_CIPHER_IDEA					1
#define SSH_CIPHER_DES					2
#define SSH_CIPHER_3DES					3
#define SSH_CIPHER_BROKEN_TSS				4
#define SSH_CIPHER_BROKEN_RC4				5
#define SSH_CIPHER_BLOWFISH				6
#define SSH_CIPHER_RESERVED				7
#define SSH_CIPHER_MAX					31

#define SSH1_MSG_DISCONNECT				1	
#define SSH1_SMSG_PUBLIC_KEY				2
#define SSH1_CMSG_SESSION_KEY				3
#define SSH1_CMSG_USER					4
#define SSH1_CMSG_AUTH_RSA				6
#define SSH1_SMSG_AUTH_RSA_CHALLENGE			7
#define SSH1_CMSG_AUTH_RSA_RESPONSE			8
#define SSH1_CMSG_AUTH_PASSWORD				9
#define SSH1_CMSG_REQUEST_PTY				10
#define SSH1_CMSG_WINDOW_SIZE				11
#define SSH1_CMSG_EXEC_SHELL				12
#define SSH1_CMSG_EXEC_CMD				13
#define SSH1_SMSG_SUCCESS				14
#define SSH1_SMSG_FAILURE				15
#define SSH1_CMSG_STDIN_DATA				16
#define SSH1_SMSG_STDOUT_DATA				17
#define SSH1_SMSG_STDERR_DATA				18
#define SSH1_CMSG_EOF					19
#define SSH1_SMSG_EXIT_STATUS				20
#define SSH1_MSG_CHANNEL_OPEN_CONFIRMATION		21
#define SSH1_MSG_CHANNEL_OPEN_FAILURE			22
#define SSH1_MSG_CHANNEL_DATA				23
#define SSH1_MSG_CHANNEL_CLOSE				24
#define SSH1_MSG_CHANNEL_CLOSE_CONFIRMATION		25
#define SSH1_SMSG_X11_OPEN				27
#define SSH1_CMSG_PORT_FORWARD_REQUEST			28
#define SSH1_MSG_PORT_OPEN				29
#define SSH1_CMSG_AGENT_REQUEST_FORWARDING		30
#define SSH1_SMSG_AGENT_OPEN				31
#define SSH1_MSG_IGNORE					32
#define SSH1_CMSG_EXIT_CONFIRMATION			33
#define SSH1_CMSG_X11_REQUEST_FORWARDING		34
#define SSH1_CMSG_AUTH_RHOSTS_RSA			35
#define SSH1_MSG_DEBUG					36
#define SSH1_CMSG_REQUEST_COMPRESSION			37
#define SSH1_CMSG_AUTH_TIS				39
#define SSH1_SMSG_AUTH_TIS_CHALLENGE			40
#define SSH1_CMSG_AUTH_TIS_RESPONSE			41
#define SSH1_CMSG_AUTH_CCARD				70
#define SSH1_SMSG_AUTH_CCARD_CHALLENGE			71
#define SSH1_CMSG_AUTH_CCARD_RESPONSE			72
#define SSH1_AUTH_TIS					5
#define SSH1_PROTOFLAG_SCREEN_NUMBER			1
#define SSH1_PROTOFLAGS_SUPPORTED			0


#define SSH2_MSG_DISCONNECT				1
#define SSH2_MSG_IGNORE					2
#define SSH2_MSG_UNIMPLEMENTED				3
#define SSH2_MSG_DEBUG					4
#define SSH2_MSG_SERVICE_REQUEST			5
#define SSH2_MSG_SERVICE_ACCEPT				6
#define SSH2_MSG_KEXINIT				20
#define SSH2_MSG_NEWKEYS				21
#define SSH2_MSG_KEXDH_INIT				30
#define SSH2_MSG_KEXDH_REPLY				31
#define SSH2_MSG_KEX_DH_GEX_REQUEST_OLD			30
#define SSH2_MSG_KEX_DH_GEX_GROUP			31
#define SSH2_MSG_KEX_DH_GEX_INIT			32
#define SSH2_MSG_KEX_DH_GEX_REPLY			33
#define SSH2_MSG_KEX_DH_GEX_REQUEST			34
#define SSH2_MSG_USERAUTH_REQUEST			50
#define SSH2_MSG_USERAUTH_FAILURE			51
#define SSH2_MSG_USERAUTH_SUCCESS			52
#define SSH2_MSG_USERAUTH_BANNER			53
#define SSH2_MSG_USERAUTH_PK_OK				60
#define SSH2_MSG_USERAUTH_PASSWD_CHANGEREQ		60
#define SSH2_MSG_USERAUTH_INFO_REQUEST			60
#define SSH2_MSG_USERAUTH_INFO_RESPONSE			61
#define SSH2_MSG_GLOBAL_REQUEST				80
#define SSH2_MSG_REQUEST_SUCCESS			81
#define SSH2_MSG_REQUEST_FAILURE			82
#define SSH2_MSG_CHANNEL_OPEN				90
#define SSH2_MSG_CHANNEL_OPEN_CONFIRMATION		91
#define SSH2_MSG_CHANNEL_OPEN_FAILURE			92
#define SSH2_MSG_CHANNEL_WINDOW_ADJUST			93
#define SSH2_MSG_CHANNEL_DATA				94
#define SSH2_MSG_CHANNEL_EXTENDED_DATA			95
#define SSH2_MSG_CHANNEL_EOF				96
#define SSH2_MSG_CHANNEL_CLOSE				97
#define SSH2_MSG_CHANNEL_REQUEST			98
#define SSH2_MSG_CHANNEL_SUCCESS			99
#define SSH2_MSG_CHANNEL_FAILURE			100
#define SSH2_DISCONNECT_HOST_NOT_ALLOWED_TO_CONNECT	1
#define SSH2_DISCONNECT_PROTOCOL_ERROR			2
#define SSH2_DISCONNECT_KEY_EXCHANGE_FAILED		3
#define SSH2_DISCONNECT_HOST_AUTHENTICATION_FAILED	4
#define SSH2_DISCONNECT_RESERVED			4
#define SSH2_DISCONNECT_MAC_ERROR			5
#define SSH2_DISCONNECT_COMPRESSION_ERROR		6
#define SSH2_DISCONNECT_SERVICE_NOT_AVAILABLE		7
#define SSH2_DISCONNECT_PROTOCOL_VERSION_NOT_SUPPORTED	8
#define SSH2_DISCONNECT_HOST_KEY_NOT_VERIFIABLE		9
#define SSH2_DISCONNECT_CONNECTION_LOST			10
#define SSH2_DISCONNECT_BY_APPLICATION			11
#define SSH2_DISCONNECT_TOO_MANY_CONNECTIONS		12
#define SSH2_DISCONNECT_AUTH_CANCELLED_BY_USER		13
#define SSH2_DISCONNECT_NO_MORE_AUTH_METHODS_AVAILABLE	14
#define SSH2_DISCONNECT_ILLEGAL_USER_NAME		15
#define SSH2_OPEN_ADMINISTRATIVELY_PROHIBITED		1
#define SSH2_OPEN_CONNECT_FAILED			2
#define SSH2_OPEN_UNKNOWN_CHANNEL_TYPE			3
#define SSH2_OPEN_RESOURCE_SHORTAGE			4
#define SSH2_EXTENDED_DATA_STDERR			1

}  // namespace FQTerm

#endif // FQTERMCONST_H
