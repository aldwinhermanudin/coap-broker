#ifndef COAP_RD_HPP
#define COAP_RD_HPP

#include <stdio.h>
#include <string.h>
#include <coap.h>

#define RD_ROOT_STR   ((unsigned char *)"rd")
#define RD_ROOT_SIZE  2

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

typedef struct rd_t {
  UT_hash_handle hh;      /**< hash handle (for internal use only) */
  coap_key_t key;         /**< the actual key bytes for this resource */

  size_t etag_len;        /**< actual length of @c etag */
  unsigned char etag[8];  /**< ETag for current description */

  str data;               /**< points to the resource description  */
} rd_t;

rd_t * rd_new(void);

void rd_delete(rd_t *rd);

void hnd_get_resource(coap_context_t  *ctx ,
                 struct coap_resource_t *resource,
                 const coap_endpoint_t *local_interface ,
                 coap_address_t *peer ,
                 coap_pdu_t *request ,
                 str *token ,
                 coap_pdu_t *response) ;
void hnd_put_resource(coap_context_t  *ctx ,
                 struct coap_resource_t *resource ,
                 const coap_endpoint_t *local_interface ,
                 coap_address_t *peer ,
                 coap_pdu_t *request ,
                 str *token ,
                 coap_pdu_t *response) ;
                 
void hnd_delete_resource(coap_context_t  *ctx,
                    struct coap_resource_t *resource,
                    const coap_endpoint_t *local_interface ,
                    coap_address_t *peer ,
                    coap_pdu_t *request ,
                    str *token ,
                    coap_pdu_t *response) ;
                    
void hnd_get_rd(coap_context_t  *ctx ,
           struct coap_resource_t *resource ,
           const coap_endpoint_t *local_interface ,
           coap_address_t *peer ,
           coap_pdu_t *request ,
           str *token ,
           coap_pdu_t *response);
           
int parse_param(unsigned char *search,
            size_t search_len,
            unsigned char *data,
            size_t data_len,
            str *result) ;
            
void add_source_address(struct coap_resource_t *resource,
                   coap_address_t *peer);
                   
rd_t * make_rd(coap_address_t *peer , coap_pdu_t *pdu) ;

void hnd_post_rd(coap_context_t  *ctx,
            struct coap_resource_t *resource ,
            const coap_endpoint_t *local_interface ,
            coap_address_t *peer,
            coap_pdu_t *request,
            str *token ,
            coap_pdu_t *response);
            
void init_rd_resources(coap_context_t *ctx);
#endif 
