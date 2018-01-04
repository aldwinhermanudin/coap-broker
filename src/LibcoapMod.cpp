#include "LibcoapMod.hpp"

void coapDeleteAttr(coap_attr_t *attr) {
     if (!attr)
       return; 
     coap_free(attr->name.s); 
     coap_free(attr->value.s);    
     coap_free_type(COAP_RESOURCEATTR, attr); 
   }           
void coapFreeResource(coap_resource_t *resource){
	 coap_attr_t *attr, *tmp;
     coap_subscription_t *obs, *otmp;
   
     assert(resource);
   
     /* delete registered attributes */
     LL_FOREACH_SAFE(resource->link_attr, attr, tmp) coapDeleteAttr(attr);
   
     if (resource->flags & COAP_RESOURCE_FLAGS_RELEASE_URI)
       coap_free(resource->uri.s);
   
     /* free all elements from resource->subscribers */
     // modifying this to re-create free resource. CHANGED : COAP_FREE_TYPE(subscription, obs) to coap_free(obs) 
     // check original coap_free_resource()
     LL_FOREACH_SAFE(resource->subscribers, obs, otmp) coap_free(obs);
 
     coap_free_type(COAP_RESOURCE, resource); 
}

namespace coap{

        Option::Option(){}
        Option::Option(coap_pdu_t *request){
          coap_opt_t *option;
          coap_opt_iterator_t opt_iter; 
          coap_option_iterator_init(request, &opt_iter, COAP_OPT_ALL);
          debug("##### available option type #####\n");
          while ((option = coap_option_next(&opt_iter))) {
            debug("Option Type : %ld\n", opt_iter.type);
            coap::UString value(coap_opt_value(option),coap_opt_length(option)); 
            data_.push_back(std::make_pair(opt_iter.type, value));
          }
        }

        bool Option::type_exist(unsigned short type){
          for(std::pair<unsigned short, coap::UString> value : data_){
            if ( value.first == type){ return true;}
          }
          return false;
        }

        coap::UString Option::get_type_value(unsigned short type){
          for(std::pair<unsigned short, coap::UString> value : data_){
            if ( value.first == type){ return value.second;}
          }
          coap::UString temp;
          return temp;
        }

        std::vector<std::pair<unsigned short,UString>> Option::get_data(){
          return data_;
        }

        int Option::decode_value(unsigned short type){
          coap::UString opt_value(get_type_value(type));
          unsigned char *temp = opt_value.copy_uchar_unsafe();
          int result = coap_decode_var_bytes(temp, opt_value.get_length());
          free(temp);
          return result;
        }
}
