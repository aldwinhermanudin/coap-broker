#ifndef LIBCOAP_MOD_HPP
#define LIBCOAP_MOD_HPP

#include <coap.h>
#include <iostream>
#include <vector>
#include "UString.hpp"
#include "ProtocolDataUnit.hpp"
#include "Option.hpp"
#include "OptionList.hpp"
#include "resource.h"

 #ifdef WITH_LWIP
 /* mem.h is only needed for the string free calls for
  * COAP_ATTR_FLAGS_RELEASE_NAME / COAP_ATTR_FLAGS_RELEASE_VALUE /
  * COAP_RESOURCE_FLAGS_RELEASE_URI. not sure what those lines should actually
  * do on lwip. */
 
 #include <lwip/memp.h>
 
 #define COAP_MALLOC_TYPE(Type) \
   ((coap_##Type##_t *)memp_malloc(MEMP_COAP_##Type))
 #define COAP_FREE_TYPE(Type, Object) memp_free(MEMP_COAP_##Type, Object)
 
 #endif
 
 #ifdef WITH_POSIX
 
 #define COAP_MALLOC_TYPE(Type) \
   ((coap_##Type##_t *)coap_malloc(sizeof(coap_##Type##_t)))
 #define COAP_FREE_TYPE(Type, Object) coap_free(Object)
 
 #endif /* WITH_POSIX */
 #ifdef WITH_CONTIKI
 #include "memb.h"
 
 #define COAP_MALLOC_TYPE(Type) \
   ((coap_##Type##_t *)memb_alloc(&(Type##_storage)))
 #define COAP_FREE_TYPE(Type, Object) memb_free(&(Type##_storage), (Object))
 
 MEMB(subscription_storage, coap_subscription_t, COAP_MAX_SUBSCRIBERS);
 
 void
 coap_resources_init() {
   memb_init(&subscription_storage);
 }
 
 static inline coap_subscription_t *
 coap_malloc_subscription() {
   return memb_alloc(&subscription_storage);
 }
 
 static inline void
 coap_free_subscription(coap_subscription_t *subscription) {
   memb_free(&subscription_storage, subscription);
 }
 
 #endif /* WITH_CONTIKI */

void coapDeleteAttr(coap_attr_t *attr);
void coapFreeResource(coap_resource_t *resource);
namespace coap{
 
    class HandlerData{

        private:
            coap_context_t *ctx_;
            struct coap_resource_t *resource_;
            const coap_endpoint_t *local_interface_;
            coap_address_t *peer_;
            coap_pdu_t *request_;
            str *token_;
            coap_pdu_t *response_;

        public:
            HandlerData(coap_context_t *ctx, struct coap_resource_t *resource, 
              const coap_endpoint_t *local_interface, coap_address_t *peer, 
              coap_pdu_t *request, str *token, coap_pdu_t *response);
    };

}
#endif
