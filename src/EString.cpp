#include "EString.hpp"
 
    void EString::set_data(const char* input){
        string_data = (input);
        uchar_data.reset(new unsigned char[string_data.length()+1], std::default_delete<unsigned char[]>());
        strcpy( (char*)( uchar_data.get() ), string_data.c_str() );
        char_data.reset(new char[string_data.length()+1], std::default_delete<char[]>());
        strcpy( (char*)( char_data.get() ), string_data.c_str() );
    }
    void EString::set_data(std::string input){
        set_data(input.c_str());
    } 

    EString::EString(){}
    EString::EString(const char* input){
        set_data(input);
    }
    EString::EString(std::string input){
        set_data(input);
    }
    EString::EString(const EString &input){
        set_data(input.string_data);
    }
    EString EString::operator=(const char* input){
        set_data(input);
    }
    EString EString::operator=(std::string input){
        set_data(input);
    }
    EString EString::operator=(const EString &input){
        set_data(input.string_data);
    }

    std::ostream& operator<<( std::ostream &output, const EString &input ) { 
        output << input.string_data ;
        return output;            
    }
 
    std::shared_ptr<unsigned char> EString::get_uchar(){
        return uchar_data;
    }

    std::shared_ptr<char> EString::get_char(){
        return char_data;
    }

    unsigned char* EString::copy_uchar(){
        unsigned char * temp = (unsigned char*) malloc (sizeof(unsigned char) * (string_data.length() + 1));
        strcpy( (char*)( temp ), string_data.c_str() );
        return temp;
    }

    char* EString::copy_char(){
        char * temp = (char*) malloc (sizeof(char) * (string_data.length() + 1));
        strcpy( (char*)( temp ), string_data.c_str() );
        return temp;
    }

    std::string EString::get_string(){
        return string_data;
    }
    size_t EString::get_length(){
        return string_data.length();
    } 

    