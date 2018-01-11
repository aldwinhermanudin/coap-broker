#include "Server.hpp"
 
 namespace coap{
        void Server::configure_socket(short family, unsigned int ip_address,uint16_t port){
            coap_address_init(&serv_addr_);
            serv_addr_.addr.sin.sin_family      = family;
            serv_addr_.addr.sin.sin_addr.s_addr = ip_address;
            serv_addr_.addr.sin.sin_port        = htons(port); //default port
            ctx_                                = coap_new_context(&serv_addr_);
        }
     
        Server::Server(){
            configure_socket(AF_INET6, INADDR_ANY, 5683);
            coap_set_log_level (LOG_DEBUG);
        }

        Server::Server(coap_context_t* ctx){
            ctx_ = ctx;
            /* need to copy fd_set and coap_address_t data */
        }

        void Server::add_resource(coap_resource_t *resource){
            coap_add_resource(ctx_, resource);
        }
        void Server::add_resource(Resource &resource){
            coap_add_resource(ctx_, resource.get_resource());
        }

        bool Server::is_avaiable(){
            if (!ctx_) return 0;
            else return 1;
        }
    
        coap_context_t*& Server::get_context(){
            return ctx_;
        }
    
        void Server::set_log_level(coap_log_t type){
            coap_set_log_level (type);
        }
    
        void Server::run(){
            FD_ZERO(&readfds_);
            FD_SET(ctx_->sockfd, &readfds_ );
            /* Block until there is something to read from the socket */
            int result = select( FD_SETSIZE, &readfds_, 0, 0, NULL );
            if ( result < 0 ) {         /* error */
                perror("select");
                exit(EXIT_FAILURE);
            } else if ( result > 0 ) {  /* read from socket */
                if ( FD_ISSET( ctx_->sockfd, &readfds_ ) ) 
                    coap_read( ctx_ );       
            }
        }

        bool Server::is_resource_exist(UString uri){
            RESOURCES_ITER(ctx_->resources, r) {
                Resource temp(r);
                if(uri.is_equal(temp.get_name())){
                    return true;
                }
		    }
            return false;
        }

        Resource Server::get_resource(UString uri){
            Resource temp;
            RESOURCES_ITER(ctx_->resources, r) {
                Resource temp(r);
                if(uri.is_equal(temp.get_name())){
                    temp = Resource(r);
                    break;
                }
		    }
            return temp;
        }

        void Server::delete_resource(Resource resource){
            RESOURCES_DELETE(ctx_->resources, resource.get_resource());
            resource.free_resource();
        }

        bool Server::delete_resource(UString uri){

            RESOURCES_ITER(ctx_->resources, r) {
                
                Resource res(r);
                if(res.get_name().is_equal(uri)){
                    RESOURCES_DELETE(ctx_->resources, r);
                    res.free_resource();
                    return true;
                }
            }
            return false;
        }

        int Server::delete_resource(Resource comparison ,std::function< bool(Resource,Resource) >& comparator){
            int counter = 0;
            RESOURCES_ITER(ctx_->resources, r) {
                
                Resource res(r);
                if(comparator(comparison,res)){
                    delete_resource(res);
                    counter++;
                }
            }
            return counter;
        }

        void Server::notify(){
            coap_check_notify(ctx_);
        }

        void Server::end_server(){
            coap_delete_all_resources(ctx_);
            coap_free_context(ctx_);
        } 
 }