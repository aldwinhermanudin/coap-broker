#include "client_misc.h"

inline void set_timeout(coap_tick_t *timer, const unsigned int seconds) {
	coap_ticks(timer);
	*timer += seconds * COAP_TICKS_PER_SECOND;
}

//int append_to_output(const unsigned char *data, size_t len, FILE *file, str *output_file) {
	//size_t written;
	
	//if (!file) {
		//if (!output_file->s || (output_file->length && output_file->s[0] == '-'))
			//file = stdout;
		//else {
			//if (!(file = fopen((char *)output_file->s, "w"))) {
				//perror("fopen");
				//return -1;
			//}
		//}
	//}

	//do {
		//written = fwrite(data, 1, len, file);
		//len -= written;
		//data += written;
	//} while ( written && len );
	//fflush(file);

	//return 0;
//}

//void close_output(FILE *file, str *output_file) {
	//if (file) {

    ///* add a newline before closing in case were writing to stdout */
		//if (!output_file->s || (output_file->length && output_file->s[0] == '-'))
			//fwrite("\n", 1, 1, file);

		//fflush(file);
		//fclose(file);
	//}
//}

int order_opts(void *a, void *b) {
	coap_option *o1, *o2;

	if (!a || !b)
		return a < b ? -1 : 1;

	o1 = (coap_option *)(((coap_list_t *)a)->data);
	o2 = (coap_option *)(((coap_list_t *)b)->data);

	return (COAP_OPTION_KEY(*o1) < COAP_OPTION_KEY(*o2)) ? -1 : (COAP_OPTION_KEY(*o1) != COAP_OPTION_KEY(*o2));
}

//coap_pdu_t *coap_new_request(coap_context_t *ctx,
                 //method_t m,
                 //coap_list_t **options,
                 //unsigned char *data,
                 //size_t length,
                 //unsigned char msgtype) {
	//coap_pdu_t *pdu;
	//coap_list_t *opt;

	//if ( ! ( pdu = coap_new_pdu() ) )
		//return NULL;

	//pdu->hdr->type = msgtype;
	//pdu->hdr->id = coap_new_message_id(ctx);
	//pdu->hdr->code = m;

	//pdu->hdr->token_length = the_token.length;
	//if ( !coap_add_token(pdu, the_token.length, the_token.s)) {
		//debug("cannot add token to request\n");
	//}

	//coap_show_pdu(pdu);

	//if (options) {
		///* sort options for delta encoding */
		//LL_SORT((*options), order_opts);

		//LL_FOREACH((*options), opt) {
			//coap_option *o = (coap_option *)(opt->data);
			//coap_add_option(pdu,
                      //COAP_OPTION_KEY(*o),
                      //COAP_OPTION_LENGTH(*o),
                      //COAP_OPTION_DATA(*o));
		//}
	//}

	//if (length) {
		//if ((flags & FLAGS_BLOCK) == 0)
			//coap_add_data(pdu, length, data);
    //else
		//coap_add_block(pdu, length, data, block.num, block.szx);
	//}

	//return pdu;
//}

