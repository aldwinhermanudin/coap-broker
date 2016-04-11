#include "cmdline.h"

coap_list_t *new_option_node(unsigned short key, unsigned int length, unsigned char *data) {
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

void cmdline_uri(char *arg, str *proxy, coap_list_t *optlist, coap_uri_t *uri) {
  unsigned char portbuf[2];
#define BUFSIZE 40
  unsigned char _buf[BUFSIZE];
  unsigned char *buf = _buf;
  size_t buflen;
  int res;

  if (proxy->length) {   /* create Proxy-Uri from argument */
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
      coap_split_uri((unsigned char *)arg, strlen(arg), uri ); //mencurigakan, bisa ga uri aja

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
