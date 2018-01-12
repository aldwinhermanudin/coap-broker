#include "Option.hpp"

namespace coap{

        Option::Option(){}
        Option::Option(unsigned short delta, UString value){
          delta_ = delta;
          value_ = value;
        }
        unsigned short Option::get_delta(){
          return delta_;
        }
        UString Option::get_value(){
          return value_;
        }
        int Option::decode_value(){
          unsigned char *temp = value_.copy_uchar_unsafe();
          int result = coap_decode_var_bytes(temp, value_.get_length());
          free(temp);
          return result;
        }
        UString Option::encode_data(unsigned int value){
          unsigned char buf[3];
          unsigned int size =  coap_encode_var_bytes(buf,value);
          UString result(buf,size);
          return result;
        }
}