#include "coap_config.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "coap.h"
#include "coap_list.h"

#ifdef __GNUC__
#define UNUSED_PARAM __attribute__ ((unused))
#else /* not a GCC */
#define UNUSED_PARAM
#endif /* GCC */

#define BUFSIZE 40
#define FLAGS_BLOCK 0x01

//semua yang dibutuhin di message_handler
unsigned int obs_seconds = 30;          /* default observe time */
coap_tick_t obs_wait = 0;               /* timeout for current subscription */
unsigned int wait_seconds = 90;         /* default timeout in seconds */
coap_tick_t max_wait;                   /* global timeout (changed by set_timeout()) */
coap_block_t block = { .num = 0, .m = 0, .szx = 6 };
static str payload = { 0, NULL };       /* optional payload to send */
/* reading is done when this flag is set */
static int ready = 0;
//end of semua yang ada di message handler

//buat fungsi check_token
static unsigned char _token_data[8];
str the_token = { 0, _token_data };
//end of buat check_token

int flags = 0;
static str output_file = { 0, NULL };   /* output file name */
static FILE *file = NULL;               /* output file stream */
unsigned char msgtype = COAP_MESSAGE_CON; /* usually, requests are sent confirmable */
static coap_list_t *optlist = NULL;
unsigned char method;
unsigned short port = COAP_DEFAULT_PORT;
static coap_uri_t uri;
/* typedef struct {
      str host;         
      unsigned short port;      
      str path;         
      str query;            
    } coap_uri_t;
 */

static int
order_opts(void *a, void *b) {
  coap_option *o1, *o2;

  if (!a || !b)
    return a < b ? -1 : 1;

  o1 = (coap_option *)(((coap_list_t *)a)->data);
  o2 = (coap_option *)(((coap_list_t *)b)->data);

  return (COAP_OPTION_KEY(*o1) < COAP_OPTION_KEY(*o2))
    ? -1
    : (COAP_OPTION_KEY(*o1) != COAP_OPTION_KEY(*o2));
}

static int
append_to_output(const unsigned char *data, size_t len) {
  size_t written;

  if (!file) {
    if (!output_file.s || (output_file.length && output_file.s[0] == '-'))
      file = stdout;
    else {
      if (!(file = fopen((char *)output_file.s, "w"))) {
        perror("fopen");
        return -1;
      }
    }
  }

  do {
    written = fwrite(data, 1, len, file);
    len -= written;
    data += written;
  } while ( written && len );
  fflush(file);

  return 0;
}

static coap_list_t *new_option_node(unsigned short key, 
	unsigned int length, unsigned char *data) {
	coap_list_t *node;

	node = coap_malloc(sizeof(coap_list_t) + sizeof(coap_option) + length);

	if (node) {
		coap_option *option;
		option = (coap_option *)(node->data);
		COAP_OPTION_KEY(*option) = key;
		COAP_OPTION_LENGTH(*option) = length;
		memcpy(COAP_OPTION_DATA(*option), data, length);
	} else {
		coap_log(LOG_DEBUG, "new_option_node: malloc\n");
	}

	return node;
}

static int resolve_address(const str *server, struct sockaddr *dst) {
	struct addrinfo *res, *ainfo;
	struct addrinfo hints;
	static char addrstr[256];
	int error, len=-1;

	memset(addrstr, 0, sizeof(addrstr));
	memcpy(addrstr, server->s, server->length);
	
	memset ((char *)&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_family = AF_INET6;

	error = getaddrinfo(addrstr, NULL, &hints, &res);

	if (error != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
		return error;
	}

	for (ainfo = res; ainfo != NULL; ainfo = ainfo->ai_next) {
		switch (ainfo->ai_family) {
			case AF_INET6:
			case AF_INET:
				len = ainfo->ai_addrlen;
				memcpy(dst, ainfo->ai_addr, len);
				goto finish;
			default:
			  ;
		}
	}

	finish:
	freeaddrinfo(res);
	return len;
}

static coap_context_t *get_context(const char *node, const char *port) {
	coap_context_t *ctx = NULL;
	int s;
	struct addrinfo hints;
	struct addrinfo *result, *rp;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET6;    /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_DGRAM; /* Coap uses UDP */
	hints.ai_flags = AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV | AI_ALL;

	s = getaddrinfo(node, port, &hints, &result);
	if ( s != 0 ) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		return NULL;
	}

	/* iterate through results until success */
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		coap_address_t addr;

		if (rp->ai_addrlen <= sizeof(addr.addr)) {
			coap_address_init(&addr);
			addr.size = rp->ai_addrlen;
			memcpy(&addr.addr, rp->ai_addr, rp->ai_addrlen);

			ctx = coap_new_context(&addr);
			if (ctx) {
			/* TODO: output address:port for successful binding */
			goto finish;
			}
		}
	}

	fprintf(stderr, "no context available for interface '%s'\n", node);

	finish:
	freeaddrinfo(result);
	return ctx;
}

