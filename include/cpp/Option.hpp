#ifndef OPTION_HPP
#define OPTION_HPP

#include <coap.h>
#include <vector>
#include "UString.hpp"


namespace coap{

    class Option{

    private:
        unsigned short delta_;
        UString value_;

    public:
        Option();
        Option(unsigned short delta, UString value);
        unsigned short get_delta();
        UString get_value();
        int decode_value();
    };
}

#endif