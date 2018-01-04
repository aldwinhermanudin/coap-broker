#include <iostream>
#include <vector>
#include <signal.h>
#include <cstring>
#include <cstdio>
#include <memory>
#include "coap.h"
#include <MQTTClient.h>
#include <LinkedListDB.hpp>
#include "CoAPRD.hpp"
#include "UString.hpp"
#include "CoAPResource.hpp"
#include "CoAPServer.hpp"
#include "CoAPBroker.hpp"

using namespace std; 
using namespace coap;

int quit = 0;
static void handle_sigint(int signum) {
  quit = 1;
}

static void usage( const char *program, const char *version);

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

    return 0;
}