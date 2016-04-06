#ifndef CMDLINE_H
#define CMDLINE_H

#include "coap_config.h"
#include "coap.h"
#include "coap_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#define min(a,b) ((a) < (b) ? (a) : (b))
/**
 * Calculates decimal value from hexadecimal ASCII character given in
 * @p c. The caller must ensure that @p c actually represents a valid
 * heaxdecimal character, e.g. with isxdigit(3).
 *
 * @hideinitializer
 */
#define hexchar_to_dec(c) ((c) & 0x40 ? ((c) & 0x0F) + 9 : ((c) & 0x0F))

#ifdef __GNUC__
#define UNUSED_PARAM __attribute__ ((unused))
#else /* not a GCC */
#define UNUSED_PARAM
#endif /* GCC */

//declarations
typedef struct {
	unsigned char code;
	char *media_type;
} content_type_t;

#define FLAGS_BLOCK 0x01
static coap_list_t *optlist = NULL;
/* Request URI.
 * TODO: associate the resources with transaction id and make it expireable */
static coap_uri_t uri;
static str proxy = { 0, NULL };
static unsigned short proxy_port = COAP_DEFAULT_PORT;

typedef unsigned char method_t;

extern str the_token;
extern int flags; 
extern coap_block_t block;
extern unsigned int obs_seconds;

//end of declarations

coap_list_t *new_option_node(unsigned short key, unsigned int length, unsigned char *data);
void cmdline_content_type(char *arg, unsigned short key);
int cmdline_blocksize(char *arg);
void cmdline_subscribe(char *arg UNUSED_PARAM);
int cmdline_proxy(char *arg);
inline void cmdline_token(char *arg);
void cmdline_option(char *arg);
int cmdline_input(char *text, str *buf);
int check_segment(const unsigned char *s, size_t length);
void decode_segment(const unsigned char *seg, size_t length, unsigned char *buf);
int cmdline_input_from_file(char *filename, str *buf);
method_t cmdline_method(char *arg);
#endif //end of CMDLINE_H
