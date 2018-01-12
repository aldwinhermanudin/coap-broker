#include "Address.hpp"

namespace coap{
    Address::Address(){}
    Address::Address(coap_address_t *address){
        address_ = address;
    }
    void Address::initialize(){
        coap_address_init(address_);
    }
    bool Address::is_any(){
        return coap_address_isany(address_);
    }
    coap_address_t *Address::get_address(){
        return address_;
    }


}