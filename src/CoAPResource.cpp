#include "CoAPResource.hpp"
 
        CoAPResource::CoAPResource(coap::UString name){ 
            resource = coap_resource_init(name.copy_uchar_unsafe(), name.get_length(), 0);
        }
        
        void CoAPResource::register_handler(unsigned char method, coap_method_handler_t handler){
            coap_register_handler(resource, method, handler);
        }
    
        void CoAPResource::add_attribute(coap::UString name, coap::UString value){
            coap_add_attr(resource, name.copy_uchar_unsafe(), 
                          name.get_length(),value.copy_uchar_unsafe(), 
                          value.get_length(), 0);
        }
    
        coap_resource_t*& CoAPResource::get_resource(){
            return resource;
        }