/*
 * Linux IEEE 802.15.4 ping tool
 *
 * Copyright (C) 2015 Stefan Schmidt <stefan@datenfreihafen.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <stdbool.h>

#include <netlink/netlink.h>

#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/attr.h>

#include "nl802154.h"



#define MIN_PAYLOAD_LEN 5
#define MAX_PAYLOAD_LEN 105 //116 with short address
#define IEEE802154_ADDR_LEN 8
/* Set the dispatch header to not 6lowpan for compat */
#define NOT_A_6LOWPAN_FRAME 0x00

#define DEBUG 0

enum {
	IEEE802154_ADDR_NONE = 0x0,
	IEEE802154_ADDR_SHORT = 0x2,
	IEEE802154_ADDR_LONG = 0x3,
};

struct ieee802154_addr_sa {
	int addr_type;
	uint16_t pan_id;
	union {
		uint8_t hwaddr[IEEE802154_ADDR_LEN];
		uint16_t short_addr;
	};
};

struct sockaddr_ieee802154 {
	sa_family_t family;
	struct ieee802154_addr_sa addr;
};

#ifdef _GNU_SOURCE
static const struct option perf_long_opts[] = {
	{ "daemon", no_argument, NULL, 'd' },
	{ "address", required_argument, NULL, 'a' },
	{ "extended", no_argument, NULL, 'e' },
	{ "count", required_argument, NULL, 'c' },
	{ "size", required_argument, NULL, 's' },
	{ "interface", required_argument, NULL, 'i' },
	{ "version", no_argument, NULL, 'v' },
	{ "help", no_argument, NULL, 'h' },
	{ NULL, 0, NULL, 0 },
};
#endif

struct config {
	char packet_len;
	unsigned short packets;
	bool extended;
	bool server;
	char *interface;
	struct nl_sock *nl_sock;
	int nl802154_id;
	struct sockaddr_ieee802154 src;
	struct sockaddr_ieee802154 dst;
};

extern char *optarg;

static int nl802154_init(struct config *conf)
{
	int err;

	conf->nl_sock = nl_socket_alloc();
	if (!conf->nl_sock) {
		fprintf(stderr, "Failed to allocate netlink socket.\n");
		return -ENOMEM;
	}

	nl_socket_set_buffer_size(conf->nl_sock, 8192, 8192);

	if (genl_connect(conf->nl_sock)) {
		fprintf(stderr, "Failed to connect to generic netlink.\n");
		err = -ENOLINK;
		goto out_handle_destroy;
	}

	conf->nl802154_id = genl_ctrl_resolve(conf->nl_sock, "nl802154");
	if (conf->nl802154_id < 0) {
		fprintf(stderr, "nl802154 not found.\n");
		err = -ENOENT;
		goto out_handle_destroy;
	}

	return 0;

out_handle_destroy:
	nl_socket_free(conf->nl_sock);
	return err;
}

static void nl802154_cleanup(struct config *conf)
{
	nl_close(conf->nl_sock);
	nl_socket_free(conf->nl_sock);
}

static int nl_msg_cb(struct nl_msg* msg, void* arg)
{
	struct config *conf = arg;
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	struct nlattr *attrs[NL802154_ATTR_MAX+1];
	uint64_t temp;

	struct genlmsghdr *gnlh = (struct genlmsghdr*) nlmsg_data(nlh);

	nla_parse(attrs, NL802154_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	if (!attrs[NL802154_ATTR_SHORT_ADDR] || !attrs[NL802154_ATTR_PAN_ID]
	    || !attrs[NL802154_ATTR_EXTENDED_ADDR])
		return NL_SKIP;

	conf->src.family = AF_IEEE802154;
	conf->src.addr.pan_id = conf->dst.addr.pan_id = nla_get_u16(attrs[NL802154_ATTR_PAN_ID]);

	if (!conf->extended) {
		conf->src.addr.addr_type = IEEE802154_ADDR_SHORT;
		conf->src.addr.short_addr = nla_get_u16(attrs[NL802154_ATTR_SHORT_ADDR]);
	} else {
		conf->src.addr.addr_type = IEEE802154_ADDR_LONG;
		temp = htobe64(nla_get_u64(attrs[NL802154_ATTR_EXTENDED_ADDR]));
		memcpy(&conf->src.addr.hwaddr, &temp, IEEE802154_ADDR_LEN);
	}

	return NL_SKIP;
}

static int get_interface_info(struct config *conf) {
	struct nl_msg *msg;

	nl802154_init(conf);

	/* Build and send message */
	nl_socket_modify_cb(conf->nl_sock, NL_CB_VALID, NL_CB_CUSTOM, nl_msg_cb, conf);
	msg = nlmsg_alloc();
	genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, conf->nl802154_id, 0, NLM_F_DUMP, NL802154_CMD_GET_INTERFACE, 0);
	nla_put_string(msg, NL802154_ATTR_IFNAME, "conf->interface");
	nl_send_sync(conf->nl_sock, msg);

	nl802154_cleanup(conf);
	return 0;
}

#if DEBUG
static void dump_packet(unsigned char *buf, int len) {
	int i;

	fprintf(stdout, "Packet payload:");
	for (i = 0; i < len; i++) {
		printf(" %x", buf[i]);
	}
	printf("\n");
}
#endif

static void init_server(int sd) {
	ssize_t len;
	unsigned char *buf;
	struct sockaddr_ieee802154 src;
	socklen_t addrlen;

	addrlen = sizeof(src);

	len = 0;
	fprintf(stdout, "Server mode. Waiting for packets...\n");
	buf = (unsigned char *)malloc(MAX_PAYLOAD_LEN);

	while (1) {
		len = recvfrom(sd, buf, MAX_PAYLOAD_LEN, 0, (struct sockaddr *)&src, &addrlen);
		if (len < 0) {
			perror("recvfrom");
			continue;
		}
#if DEBUG
		dump_packet(buf, len);
#endif
		/* Send same packet back */
		len = sendto(sd, buf, len, 0, (struct sockaddr *)&src, addrlen);
		if (len < 0) {
			perror("sendto");
			continue;
		}
	}
	free(buf);
}

static int init_network(struct config *conf) {
	int sd;
	int ret;

	sd = socket(PF_IEEE802154, SOCK_DGRAM, 0);
	if (sd < 0) {
		perror("socket");
		return 1;
	}

	/* Bind socket on this side */
	ret = bind(sd, (struct sockaddr *)&conf->src, sizeof(conf->src));
	if (ret) {
		perror("bind");
		close(sd);
		return 1;
	}

	if (conf->server)
		init_server(sd);
	//else
		//measure_roundtrip(conf, sd); //bisa buat analisa, ambil lagi aja dari wpan-ping

	shutdown(sd, SHUT_RDWR);
	close(sd);
	return 0;
}

int main(int argc, char *argv[]) {
	int c, ret;
	struct config *conf;
	char *dst_addr = NULL;

	conf = malloc(sizeof(struct config));

	/* Default to interface wpan0 if nothing else is given */
	conf->interface = "wpan0";

	/* Deafult to minimum packet size */
	conf->packet_len = MIN_PAYLOAD_LEN;

	/* Default to short addressing */
	conf->extended = false;

	conf->server = true;

	get_interface_info(conf);

	init_network(conf);
	free(conf);
	return 0;
}