//coap_tid_t clear_obs(coap_context_t *ctx,
          //const coap_endpoint_t *local_interface,
          //const coap_address_t *remote,
          //unsigned char msgtype) {
	
	//coap_pdu_t *pdu;
	//coap_list_t *option;
	//coap_tid_t tid = COAP_INVALID_TID;
	//unsigned char buf[2];

	///* create bare PDU w/o any option  */
	//pdu = coap_pdu_init(msgtype,
                      //COAP_REQUEST_GET,
                      //coap_new_message_id(ctx),
                      //COAP_MAX_PDU_SIZE);

	//if (!pdu) {
		//return tid;
	//}

	//if (!coap_add_token(pdu, the_token.length, the_token.s)) {
		//coap_log(LOG_CRIT, "cannot add token");
		//goto error;
	//}

	//for (option = optlist; option; option = option->next ) {
		//coap_option *o = (coap_option *)(option->data);
		//if (COAP_OPTION_KEY(*o) == COAP_OPTION_URI_HOST) {
			//if (!coap_add_option(pdu,
				//COAP_OPTION_KEY(*o),
				//COAP_OPTION_LENGTH(*o),
				//COAP_OPTION_DATA(*o))) {
				//goto error;
			//}
			//break;
		//}
	//}

	//if (!coap_add_option(pdu,
      //COAP_OPTION_OBSERVE,
      //coap_encode_var_bytes(buf, COAP_OBSERVE_CANCEL),
      //buf)) {
		//coap_log(LOG_CRIT, "cannot add option Observe: %u", COAP_OBSERVE_CANCEL);
		//goto error;
	//}

	//for (option = optlist; option; option = option->next ) {
		//coap_option *o = (coap_option *)(option->data);
		//switch (COAP_OPTION_KEY(*o)) {
			//case COAP_OPTION_URI_PORT :
			//case COAP_OPTION_URI_PATH :
			//case COAP_OPTION_URI_QUERY :
			//if (!coap_add_option (pdu,
                            //COAP_OPTION_KEY(*o),
                            //COAP_OPTION_LENGTH(*o),
                            //COAP_OPTION_DATA(*o))) {
			//goto error;
			//}
			//break;
		//default:
		//;
		//}
	//}

	//coap_show_pdu(pdu);

	//if (pdu->hdr->type == COAP_MESSAGE_CON)
		//tid = coap_send_confirmed(ctx, local_interface, remote, pdu);
	//else
		//tid = coap_send(ctx, local_interface, remote, pdu);

	//if (tid == COAP_INVALID_TID) {
		//debug("clear_obs: error sending new request");
		//coap_delete_pdu(pdu);
	//} else if (pdu->hdr->type != COAP_MESSAGE_CON)
		//coap_delete_pdu(pdu);

	//return tid;
	//error:

	//coap_delete_pdu(pdu);
	//return tid;
//}

//int
//resolve_address(const str *server, struct sockaddr *dst) {

  //struct addrinfo *res, *ainfo;
  //struct addrinfo hints;
  //static char addrstr[256];
  //int error, len=-1;

  //memset(addrstr, 0, sizeof(addrstr));
  //if (server->length)
    //memcpy(addrstr, server->s, server->length);
  //else
    //memcpy(addrstr, "localhost", 9);

  //memset ((char *)&hints, 0, sizeof(hints));
  //hints.ai_socktype = SOCK_DGRAM;
  //hints.ai_family = AF_UNSPEC;

  //error = getaddrinfo(addrstr, NULL, &hints, &res);

  //if (error != 0) {
    //fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
    //return error;
  //}

  //for (ainfo = res; ainfo != NULL; ainfo = ainfo->ai_next) {
    //switch (ainfo->ai_family) {
    //case AF_INET6:
    //case AF_INET:
      //len = ainfo->ai_addrlen;
      //memcpy(dst, ainfo->ai_addr, len);
      //goto finish;
    //default:
      //;
    //}
  //}

 //finish:
  //freeaddrinfo(res);
  //return len;
//}

//inline int check_token(coap_pdu_t *received) {
	//return received->hdr->token_length == the_token.length &&
		//memcmp(received->hdr->token, the_token.s, the_token.length) == 0;
//}

