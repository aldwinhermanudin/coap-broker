#include <iostream>
#include <vector>
#include <signal.h>
#include <cstring>
#include <memory>
#include "coap.h"
#include <LinkedListDB.hpp>
#include "CoAPRD.hpp"
#include "EString.hpp"
#include "CoAPResource.hpp"
#include "CoAPServer.hpp"

using namespace std;
/* temporary storage for dynamic resource representations */
static int quit = 0;
/* SIGINT handler: set quit to 1 for graceful termination */
static void
handle_sigint(int signum) {
  quit = 1;
}

int main(){
 
    CoAPServer coap_server; 

    CoAPRD coap_rd(EString("rd")); 

    coap_server.add_resource(coap_rd);  

    signal(SIGINT, handle_sigint);
    while (!quit) {
        coap_server.run();
    }   

    return 0;
}