/*
 * ssh_known_hosts.h: parser for ~/.ssh/known_hosts
 * Copyright (C) 2018  Iru Cai <mytbk920423@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef SSH_KNOWN_HOSTS_H
#define SSH_KNOWN_HOSTS_H

#ifdef __cplusplus
extern "C" {
#endif /* } */

struct ssh_host {
	char hostname[256], keytype[32], pubkey[1024];
};

struct ssh_host *parse_hosts_file(const char *fn, int *list_sz);
int find_ssh_host(struct ssh_host *h, int n, const char *hostname, int port);
int key_matches(struct ssh_host *h, const unsigned char *K_S, int K_S_len);
const char *ssh_hosts_filename(void);
struct ssh_host *parse_unix_hosts_file(int *nhosts);
void append_hostkey(const char *, const char *, const unsigned char *, int);

#ifdef __cplusplus
}
#endif

#endif
