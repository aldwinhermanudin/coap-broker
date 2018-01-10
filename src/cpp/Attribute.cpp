#include "Attribute.hpp"

namespace coap{

        Attribute::Attribute(){}
        Attribute::Attribute(UString name, UString value, int flags){
            name_ = name;
            value_ = value;
            flags_ = flags;
        }
        Attribute::Attribute(coap_attr_t* attribute){
            name_ = UString(attribute->name.s,attribute->name.length);
            value_ = UString(attribute->value.s,attribute->value.length);
            flags_ = attribute->flags;
        }
        UString Attribute::get_name(){
            return name_;
        }
        UString Attribute::get_value(){
            return value_;
        }
        int Attribute::get_flags(){
            return flags_;
        }
        void Attribute::set_name(UString name){
            name_ = name;
        }
        void Attribute::set_value(UString value){
            value_ = value;
        }
        void Attribute::set_flags(int flags){
            flags_ = flags;
        }

}