#ifndef CMDLINE_H_
#define CMDLINE_H_

#include "coap_config.h"
#include "coap.h"
#include "coap_list.h"

typedef struct {
  unsigned char code;
  char *media_type;
} content_type_t;

void cmdline_content_type(char *arg, unsigned short key, coap_list_t *optlist);
coap_list_t *new_option_node(unsigned short key, unsigned int length, unsigned char *data);
void cmdline_uri(char *arg, str *proxy, coap_list_t *optlist, coap_uri_t *uri);

#endif
