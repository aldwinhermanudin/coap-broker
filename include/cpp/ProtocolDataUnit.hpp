#ifndef PDU_HPP
#define PDU_HPP

#include <coap.h>
#include "UString.hpp"
#include "Option.hpp"
#include "OptionList.hpp"

namespace coap{

    class ProtocolDataUnit{

        private:
            coap_pdu_t * pdu_;
            size_t size_;


        public:
        ProtocolDataUnit(unsigned char type, unsigned char code, unsigned short id, size_t size);
        ProtocolDataUnit(coap_pdu_t * pdu);
        void clear();
        void clear(size_t size);
        void set_size(size_t value);
        int add_token(UString data);
        int add_option(Option data);
        int add_data(UString data);
        int get_data(UString &result);
        coap_pdu_t *get_pdu();
        void add_response_data(unsigned short code);
        void add_response_data(UString data);
        void add_response_data(unsigned short code, UString data);

        OptionList get_option();

        static void delete_pdu(ProtocolDataUnit pdu);
        static void delete_pdu(coap_pdu_t * pdu);
        static UString response_phrase(unsigned char code);

    };


}


#endif