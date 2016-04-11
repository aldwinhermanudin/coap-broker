#include "cmdline.h"

int flags = 0;
coap_block_t block = { .num = 0, .m = 0, .szx = 6 };
unsigned int obs_seconds = 30;          /* default observe time */
static unsigned char _token_data[8];
str the_token = { 0, _token_data };

coap_list_t *new_option_node(unsigned short key, 
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

void cmdline_content_type(char *arg, unsigned short key, coap_list_t *optlist) {
	static content_type_t content_types[] = {
		{  0, "plain" },
		{  0, "text/plain" },
		{ 40, "link" },
		{ 40, "link-format" },
		{ 40, "application/link-format" },
		{ 41, "xml" },
		{ 41, "application/xml" },
		{ 42, "binary" },
		{ 42, "octet-stream" },
		{ 42, "application/octet-stream" },
		{ 47, "exi" },
		{ 47, "application/exi" },
		{ 50, "json" },
		{ 50, "application/json" },
		{ 60, "cbor" },
		{ 60, "application/cbor" },
		{ 255, NULL }
	};
	
	coap_list_t *node;
	unsigned char i, value[10];
	int valcnt = 0;
	unsigned char buf[2];
	char *p, *q = arg;

	while (q && *q) {
		p = strchr(q, ',');

		if (isdigit(*q)) {
			if (p)
				*p = '\0';
      
			value[valcnt++] = atoi(q);
		} else {
			for (i=0;
				content_types[i].media_type &&
				strncmp(q, content_types[i].media_type, p ? (size_t)(p-q) : strlen(q)) != 0 ;
				++i)
				;

			if (content_types[i].media_type) {
				value[valcnt] = content_types[i].code;
				valcnt++;
			} else {
				warn("W: unknown content-format '%s'\n",arg);
			}
		}

		if (!p || key == COAP_OPTION_CONTENT_TYPE)
			break;

		q = p+1;
	}

	for (i = 0; i < valcnt; ++i) {
		node = new_option_node(key, coap_encode_var_bytes(buf, value[i]), buf);
		
		if (node) {
			LL_PREPEND(optlist, node);
		}
	}
}

void cmdline_uri(char *arg, coap_uri_t *uri, coap_list_t *optlist) {
	unsigned char portbuf[2];
	#define BUFSIZE 40
	unsigned char _buf[BUFSIZE];
	unsigned char *buf = _buf;
	size_t buflen;
	int res;

	if (proxy.length) {   /* create Proxy-Uri from argument */
		size_t len = strlen(arg);
		while (len > 270) {
		coap_insert(&optlist,
			new_option_node(COAP_OPTION_PROXY_URI,
            270,
            (unsigned char *)arg));

			len -= 270;
			arg += 270;
		}

		coap_insert(&optlist,
			new_option_node(COAP_OPTION_PROXY_URI,
			len,
            (unsigned char *)arg));

	} else {      /* split arg into Uri-* options */
		//coap_split_uri((unsigned char *)arg, strlen(arg), &uri );
		coap_split_uri((unsigned char *)arg, strlen(arg), uri );

		if (uri->port != COAP_DEFAULT_PORT) {
		coap_insert(&optlist,
			new_option_node(COAP_OPTION_URI_PORT,
            coap_encode_var_bytes(portbuf, uri->port),
            portbuf));
		}

		if (uri->path.length) {
			buflen = BUFSIZE;
			res = coap_split_path(uri->path.s, uri->path.length, buf, &buflen);

			while (res--) {
				coap_insert(&optlist,
					new_option_node(COAP_OPTION_URI_PATH,
					COAP_OPT_LENGTH(buf),
					COAP_OPT_VALUE(buf)));

				buf += COAP_OPT_SIZE(buf);
			}
		}

		if (uri->query.length) {
			buflen = BUFSIZE;
			buf = _buf;
			res = coap_split_query(uri->query.s, uri->query.length, buf, &buflen);

			while (res--) {
				coap_insert(&optlist,
						new_option_node(COAP_OPTION_URI_QUERY,
						COAP_OPT_LENGTH(buf),
						COAP_OPT_VALUE(buf)));

				buf += COAP_OPT_SIZE(buf);
			}
		}	
	}
}

int cmdline_blocksize(char *arg) {
	unsigned short size;

	again:
	size = 0;
	while(*arg && *arg != ',')
		size = size * 10 + (*arg++ - '0');

	if (*arg == ',') {
		arg++;
		block.num = size;
		goto again;
	}

	if (size)
		block.szx = (coap_fls(size >> 4) - 1) & 0x07;

	flags |= FLAGS_BLOCK;
	return 1;
}

void cmdline_subscribe(char *arg UNUSED_PARAM, coap_list_t *optlist) {
	//obs_seconds = atoi(optarg);
	obs_seconds = atoi(arg);
	coap_insert(&optlist, new_option_node(COAP_OPTION_SUBSCRIPTION, 0, NULL));
}

int cmdline_proxy(char *arg) {
	char *proxy_port_str = strrchr((const char *)arg, ':'); /* explicit port ? */
	
	if (proxy_port_str) {
		char *ipv6_delimiter = strrchr((const char *)arg, ']');
		if (!ipv6_delimiter) {
			if (proxy_port_str == strchr((const char *)arg, ':')) {
				/* host:port format - host not in ipv6 hexadecimal string format */
				*proxy_port_str++ = '\0'; /* split */
				proxy_port = atoi(proxy_port_str);
			}
		} else {
			arg = strchr((const char *)arg, '[');
			if (!arg) return 0;
			arg++;
			*ipv6_delimiter = '\0'; /* split */
			if (ipv6_delimiter + 1 == proxy_port_str++) {
			/* [ipv6 address]:port */
				proxy_port = atoi(proxy_port_str);
			}
		}
	}

	proxy.length = strlen(arg);
	if ( (proxy.s = coap_malloc(proxy.length + 1)) == NULL) {
		proxy.length = 0;
		return 0;
	}

	memcpy(proxy.s, arg, proxy.length+1);
	return 1;
}

inline void cmdline_token(char *arg) {
	strncpy((char *)the_token.s, arg, min(sizeof(_token_data), strlen(arg)));
	the_token.length = strlen(arg);
}

void cmdline_option(char *arg, coap_list_t *optlist) {
	unsigned int num = 0;

	while (*arg && *arg != ',') {
		num = num * 10 + (*arg - '0');
		++arg;
	}
	if (*arg == ',')
		++arg;

	coap_insert(&optlist,
              new_option_node(num, strlen(arg), (unsigned char *)arg));
}

/**
 * Runs through the given path (or query) segment and checks if
 * percent-encodings are correct. This function returns @c -1 on error
 * or the length of @p s when decoded.
 */
int check_segment(const unsigned char *s, size_t length) {

	size_t n = 0;

	while (length) {
		if (*s == '%') {
			if (length < 2 || !(isxdigit(s[1]) && isxdigit(s[2])))
				return -1;

			s += 2;
			length -= 2;
		}

		++s; ++n; --length;
	}

	return n;
}

/**
 * Decodes percent-encoded characters while copying the string @p seg
 * of size @p length to @p buf. The caller of this function must
 * ensure that the percent-encodings are correct (i.e. the character
 * '%' is always followed by two hex digits. and that @p buf provides
 * sufficient space to hold the result. This function is supposed to
 * be called by make_decoded_option() only.
 *
 * @param seg     The segment to decode and copy.
 * @param length  Length of @p seg.
 * @param buf     The result buffer.
 */
void decode_segment(const unsigned char *seg, size_t length, unsigned char *buf) {

	while (length--) {

		if (*seg == '%') {
			*buf = (hexchar_to_dec(seg[1]) << 4) + hexchar_to_dec(seg[2]);

			seg += 2; length -= 2;
		} else {
			*buf = *seg;
		}

		++buf; ++seg;
	}
}

int cmdline_input(char *text, str *buf) {
	int len;
	len = check_segment((unsigned char *)text, strlen(text));

	if (len < 0)
		return 0;

	buf->s = (unsigned char *)coap_malloc(len);
	if (!buf->s)
		return 0;

	buf->length = len;
	decode_segment((unsigned char *)text, strlen(text), buf->s);
	return 1;
}

int cmdline_input_from_file(char *filename, unsigned char *buf) {
	FILE *inputfile = NULL;
	ssize_t len;
	int result = 1;
	struct stat statbuf;

	if (!filename || !buf)
		return 0;

	if (filename[0] == '-' && !filename[1]) { /* read from stdin */
		buf->length = 20000;
		buf->s = (unsigned char *)coap_malloc(buf->length);
		if (!buf->s)
			return 0;

		inputfile = stdin;
	} else {
		/* read from specified input file */
		if (stat(filename, &statbuf) < 0) {
			perror("cmdline_input_from_file: stat");
			return 0;
		}

		buf->length = statbuf.st_size;
		buf->s = (unsigned char *)coap_malloc(buf->length);
		if (!buf->s)
			return 0;

		inputfile = fopen(filename, "r");
		if ( !inputfile ) {
			perror("cmdline_input_from_file: fopen");
			coap_free(buf->s);
			return 0;
		}
	}

	len = fread(buf->s, 1, buf->length, inputfile);

	if (len < 0 || ((size_t)len < buf->length)) {
		if (ferror(inputfile) != 0) {
			perror("cmdline_input_from_file: fread");
			coap_free(buf->s);
			buf->length = 0;
			buf->s = NULL;
			result = 0;
		} else {
			buf->length = len;
		}
	}

	if (inputfile != stdin)
		fclose(inputfile);

	return result;
}

method_t cmdline_method(char *arg) {
	static char *methods[] =
     { 0, "get", "post", "put", "delete", 0};
    unsigned char i;

    for (i=1; methods[i] && strcasecmp(arg,methods[i]) != 0 ; ++i);

	return i;     /* note that we do not prevent illegal methods */
}
