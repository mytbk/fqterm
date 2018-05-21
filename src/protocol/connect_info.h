#ifndef CONNECT_INFO_H
#define CONNECT_INFO_H

enum protocol
{
	PROTO_LOCAL,
	PROTO_TELNET,
	PROTO_SSH
};

typedef struct
{
	char hostname[256];
	int port;
	enum protocol proto;
	struct
	{
		int proto_version;
		const char *c2s_cipher;
		const char *s2c_cipher;
		const char *c2s_mac;
		const char *s2c_mac;
		unsigned char hash[32];
	} ssh_proto_info;
} conn_info_t;

#endif
