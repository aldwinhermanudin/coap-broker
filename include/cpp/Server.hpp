#ifndef COAP_SERVER_HPP
#define COAP_SERVER_HPP

#include "coap.h"
#include "resource.h"
#include "Resource.hpp"

namespace coap{
    class Server {
        
        private:
            coap_context_t*  ctx_;
            coap_address_t   serv_addr_;
            fd_set           readfds_;  
            void configure_socket(short family, unsigned int ip_address,uint16_t port);
        public:
            Server();
            Server(coap_context_t* ctx);
            void add_resource(coap_resource_t *resource);
            void add_resource(Resource &resource);
            bool is_avaiable();
            coap_context_t*& get_context();
            void set_log_level(coap_log_t type);
            bool is_resource_exist(UString uri);
            Resource get_resource(UString uri);
            void delete_resource(Resource resource);
            bool delete_resource(UString uri);
            int delete_resource(Resource comparison, std::function< bool(Resource,Resource) >& comparator);
            unsigned int get_obs_value();
            void notify();
            void run();
            void end_server();
    };
}

#endif