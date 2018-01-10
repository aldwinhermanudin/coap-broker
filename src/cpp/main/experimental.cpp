#include <iostream>
#include <vector>
#include <signal.h>
#include <cstring>
#include <memory>
#include "coap.h"
#include <MQTTClient.h>
#include <LinkedListDB.hpp>
#include "UString.hpp"
#include "Resource.hpp"
#include "Server.hpp"
#include "BrokerResource.hpp"


using namespace std; 
using namespace coap;

int main(){
    string sample2 = "banana";
    UString banana (sample2);
    UString b4(banana);
    UString b1;
    {
        b1 = UString("bob");
    }
    cout << b1 << endl;
    UString b3("asasdas");
    cout << b3 << endl;
    cout << banana << endl;
    cout << b4 << endl;
    cout << b4.get_length() << endl;
    UString sample3 = UString("qwqwqw");
    unsigned char *new_sample = sample3.copy_uchar_safe();
    cout << new_sample  << endl;

    printf("%s\n", new_sample); 

    char * new_char_sample = banana.copy_char_safe();
    free(new_char_sample);
    cout << new_char_sample << endl;
    cout << sample3.get_string() << endl;

    UString new_uchar(new_sample, sample3.get_length());
    cout << new_uchar << endl;

    free(new_sample);

    UString path("<as/sd/asd>");

    std::string s = "<as/sd/asd>;ct=10;rt=\"banana\"";
    
    std::cout << coap::LinkFormat::check_link_format(path) << endl;

    coap::UString s2 ("<assdsdad>;ct=10;rt=\"sda\"");

    if ( coap::LinkFormat::check_link_format(s2))
    std::cout << "string object matched\n";
    else
    std::cout << "string object not matched\n";
    coap::LinkFormat test(s2, 3);
    cout << "Path : " << test.get_path() << endl;
    for( auto val : test.get_attribute_list()){
        cout  << "Attr : " << val.get_value() << endl;
    }

    std::string str = s;
    std::vector<std::string> cont;
    char delim = ';';
    std::size_t current, previous = 0;
    current = str.find(delim);
    while (current != std::string::npos) {
        cont.push_back(str.substr(previous, current - previous));
        previous = current + 1;
        current = str.find(delim, previous);
    }
    cont.push_back(str.substr(previous, current - previous));
    for(auto val:cont){
        cout << val << endl;
    }

    std::string bananana = "<as/sd/asd>;ct=10;rt=banana";
    boost::erase_all(bananana, "\"");
    cout << bananana << endl;


    std::string text = "<as/sd/asd>;ct=10;rt=banana";
    std::vector<std::string> results;
    boost::split(results, text, [](char c){return c == ';';});

    for ( std::string value : results){
        cout << value << endl;
    }

     std::string lf_example = "<as/sd/asd>;ct=10;rt=banana;cs=\"lol\"";
     coap::LinkFormat lf_alpha(lf_example,3);
     cout << lf_alpha.get_path() << endl;
      for( auto val : lf_alpha.get_attribute_list()){
        cout  << "Attr : " << val.get_name() << " " << val.get_value() << endl;
    }

    return 0;
}