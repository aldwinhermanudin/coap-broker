#include "UString.hpp"
  
  namespace coap{
    UString::UString(){}
    UString::UString(const unsigned char* input, size_t length){
        data_ = ustring(input, length);;
    }
    UString::UString(const char* input){
        data_ = reinterpret_cast<const unsigned char*>(input); // using reinterpret_cast to preserve original binary data
    }
    UString::UString(std::string input){
        data_ = reinterpret_cast<const unsigned char*>(input.c_str()); // using reinterpret_cast to preserve original binary data
    }

    std::ostream& operator<<( std::ostream &output, const UString &input ) { 
        output << input.data_.c_str();
        return output;            
    }
 
    // with null-terminated
    unsigned char* UString::copy_uchar_safe(){
        unsigned char * temp = (unsigned char*) malloc (sizeof(unsigned char) * (data_.length() + 1));
        memcpy(temp, &(data_.c_str()[0]), data_.length());
        temp[data_.length()] = '\0';
        return temp;
    }

    // without null-terminated
    unsigned char* UString::copy_uchar_unsafe(){
        unsigned char * temp = (unsigned char*) malloc (sizeof(unsigned char) * (data_.length()));
        memcpy(temp, &(data_.c_str()[0]), data_.length());
        return temp;
    }

    // with null-terminated
    char* UString::copy_char_safe(){
        char * temp = reinterpret_cast<char*>(copy_uchar_safe());
        return temp;
    }

    // without null-terminated
    char* UString::copy_char_unsafe(){
        char * temp = reinterpret_cast<char*>(copy_uchar_unsafe());
        return temp;
    }

    ustring UString::get_ustring(){
        return data_;
    }

    std::string UString::get_string(){
        std::string temp(reinterpret_cast<const char*>(data_.c_str()));
        return temp;
    }

    size_t UString::get_length(){
        return data_.length();
    } 

    bool UString::is_equal(UString data){
        if(data_.compare(data.get_ustring()) == 0)return true;
        
        else return false;        
    }
  }
    