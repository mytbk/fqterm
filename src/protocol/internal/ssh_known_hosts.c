/*
 * ssh_known_hosts.c: parser for ~/.ssh/known_hosts
 * Copyright (C) 2018  Iru Cai <mytbk920423@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ssh_endian.h"
#include "ccan_base64.h"
#include "ssh_known_hosts.h"

static char *ssh_hosts_file = NULL;

static char *ssh_get_string(const char *buf, char *s, int size, char delim)
{
	char *end = strchr(buf, delim);
	if (end == NULL)
		return NULL;

	int l = end - buf;
	if (l >= size)
		return NULL;

	memcpy(s, buf, l);
	s[l] = 0;
	return end;
}

struct ssh_host *parse_hosts_file(const char *fn, int *list_sz)
{
	FILE *fp;
	char linebuf[1536];
	int nHosts;
	struct ssh_host *hosts;

	fp = fopen(fn, "r");
	if (fp == NULL) {
		*list_sz = 0;
		return NULL;
	}

	fseek(fp, 0, SEEK_END); // seek to end of file
	long size = ftell(fp);  // get current file pointer
	fseek(fp, 0, SEEK_SET); // seek back to beginning of file

	/* use file size/64 to estimate # of hosts */
	hosts = (struct ssh_host *)malloc(sizeof(struct ssh_host) *
					  (size / 64 + 1));
	nHosts = 0;
	while (fgets(linebuf, sizeof(linebuf), fp)) {
		char *host_end = ssh_get_string(linebuf, hosts[nHosts].hostname,
						256, ' ');
		if (host_end == NULL)
			continue;
		char *type_end = ssh_get_string(host_end + 1,
						hosts[nHosts].keytype, 32, ' ');
		if (type_end == NULL)
			continue;
		char *key_end = ssh_get_string(
			type_end + 1, hosts[nHosts].pubkey, 1024, '\n');
		if (key_end == NULL)
			continue;
		if (key_end - linebuf < 64) /* too short */
			continue;
		nHosts++;
	}
	fclose(fp);
	*list_sz = nHosts;
	return hosts;
}

int find_ssh_host(struct ssh_host *h, int n, const char *hostname, int port)
{
	int L = strlen(hostname);
	for (int i = 0; i < n; i++) {
		const char *hn = h[i].hostname;
		const char *hn_end;
		int hlen;

		if (port == 22 && hn[0] == '[')
			continue;
		if (port != 22 && hn[0] != '[')
			continue;

		if (port == 22) {
			hn_end = strchr(hn, ',');
			if (hn_end == NULL)
				hlen = strlen(hn);
			else
				hlen = hn_end - hn;
		} else {
			hn_end = strchr(hn, ':');
			if (hn_end == NULL || atoi(hn_end + 1) != port)
				continue;
			hn++;
			hlen = hn_end - hn - 1;
		}

		/* TODO: match IP address */

		if (hlen != L || memcmp(hostname, hn, L) != 0)
			continue;
		else
			return i;
	}
	return -1;
}

int key_matches(struct ssh_host *h, const unsigned char *K_S, int K_S_len)
{
	int algo_len = ntohu32(K_S);
	int h_keylen = strlen(h->pubkey);
	char buf[1536];

	if (strlen(h->keytype) == algo_len &&
	    memcmp(h->keytype, K_S + 4, algo_len) == 0) {
		if (base64_encoded_length(K_S_len) != h_keylen)
			return 0;
		base64_encode(buf, sizeof(buf), (const char *)K_S, K_S_len);
		if (memcmp(buf, h->pubkey, h_keylen) == 0)
			return 1;
		else
			return 0;
	} else {
		return 0;
	}
}

void append_hostkey(const char *fn, const char *hostname, const unsigned char *K_S, int K_S_len)
{
	FILE *fp = fopen(fn, "a");
	if (fp == NULL)
		return;

	char algo[32];
	char buf[1536];

	int algo_len = ntohu32(K_S);
	if (algo_len >= sizeof(algo))
		goto end;

	memcpy(algo, K_S + 4, algo_len);
	algo[algo_len] = 0;

	int b64len = base64_encoded_length(K_S_len);
	if (b64len >= sizeof(buf))
		goto end;

	base64_encode(buf, sizeof(buf), (const char *)K_S, K_S_len);
	buf[b64len] = 0;

	fprintf(fp, "%s %s %s\n", hostname, algo, buf);
end:
	fclose(fp);
	return;
}

const char *ssh_hosts_filename(void)
{
	if (ssh_hosts_file != NULL)
		return ssh_hosts_file;

	char *myhome = getenv("HOME");
	ssh_hosts_file = malloc(strlen(myhome) + 20);
	strcpy(ssh_hosts_file, myhome);
	strcat(ssh_hosts_file, "/.ssh/known_hosts");
	return ssh_hosts_file;
}

struct ssh_host *parse_unix_hosts_file(int *nhosts)
{
	const char *hosts_file = ssh_hosts_filename();
	return parse_hosts_file(hosts_file, nhosts);
}
