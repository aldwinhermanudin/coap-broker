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
}