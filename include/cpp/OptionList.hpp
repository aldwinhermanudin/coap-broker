#ifndef OPTIONLIST_HPP
#define OPTIONLIST_HPP

#include <coap.h>
#include <vector>
#include "UString.hpp"
#include "Option.hpp"


namespace coap{

    class OptionList{

    private:
        std::vector<Option> data_;

    public:
        OptionList();
        OptionList(coap_pdu_t *request);
        std::vector<Option> get_data();
        bool type_exist(unsigned short type);
        Option get_option(unsigned short type);
    };
}

#endif