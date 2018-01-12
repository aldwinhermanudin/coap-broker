#ifndef ADDRESS_HPP
#define ADDRESS_HPP

#include <address.h>
#include <coap.h>
#include "UString.hpp"

namespace coap{

    class Address{

    private:
        coap_address_t *address_;

    public:
        Address();
        Address(coap_address_t *address);
        void initialize();
        bool is_any(); 
        coap_address_t *get_address();
    };
}

#endif