#ifndef COAP_ATTR_H
#define COAP_ATTR_H

#include "coap.h"
#include <iostream>
#include "UString.hpp"

namespace coap{    
    
    class Attribute{

    private:
        UString name_;
        UString value_;
        int flags_;

    public:
        Attribute();
        Attribute(coap_attr_t* attribute);
        Attribute(UString name, UString value, int flags);
        UString get_name();
        UString get_value();
        int get_flags();
        void set_name(UString name);
        void set_value(UString value);
        void set_flags(int flags);
        

    };
}

#endif