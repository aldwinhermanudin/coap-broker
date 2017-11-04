#ifndef COAP_RD_HPP
#define COAP_RD_HPP

#include <coap.h>  
#include "EString.hpp"
#include "CoAPResource.hpp"
#include "CoAPServer.hpp"

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

struct rd_t {
  UT_hash_handle hh;      /**< hash handle (for internal use only) */
  coap_key_t key;         /**< the actual key bytes for this resource */

  size_t etag_len;        /**< actual length of @c etag */
  unsigned char etag[8];  /**< ETag for current description */

  str data;               /**< points to the resource description  */
};

class CoAPRD : public CoAPResource{

private:

  static EString resource_name;
  static unsigned char *RD_ROOT_STR;
  static size_t RD_ROOT_SIZE;
  static rd_t * rd_new(void);
  static void rd_delete(rd_t *rd);                               
  static rd_t * make_rd(coap_address_t *peer , coap_pdu_t *pdu);
  static rd_t *resources;
  static void add_source_address(struct coap_resource_t *resource,
                                          coap_address_t *peer);
  static int parse_param(unsigned char *search,
            size_t search_len,
            unsigned char *data,
            size_t data_len,
            str *result);

  static void hnd_get_resource(coap_context_t  *ctx ,
                          struct coap_resource_t *resource,
                          const coap_endpoint_t *local_interface ,
                          coap_address_t *peer ,
                          coap_pdu_t *request ,
                          str *token ,
                          coap_pdu_t *response) ;
  static void hnd_put_resource(coap_context_t  *ctx ,
                          struct coap_resource_t *resource ,
                          const coap_endpoint_t *local_interface ,
                          coap_address_t *peer ,
                          coap_pdu_t *request ,
                          str *token ,
                          coap_pdu_t *response) ;                 
  static void hnd_delete_resource(coap_context_t  *ctx,  
                          struct coap_resource_t *resource,
                          const coap_endpoint_t *local_interface ,
                          coap_address_t *peer ,
                          coap_pdu_t *request ,
                          str *token ,
                          coap_pdu_t *response) ;
  static void hnd_get_rd(coap_context_t  *ctx ,
                          struct coap_resource_t *resource ,
                          const coap_endpoint_t *local_interface ,
                          coap_address_t *peer ,
                          coap_pdu_t *request ,
                          str *token ,
                          coap_pdu_t *response);

  static void hnd_post_rd(coap_context_t  *ctx,
            struct coap_resource_t *resource ,
            const coap_endpoint_t *local_interface ,
            coap_address_t *peer,
            coap_pdu_t *request,
            str *token ,
            coap_pdu_t *response);


public:
   CoAPRD( EString name);

};
#endif 
