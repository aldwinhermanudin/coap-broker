#ifndef LIBCOAP_MOD_HPP
#define LIBCOAP_MOD_HPP

#include <coap.h>
#include <iostream>
#include <vector>
#include "UString.hpp"

void coapDeleteAttr(coap_attr_t *attr);
void coapFreeResource(coap_resource_t *resource);
namespace coap{
    class Option{

    private:
        std::vector<std::pair<unsigned short,UString>> data_;

    public:
        Option();
        Option(coap_pdu_t *request);
        std::vector<std::pair<unsigned short,UString>> get_data();
        bool type_exist(unsigned short type);
        coap::UString get_type_value(unsigned short type);
        int decode_value(unsigned short type);
    };
}
#endif