void usage( const char *program, const char *version, unsigned int wait_seconds) {
  const char *p;

  p = strrchr( program, '/' );
  if ( p )
    program = ++p;

  fprintf( stderr, "%s v%s -- a small CoAP implementation\n"
     "(c) 2010-2015 Olaf Bergmann <bergmann@tzi.org>\n\n"
     "usage: %s [-A type...] [-t type] [-b [num,]size] [-B seconds] [-e text]\n"
     "\t\t[-m method] [-N] [-o file] [-P addr[:port]] [-p port]\n"
     "\t\t[-s duration] [-O num,text] [-T string] [-v num] [-a addr] URI\n\n"
     "\tURI can be an absolute or relative coap URI,\n"
     "\t-a addr\tthe local interface address to use\n"
     "\t-A type...\taccepted media types as comma-separated list of\n"
     "\t\t\tsymbolic or numeric values\n"
     "\t-t type\t\tcontent format for given resource for PUT/POST\n"
     "\t-b [num,]size\tblock size to be used in GET/PUT/POST requests\n"
     "\t       \t\t(value must be a multiple of 16 not larger than 1024)\n"
     "\t       \t\tIf num is present, the request chain will start at\n"
     "\t       \t\tblock num\n"
     "\t-B seconds\tbreak operation after waiting given seconds\n"
     "\t\t\t(default is %d)\n"
     "\t-e text\t\tinclude text as payload (use percent-encoding for\n"
     "\t\t\tnon-ASCII characters)\n"
     "\t-f file\t\tfile to send with PUT/POST (use '-' for STDIN)\n"
     "\t-m method\trequest method (get|put|post|delete), default is 'get'\n"
     "\t-N\t\tsend NON-confirmable message\n"
     "\t-o file\t\toutput received data to this file (use '-' for STDOUT)\n"
     "\t-p port\t\tlisten on specified port\n"
     "\t-s duration\tsubscribe for given duration [s]\n"
     "\t-v num\t\tverbosity level (default: 3)\n"
     "\t-O num,text\tadd option num with contents text to request\n"
     "\t-P addr[:port]\tuse proxy (automatically adds Proxy-Uri option to\n"
     "\t\t\trequest)\n"
     "\t-T token\tinclude specified token\n"
     "\n"
     "examples:\n"
     "\tcoap-client -m get coap://[::1]/\n"
     "\tcoap-client -m get coap://[::1]/.well-known/core\n"
     "\tcoap-client -m get -T cafe coap://[::1]/time\n"
     "\techo 1000 | coap-client -m put -T cafe coap://[::1]/time -f -\n"
     ,program, version, program, wait_seconds);
}

/* Called after processing the options from the commandline to set
 * Block1 or Block2 depending on method. */
void set_blocksize(method_t method, str *payload) {
	static unsigned char buf[4];	/* hack: temporarily take encoded bytes */
	unsigned short opt;
	unsigned int opt_length;

	if (method != COAP_REQUEST_DELETE) {
		opt = method == COAP_REQUEST_GET ? COAP_OPTION_BLOCK2 : COAP_OPTION_BLOCK1;

		block.m = (opt == COAP_OPTION_BLOCK1) && ((1u << (block.szx + 4)) < payload->length);

		opt_length = coap_encode_var_bytes(buf,
          (block.num << 4 | block.m << 3 | block.szx));

		coap_insert(&optlist, new_option_node(opt, opt_length, buf));
	}
}

//coap_context_t *get_context(const char *node, const char *port) {
  //coap_context_t *ctx = NULL;
  //int s;
  //struct addrinfo hints;
  //struct addrinfo *result, *rp;

  //memset(&hints, 0, sizeof(struct addrinfo));
  //hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
  //hints.ai_socktype = SOCK_DGRAM; /* Coap uses UDP */
  //hints.ai_flags = AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV | AI_ALL;

  //s = getaddrinfo(node, port, &hints, &result);
  //if ( s != 0 ) {
    //fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    //return NULL;
  //}

  ///* iterate through results until success */
  //for (rp = result; rp != NULL; rp = rp->ai_next) {
    //coap_address_t addr;

    //if (rp->ai_addrlen <= sizeof(addr.addr)) {
      //coap_address_init(&addr);
      //addr.size = rp->ai_addrlen;
      //memcpy(&addr.addr, rp->ai_addr, rp->ai_addrlen);

      //ctx = coap_new_context(&addr);
      //if (ctx) {
        ///* TODO: output address:port for successful binding */
        //goto finish;
      //}
    //}
  //}

  //fprintf(stderr, "no context available for interface '%s'\n", node);

  //finish:
  //freeaddrinfo(result);
  //return ctx;
//}
