#ifndef COAP_RES_H
#define COAP_RES_H

#include "coap.h"
#include "resource.h"
#include "UString.hpp"
#include "Attribute.hpp"
#include "LinkFormat.hpp"

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


namespace coap{

   class Resource {

    protected:
        coap_resource_t *resource_ = NULL;

    public:
        Resource();
        Resource(coap::UString name, int flags);
        Resource(coap_resource_t *resource);
        Resource(coap::LinkFormat link_format, int flags);
        UString get_name();
        std::vector<Attribute> get_attribute_list();
        Attribute find_attribute(UString type);
        bool is_attribute_exist(UString type);
        void init_resource(coap::UString name, int flags);
        void register_handler(unsigned char method, coap_method_handler_t handler);
        void add_attribute(coap::Attribute attribute);
        void add_attribute(coap::Attribute attribute, int flags);
        void set_observable(bool obs_status);
        void set_dirty(bool dirty_status);
        void free_resource();
        coap_resource_t*& get_resource();
    
    };
}
#endif