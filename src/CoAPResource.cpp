#include "CoAPResource.hpp"
 
        CoAPResource::CoAPResource(EString name){ 
            resource = coap_resource_init(name.copy_uchar(), name.get_length(), 0);
        }
        
        void CoAPResource::register_handler(unsigned char method, coap_method_handler_t handler){
            coap_register_handler(resource, method, handler);
        }
    
        void CoAPResource::add_attribute(EString name, EString value){
            coap_add_attr(resource, name.copy_uchar(), 
                          name.get_length(),value.copy_uchar(), 
                          value.get_length(), 0);
        }
    
        coap_resource_t* CoAPResource::get_resource(){
            return resource;
        }