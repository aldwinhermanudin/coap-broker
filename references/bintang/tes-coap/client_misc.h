#ifndef CLIENT_MISC_H
#define CLIENT_MISC_H

#include "cmdline.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define HANDLE_BLOCK1(Pdu)                                        \
  ((method == COAP_REQUEST_PUT || method == COAP_REQUEST_POST) && \
   ((flags & FLAGS_BLOCK) == 0) &&                                \
   ((Pdu)->hdr->code == COAP_RESPONSE_CODE(201) ||                \
    (Pdu)->hdr->code == COAP_RESPONSE_CODE(204)))

inline void set_timeout(coap_tick_t *timer, const unsigned int seconds);
//int append_to_output(const unsigned char *data, size_t len, FILE *file, str *output_file);
//void close_output(FILE *file, str *output_file);
int order_opts(void *a, void *b);

//coap_pdu_t *coap_new_request(coap_context_t *ctx,
                 //method_t m,
                 //coap_list_t **options,
                 //unsigned char *data,
                 //size_t length,
                 //unsigned char msgtype);

//coap_tid_t clear_obs(coap_context_t *ctx,
          //const coap_endpoint_t *local_interface,
          //const coap_address_t *remote,
          //unsigned char msgtype);
          
//int resolve_address(const str *server, struct sockaddr *dst);
//inline int check_token(coap_pdu_t *received);
void usage(const char *program, const char *version, unsigned int wait_seconds);
void set_blocksize(method_t method, str *payload);
//coap_context_t *get_context(const char *node, const char *port);
#endif //end of CLIENT_MISC_H
