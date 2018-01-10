#include "OptionList.hpp"

namespace coap{

        OptionList::OptionList(){}
        OptionList::OptionList(coap_pdu_t *request){
          coap_opt_t *option;
          coap_opt_iterator_t opt_iter; 
          coap_option_iterator_init(request, &opt_iter, COAP_OPT_ALL);
          debug("##### available option type #####\n");
          while ((option = coap_option_next(&opt_iter))) {
            debug("Option Type : %ld\n", opt_iter.type);
            coap::UString value(coap_opt_value(option),coap_opt_length(option)); 
            Option temp(opt_iter.type,value);
            data_.push_back(temp);
          }
        }

        bool OptionList::type_exist(unsigned short type){
          for(Option value : data_){
            if ( value.get_delta() == type){ return true;}
          }
          return false;
        }

        Option OptionList::get_option(unsigned short type){

          Option temp;
          for(Option value : data_){
            if ( value.get_delta() == type){ 
              temp = value;
              break;
            }
          }
          return temp;
        }

        std::vector<Option> OptionList::get_data(){
          return data_;
        }

}