#include "LibcoapMod.hpp"

void coapDeleteAttr(coap_attr_t *attr) {
     coap_delete_attr(attr);
   }           
void coapFreeResource(coap_resource_t *resource){ 
   coap_attr_t *attr, *tmp;
   coap_subscription_t *obs, *otmp;
 
   assert(resource);
 
   /* delete registered attributes */
   LL_FOREACH_SAFE(resource->link_attr, attr, tmp) coap_delete_attr(attr);
 
   if (resource->flags & COAP_RESOURCE_FLAGS_RELEASE_URI)
     coap_free(resource->uri.s);
 
   /* free all elements from resource->subscribers */
   LL_FOREACH_SAFE(resource->subscribers, obs, otmp) COAP_FREE_TYPE(subscription, obs);
 
 #ifdef WITH_LWIP
   memp_free(MEMP_COAP_RESOURCE, resource);
 #endif
 #ifndef WITH_LWIP
   coap_free_type(COAP_RESOURCE, resource);
 #endif /* WITH_CONTIKI */
}

namespace coap{ 
        HandlerData::HandlerData(coap_context_t *ctx, struct coap_resource_t *resource, 
                                 const coap_endpoint_t *local_interface, coap_address_t *peer, 
                                 coap_pdu_t *request, str *token, coap_pdu_t *response){
                  
                  ctx_ = ctx;
                  resource_ = resource;
                  local_interface_ = local_interface;
                  peer_ = peer;
                  request_ = request;
                  token_ = token;
                  response_ = response;
                  }
}
