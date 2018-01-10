#include "ProtocolDataUnit.hpp"

namespace coap{

    ProtocolDataUnit::ProtocolDataUnit(unsigned char type, unsigned char code, unsigned short id, size_t size){
        pdu_ = coap_pdu_init (type, code, id, size);
    }

    ProtocolDataUnit::ProtocolDataUnit(coap_pdu_t * pdu){
        pdu_ = pdu;
    }
    void ProtocolDataUnit::clear(){
        coap_pdu_clear(pdu_, size_);
        
    }
    void ProtocolDataUnit::clear(size_t size){
        coap_pdu_clear(pdu_, size);
    }

    void ProtocolDataUnit::set_size(size_t value){
        size_ = value;
    }

    int ProtocolDataUnit::add_token(UString data){
        return coap_add_token(pdu_,data.get_length(), data.get_ustring().c_str());
    }
    
    int ProtocolDataUnit::add_option(Option data){

        return coap_add_option(pdu_, data.get_delta(), data.get_value().get_length(), data.get_value().get_ustring().c_str());
    }

    int ProtocolDataUnit::add_data(UString data){
        return coap_add_data(pdu_, data.get_length(), data.get_ustring().c_str());
    }
    
    int ProtocolDataUnit::get_data(UString &result){
        
        size_t size;
        unsigned char *data;
        int status = coap_get_data(pdu_, &size, &data);
        result = UString(data, size);
        return status;        
    }

    coap_pdu_t *ProtocolDataUnit::get_pdu(){
        return pdu_;
    }

    void ProtocolDataUnit::add_response_data(unsigned short code){
        pdu_->hdr->code = code;
        add_data(UString(response_phrase(code)));
    }

    void ProtocolDataUnit::add_response_data(UString data){
        add_data(data);
    }

    void ProtocolDataUnit::add_response_data(unsigned short code, UString data){
        pdu_->hdr->code = code;
        add_data(data);
    }

    OptionList ProtocolDataUnit::get_option(){
        OptionList result(pdu_);
        return result;
    }

    void ProtocolDataUnit::delete_pdu(ProtocolDataUnit pdu){
        coap_delete_pdu(pdu.get_pdu());
    }
    void ProtocolDataUnit::delete_pdu(coap_pdu_t * pdu){
         coap_delete_pdu(pdu);
    }
    UString ProtocolDataUnit::response_phrase(unsigned char code){
        UString temp (coap_response_phrase(code));
        return temp;
    }
}