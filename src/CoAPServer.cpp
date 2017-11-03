#include "CoAPServer.hpp"
 
        void CoAPServer::configure_socket(short family, unsigned int ip_address,uint16_t port){
            coap_address_init(&serv_addr);
            serv_addr.addr.sin.sin_family      = family;
            serv_addr.addr.sin.sin_addr.s_addr = ip_address;
            serv_addr.addr.sin.sin_port        = htons(port); //default port
            ctx                                = coap_new_context(&serv_addr);
        }
     
        CoAPServer::CoAPServer(){
            configure_socket(AF_INET6, INADDR_ANY, 5683);
            coap_set_log_level (LOG_DEBUG);
        }

        void CoAPServer::add_resource(coap_resource_t *resource){
            coap_add_resource(ctx, resource);
        }
        void CoAPServer::add_resource(CoAPResource &resource){
            coap_add_resource(ctx, resource.get_resource());
        }

        bool CoAPServer::is_avaiable(){
            if (!ctx) return 0;
            else return 1;
        }
    
        coap_context_t* CoAPServer::get_context(){
            return ctx;
        }
    
        void CoAPServer::set_log_level(coap_log_t type){
            coap_set_log_level (type);
        }
    
        void CoAPServer::run(){
            FD_ZERO(&readfds);
            FD_SET(ctx->sockfd, &readfds );
            /* Block until there is something to read from the socket */
            int result = select( FD_SETSIZE, &readfds, 0, 0, NULL );
            if ( result < 0 ) {         /* error */
                perror("select");
                exit(EXIT_FAILURE);
            } else if ( result > 0 ) {  /* read from socket */
                if ( FD_ISSET( ctx->sockfd, &readfds ) ) 
                    coap_read( ctx );       
            }
        }
        CoAPServer::~CoAPServer(){
            coap_delete_all_resources(ctx);
            coap_free_context(ctx);
        } 