#ifndef COAP_RES_H
#define COAP_RES_H

#include "coap.h"
#include "EString.hpp"

class CoAPResource {

protected:
    coap_resource_t *resource;

public:
    CoAPResource(EString name);
    void register_handler(unsigned char method, coap_method_handler_t handler);
    void add_attribute(EString name, EString value);
    coap_resource_t* get_resource();
  
};
#endif