static coap_pdu_t *
coap_new_request(coap_context_t *ctx,
                 unsigned char m, //method_t
                 coap_list_t **options,
                 unsigned char *data,
                 size_t length) {
  coap_pdu_t *pdu;
  coap_list_t *opt;

  if ( ! ( pdu = coap_new_pdu() ) )
    return NULL;

  pdu->hdr->type = msgtype;
  pdu->hdr->id = coap_new_message_id(ctx);
  pdu->hdr->code = m;

  pdu->hdr->token_length = the_token.length;
  if ( !coap_add_token(pdu, the_token.length, the_token.s)) {
    debug("cannot add token to request\n");
  }

  coap_show_pdu(pdu);

  if (options) {
    /* sort options for delta encoding */
    LL_SORT((*options), order_opts);

    LL_FOREACH((*options), opt) {
      coap_option *o = (coap_option *)(opt->data);
      coap_add_option(pdu,
                      COAP_OPTION_KEY(*o),
                      COAP_OPTION_LENGTH(*o),
                      COAP_OPTION_DATA(*o));
    }
  }

  if (length) {
    if ((flags & FLAGS_BLOCK) == 0)
      coap_add_data(pdu, length, data);
    else
      coap_add_block(pdu, length, data, block.num, block.szx);
  }

  return pdu;
}


static inline void
set_timeout(coap_tick_t *timer, const unsigned int seconds) {
  coap_ticks(timer);
  *timer += seconds * COAP_TICKS_PER_SECOND;
}

static inline int
check_token(coap_pdu_t *received) {
  return received->hdr->token_length == the_token.length &&
    memcmp(received->hdr->token, the_token.s, the_token.length) == 0;
}

