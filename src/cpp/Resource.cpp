#include "Resource.hpp"

namespace coap{

        Resource::Resource(){}
        Resource::Resource(coap::UString name, int flags){ 
            init_resource(name, flags);
        }

        Resource::Resource(coap_resource_t *resource){
            resource_ = resource;
        }

        Resource::Resource(coap::LinkFormat link_format, int flags){
            if(link_format.is_valid()){
                init_resource(link_format.get_path(), flags);
                for(coap::Attribute data : link_format.get_attribute_list()){
                    add_attribute(data);
                }
            }
        }

        UString Resource::get_name(){
            unsigned char *name = resource_->uri.s;
            size_t size = resource_->uri.length;
            return (UString(name, size));
        }

        std::vector<Attribute> Resource::get_attribute_list(){
            std::vector<Attribute> temp;
            coap_attr_t *attr;

            LL_FOREACH(resource_->link_attr, attr) {
                temp.push_back(Attribute(UString(attr->name.s, attr->name.length), UString(attr->value.s, attr->value.length), attr->flags));
            }
            return temp;
        }

        Attribute Resource::find_attribute(UString type){
            coap_attr_t* temp = coap_find_attr(resource_,type.get_ustring().c_str(), type.get_length());
            Attribute result;
            if(temp != NULL){
                result = Attribute(temp);
            }            
            return result;
        }

        bool Resource::is_attribute_exist(UString type){
            
            coap_attr_t* temp = coap_find_attr(resource_,type.get_ustring().c_str(), type.get_length());
            if(temp != NULL){
                return true;
            }            
            return false;
        }
        
        
        void Resource::init_resource(coap::UString name, int flags){ 
            resource_ = coap_resource_init(name.copy_uchar_safe(), name.get_length(), flags);
        }
        
        void Resource::register_handler(unsigned char method, coap_method_handler_t handler){
            coap_register_handler(resource_, method, handler);
        }
    
        void Resource::add_attribute(Attribute attribute){
            coap_add_attr(resource_, attribute.get_name().copy_uchar_safe(), 
                          attribute.get_name().get_length(),attribute.get_value().copy_uchar_safe(), 
                          attribute.get_value().get_length(), attribute.get_flags());
        }

        void Resource::add_attribute(Attribute attribute, int flags){
            coap_add_attr(resource_, attribute.get_name().copy_uchar_safe(), 
                          attribute.get_name().get_length(),attribute.get_value().copy_uchar_safe(), 
                          attribute.get_value().get_length(), flags);
        }

        void Resource::set_observable(bool obs_status){
            if (obs_status) resource_->observable = 1;
            else resource_->observable = 0;
        }

        void Resource::set_dirty(bool dirty_status){
            if (dirty_status) resource_->dirty = 1;
            else resource_->dirty = 0;            
        }

        bool Resource::is_observer_exist(Address peer,UString token){
            str temp;
            temp.s = token.copy_uchar_unsafe();
            temp.length = token.get_length();
            bool result = false;
            if (coap_find_observer(resource_, peer.get_address(), &temp) != NULL) result = true;
            free(temp.s);
            return result;
        }

        coap_resource_t*& Resource::get_resource(){
            return resource_;
        }

        void Resource::free_resource(){

            coap_attr_t *attr, *tmp;
            coap_subscription_t *obs, *otmp;
            
            assert(resource_);
            
            /* delete registered attributes */
            LL_FOREACH_SAFE(resource_->link_attr, attr, tmp) coap_delete_attr(attr);
            
            if (resource_->flags & COAP_RESOURCE_FLAGS_RELEASE_URI)
                coap_free(resource_->uri.s);
            
            /* free all elements from resource_->subscribers */
            LL_FOREACH_SAFE(resource_->subscribers, obs, otmp) COAP_FREE_TYPE(subscription, obs);
            
            #ifdef WITH_LWIP
            memp_free(MEMP_COAP_RESOURCE, resource_);
            #endif
            #ifndef WITH_LWIP
            coap_free_type(COAP_RESOURCE, resource_);
            #endif /* WITH_CONTIKI */
        }
}