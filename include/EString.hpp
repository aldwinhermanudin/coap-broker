#ifndef ESTRING_HPP
#define ESTRING_HPP

#include <iostream>
#include <cstring>
#include <string>
#include <memory>

class EString {
    
private:
    std::string string_data;
    std::shared_ptr<unsigned char> uchar_data;
    std::shared_ptr<char> char_data;
    void set_data(const char* input);
    void set_data(std::string input);
public:
    EString();
    EString(const char* input);
    EString(std::string input);
    EString(const EString &input);
    EString operator=(const char* input);
    EString operator=(std::string input);
    EString operator=(const EString &input);
    friend std::ostream &operator<<( std::ostream &output, const EString &input );
    std::shared_ptr<unsigned char> get_uchar();
    std::shared_ptr<char> get_char();
    unsigned char* copy_uchar();
    char* copy_char();
    std::string get_string();
    size_t get_length();
};

#endif