static void
message_handler(struct coap_context_t *ctx,
                const coap_endpoint_t *local_interface,
                const coap_address_t *remote,
                coap_pdu_t *sent,
                coap_pdu_t *received,
                const coap_tid_t id UNUSED_PARAM) {

  coap_pdu_t *pdu = NULL;
  coap_opt_t *block_opt;
  coap_opt_iterator_t opt_iter;
  unsigned char buf[4];
  coap_list_t *option;
  size_t len;
  unsigned char *databuf;
  coap_tid_t tid;

#ifndef NDEBUG
  if (LOG_DEBUG <= coap_get_log_level()) {
    debug("** process incoming %d.%02d response:\n",
          (received->hdr->code >> 5), received->hdr->code & 0x1F);
    coap_show_pdu(received);
  }
#endif

  /* check if this is a response to our original request */
  if (!check_token(received)) {
    /* drop if this was just some message, or send RST in case of notification */
    if (!sent && (received->hdr->type == COAP_MESSAGE_CON ||
                  received->hdr->type == COAP_MESSAGE_NON))
      coap_send_rst(ctx, local_interface, remote, received);
    return;
  }

  if (received->hdr->type == COAP_MESSAGE_RST) {
    info("got RST\n");
    return;
  }

  /* output the received data, if any */
  if (COAP_RESPONSE_CLASS(received->hdr->code) == 2) {

    /* set obs timer if we have successfully subscribed a resource */
    if (sent && coap_check_option(received, COAP_OPTION_SUBSCRIPTION, &opt_iter)) {
      debug("observation relationship established, set timeout to %d\n", obs_seconds);
      set_timeout(&obs_wait, obs_seconds);
    }

    /* Got some data, check if block option is set. Behavior is undefined if
     * both, Block1 and Block2 are present. */
    block_opt = coap_check_option(received, COAP_OPTION_BLOCK2, &opt_iter);
    if (block_opt) { /* handle Block2 */
      unsigned short blktype = opt_iter.type;

      /* TODO: check if we are looking at the correct block number */
      if (coap_get_data(received, &len, &databuf))
        append_to_output(databuf, len);

      if(COAP_OPT_BLOCK_MORE(block_opt)) {
        /* more bit is set */
        debug("found the M bit, block size is %u, block nr. %u\n",
              COAP_OPT_BLOCK_SZX(block_opt),
              coap_opt_block_num(block_opt));

        /* create pdu with request for next block */
        pdu = coap_new_request(ctx, method, NULL, NULL, 0); /* first, create bare PDU w/o any option  */
        if ( pdu ) {
          /* add URI components from optlist */
          for (option = optlist; option; option = option->next ) {
            coap_option *o = (coap_option *)(option->data);
            switch (COAP_OPTION_KEY(*o)) {
              case COAP_OPTION_URI_HOST :
              case COAP_OPTION_URI_PORT :
              case COAP_OPTION_URI_PATH :
              case COAP_OPTION_URI_QUERY :
                coap_add_option (pdu,
                                 COAP_OPTION_KEY(*o),
                                 COAP_OPTION_LENGTH(*o),
                                 COAP_OPTION_DATA(*o));
                break;
              default:
                ;     /* skip other options */
            }
          }

          /* finally add updated block option from response, clear M bit */
          /* blocknr = (blocknr & 0xfffffff7) + 0x10; */
          debug("query block %d\n", (coap_opt_block_num(block_opt) + 1));
          coap_add_option(pdu,
                          blktype,
                          coap_encode_var_bytes(buf,
                                 ((coap_opt_block_num(block_opt) + 1) << 4) |
                                  COAP_OPT_BLOCK_SZX(block_opt)), buf);

          if (pdu->hdr->type == COAP_MESSAGE_CON)
            tid = coap_send_confirmed(ctx, local_interface, remote, pdu);
          else
            tid = coap_send(ctx, local_interface, remote, pdu);

          if (tid == COAP_INVALID_TID) {
            debug("message_handler: error sending new request");
            coap_delete_pdu(pdu);
          } else {
            set_timeout(&max_wait, wait_seconds);
            if (pdu->hdr->type != COAP_MESSAGE_CON)
              coap_delete_pdu(pdu);
          }

          return;
        }
      }
    } else { /* no Block2 option */
      block_opt = coap_check_option(received, COAP_OPTION_BLOCK1, &opt_iter);

      if (block_opt) { /* handle Block1 */
        block.szx = COAP_OPT_BLOCK_SZX(block_opt);
        block.num = coap_opt_block_num(block_opt);

        debug("found Block1, block size is %u, block nr. %u\n",
        block.szx, block.num);

        if (payload.length <= (block.num+1) * (1 << (block.szx + 4))) {
          debug("upload ready\n");
          ready = 1;
          return;
        }

        /* create pdu with request for next block */
        pdu = coap_new_request(ctx, method, NULL, NULL, 0); /* first, create bare PDU w/o any option  */
        if (pdu) {

          /* add URI components from optlist */
          for (option = optlist; option; option = option->next ) {
            coap_option *o = (coap_option *)(option->data);
            switch (COAP_OPTION_KEY(*o)) {
              case COAP_OPTION_URI_HOST :
              case COAP_OPTION_URI_PORT :
              case COAP_OPTION_URI_PATH :
              case COAP_OPTION_CONTENT_FORMAT :
              case COAP_OPTION_URI_QUERY :
                coap_add_option (pdu,
                                 COAP_OPTION_KEY(*o),
                                 COAP_OPTION_LENGTH(*o),
                                 COAP_OPTION_DATA(*o));
                break;
              default:
              ;     /* skip other options */
            }
          }

          /* finally add updated block option from response, clear M bit */
          /* blocknr = (blocknr & 0xfffffff7) + 0x10; */
          block.num++;
          block.m = ((block.num+1) * (1 << (block.szx + 4)) < payload.length);

          debug("send block %d\n", block.num);
          coap_add_option(pdu,
                          COAP_OPTION_BLOCK1,
                          coap_encode_var_bytes(buf,
                          (block.num << 4) | (block.m << 3) | block.szx), buf);

          coap_add_block(pdu,
                         payload.length,
                         payload.s,
                         block.num,
                         block.szx);
          coap_show_pdu(pdu);
          if (pdu->hdr->type == COAP_MESSAGE_CON)
            tid = coap_send_confirmed(ctx, local_interface, remote, pdu);
          else
            tid = coap_send(ctx, local_interface, remote, pdu);

          if (tid == COAP_INVALID_TID) {
            debug("message_handler: error sending new request");
            coap_delete_pdu(pdu);
          } else {
            set_timeout(&max_wait, wait_seconds);
            if (pdu->hdr->type != COAP_MESSAGE_CON)
              coap_delete_pdu(pdu);
          }

          return;
        }
      } else {
        /* There is no block option set, just read the data and we are done. */
        if (coap_get_data(received, &len, &databuf))
        append_to_output(databuf, len);
		}
		}
	} else {      /* no 2.05 */

    /* check if an error was signaled and output payload if so */
    if (COAP_RESPONSE_CLASS(received->hdr->code) >= 4) {
		fprintf(stderr, "%d.%02d",
              (received->hdr->code >> 5), received->hdr->code & 0x1F);
		if (coap_get_data(received, &len, &databuf)) {
			fprintf(stderr, " ");
        while(len--)
			fprintf(stderr, "%c", *databuf++);
			}
			fprintf(stderr, "\n");
		}

	}

	/* finally send new request, if needed */
	if (pdu && coap_send(ctx, local_interface, remote, pdu) == COAP_INVALID_TID) {
		debug("message_handler: error sending response");
	}
	coap_delete_pdu(pdu);

	/* our job is done, we can exit at any time */
	ready = coap_check_option(received, COAP_OPTION_SUBSCRIPTION, &opt_iter) == NULL;
}

