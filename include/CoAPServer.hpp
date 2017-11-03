#ifndef COAP_SERVER_HPP
#define COAP_SERVER_HPP

#include "coap.h"
#include "CoAPResource.hpp"

class CoAPServer {
    
    private:
        coap_context_t*  ctx;
        coap_address_t   serv_addr;
        fd_set           readfds;  
        void configure_socket(short family, unsigned int ip_address,uint16_t port);
    public:
        CoAPServer();
        void add_resource(coap_resource_t *resource);
        void add_resource(CoAPResource &resource);
        bool is_avaiable();
        coap_context_t* get_context();
        void set_log_level(coap_log_t type);
        void run();
        ~CoAPServer();
    };

#endif