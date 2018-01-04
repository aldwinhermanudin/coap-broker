#ifndef USTRING_HPP
#define USTRING_HPP

#include <iostream>
#include <cstring>
#include <string>
#include <memory>

namespace coap{
    using ustring = std::basic_string<unsigned char>;
    class UString {
        
    private:
        ustring data_;
    public:
        UString();
        UString(unsigned char* input, size_t length);
        UString(const char* input);
        UString(std::string input);
        friend std::ostream &operator<<( std::ostream &output, const UString &input ); 
        unsigned char* copy_uchar_safe();
        unsigned char* copy_uchar_unsafe();
        char* copy_char_safe();
        char* copy_char_unsafe();
        ustring get_ustring();
        std::string get_string();
        size_t get_length();
    };
}

#endif