/**
 *  yang mau gw cobain:
 *  command: cobain-client-coap coap://<IPv6 server's address>/
 */ 
int main(int argc, char *argv[]) {
	coap_context_t  *ctx = NULL;
	coap_address_t dst;
	static str server;
	coap_log_t log_level = LOG_WARNING;
	size_t buflen;
	char opt;
	unsigned char _buf[BUFSIZE];
	unsigned char *buf = _buf;
	int res;
	char node_str[NI_MAXHOST] = "";
	void *addrptr = NULL;
	char port_str[NI_MAXSERV] = "0";
	coap_pdu_t  *pdu;
	
	/* unsigned char method:
	 * 1: get
	 * 2: post
	 * 3: put
	 * 4: delete
	 */
	while ((opt = getopt(argc, argv, "Na:b:e:f:g:m:p:s:t:o:v:A:B:O:P:T:")) != -1) {
		switch (opt) {
			case 'm':
				method = 1;
				printf("opt arg: %s\n", optarg);
				printf("method: %u\n", method);
				break;
		}
	}
	
	printf("optind: %d\n", optind);
	printf("argc: %d\n", argc);
	printf("argv[optind]: %s\n", argv[optind]);
	
	// Sets the log level to the specified value.
	coap_set_log_level(log_level);
	
	
	
	//////////////////////bagian dari cmdline_uri
	
	// Parses a given string into URI components.
	if (coap_split_uri((unsigned char *)argv[optind], 
		strlen(argv[optind]), &uri) == 0) {
		printf("sukses split uri\n");
	}
	else {
		printf("gagal split uri\n");
		exit(1);
	}
	
	//optional: if (uri.port != COAP_DEFAULT_PORT)
	
	// coap://[ipv6]/path
	if (uri.path.length) {
		buflen = BUFSIZE;
		printf("uri.path.length exists\n");
		
		//res: The number of segments created 
		//res = coap_split_path(uri.path.s, uri.path.length, buf, &buflen);
		while (res--) {
			/* int coap_insert 	( 	coap_list_t **queue,
			 * 		coap_list_t *node, 
			 * 		int(*)(void *, void *) order) 	
			 * 
			 * Adds node to given queue, ordered by specified order function
			 */
			coap_insert(&optlist,
				new_option_node(COAP_OPTION_URI_PATH,
                COAP_OPT_LENGTH(buf),
                COAP_OPT_VALUE(buf)));

			buf += COAP_OPT_SIZE(buf);
		}
	}
	
	// coap://[ipv6]/path?=query
	if (uri.query.length) {
		printf("uri.query.length exists\n");
		buflen = BUFSIZE;
		buf = _buf;
		res = coap_split_query(uri.query.s, uri.query.length, buf, &buflen);

		while (res--) {
			coap_insert(&optlist,
				new_option_node(COAP_OPTION_URI_QUERY,
				COAP_OPT_LENGTH(buf),
                COAP_OPT_VALUE(buf)));

			buf += COAP_OPT_SIZE(buf);
		}
	}
		
	//////////////////////end of bagian dari cmdline_uri
	
	server = uri.host;
	port = uri.port;
	
	printf("server: %s\n", server.s);
	printf("port: %hu\n", port);
	
	/* resolve destination address where server should be sent */
	res = resolve_address(&server, &dst.addr.sa);
	
	if (res < 0) {
		fprintf(stderr, "failed to resolve address\n");
		exit(-1);
	}
	
	dst.size = res;
	dst.addr.sin.sin_port = htons(port);
	
	addrptr = &dst.addr.sin6.sin6_addr;

    /* create context for IPv6 */
    ctx = get_context(node_str[0] == 0 ? "::" : node_str, port_str);
    
    if (!ctx) {
		coap_log(LOG_EMERG, "cannot create context\n");
		return -1;
	}
	
	coap_register_option(ctx, COAP_OPTION_BLOCK2);
	coap_register_response_handler(ctx, message_handler);
  
	if (! (pdu = coap_new_request(ctx, method, &optlist, payload.s, payload.length))) {
		printf("bikin pdu gagal\n");
		return -1;
	}
	else 
		printf("bikin pdu berhasil\n");
    
	return 0;
}
