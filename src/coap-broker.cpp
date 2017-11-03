#include <iostream>
#include <vector>
#include <signal.h>
#include <cstring>
#include <memory>
#include "coap.h"
#include <LinkedListDB.hpp>
#include "CoAPRD.hpp"



class EString {
    
private:
    std::string string_data;
    std::shared_ptr<unsigned char> uchar_data;
    std::shared_ptr<char> char_data;
    void set_data(const char* input){
        string_data = (input);
        uchar_data.reset(new unsigned char[string_data.length()+1], std::default_delete<unsigned char[]>());
        strcpy( (char*)( uchar_data.get() ), string_data.c_str() );
        char_data.reset(new char[string_data.length()+1], std::default_delete<char[]>());
        strcpy( (char*)( char_data.get() ), string_data.c_str() );
    }
    void set_data(std::string input){
        set_data(input.c_str());
    }

public:
    EString(){}
    EString(const char* input){
        set_data(input);
    }
    EString(std::string input){
        set_data(input);
    }
    EString(const EString &input){
        set_data(input.string_data);
    }
    EString operator=(const char* input){
        set_data(input);
    }
    EString operator=(std::string input){
        set_data(input);
    }
    EString operator=(const EString &input){
        set_data(input.string_data);
    }

    friend std::ostream &operator<<( std::ostream &output, const EString &input ) { 
        output << input.string_data ;
        return output;            
    }
    std::shared_ptr<unsigned char> get_uchar(){
        return uchar_data;
    }

    std::shared_ptr<char> get_char(){
        return char_data;
    }

    unsigned char* copy_uchar(){
        unsigned char * temp = (unsigned char*) malloc (sizeof(unsigned char) * (string_data.length() + 1));
        strcpy( (char*)( temp ), string_data.c_str() );
        return temp;
    }

    char* copy_char(){
        char * temp = (char*) malloc (sizeof(char) * (string_data.length() + 1));
        strcpy( (char*)( temp ), string_data.c_str() );
        return temp;
    }

    std::string get_string(){
        return string_data;
    }
    auto get_length(){
        return string_data.length();
    }
};

class CoAPResource {

protected:
    coap_resource_t *resource;

public:
    CoAPResource(EString name){ 
        resource = coap_resource_init(name.copy_uchar(), name.get_length(), 0);
    }
    void register_handler(unsigned char method, coap_method_handler_t handler){
        coap_register_handler(resource, method, handler);
    }

    void add_attribute(EString name, EString value){
        coap_add_attr(resource, name.copy_uchar(), 
                      name.get_length(),value.copy_uchar(), 
                      value.get_length(), 0);
    }

    coap_resource_t* get_resource(){
        return resource;
    }  
};

class CoAPServer {
    
    private:
        coap_context_t*  ctx;
        coap_address_t   serv_addr;
        fd_set           readfds;  
        void configure_socket(short family, unsigned int ip_address,uint16_t port){
            coap_address_init(&serv_addr);
            serv_addr.addr.sin.sin_family      = family;
            serv_addr.addr.sin.sin_addr.s_addr = ip_address;
            serv_addr.addr.sin.sin_port        = htons(port); //default port
            ctx                                = coap_new_context(&serv_addr);
        }
    
    public:
        CoAPServer(){
            configure_socket(AF_INET6, INADDR_ANY, 5683);
            coap_set_log_level (LOG_DEBUG);
        }

        void add_resource(coap_resource_t *resource){
            coap_add_resource(ctx, resource);
        }
        void add_resource(CoAPResource &resource){
            coap_add_resource(ctx, resource.get_resource());
        }

        bool is_avaiable(){
            if (!ctx) return 0;
            else return 1;
        }
    
        coap_context_t* get_context(){
            return ctx;
        }
    
        void set_log_level(coap_log_t type){
            coap_set_log_level (type);
        }
    
        void run(){
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
        ~CoAPServer(){
            coap_delete_all_resources(ctx);
            coap_free_context(ctx);
        }
    };

    class SimpleResource : public CoAPResource{
    private:
        static EString response_data ;
        static void hello_handler(coap_context_t *ctx, struct coap_resource_t *resource, 
                      const coap_endpoint_t *local_interface, coap_address_t *peer, 
                      coap_pdu_t *request, str *token, coap_pdu_t *response);

    public:
        SimpleResource( EString name);
    };

    EString SimpleResource::response_data ;
    void SimpleResource::hello_handler(coap_context_t *ctx, struct coap_resource_t *resource, 
                      const coap_endpoint_t *local_interface, coap_address_t *peer, 
                      coap_pdu_t *request, str *token, coap_pdu_t *response) 
        {
            unsigned char buf[3];
            EString rdata = SimpleResource::response_data;
            response->hdr->code           = COAP_RESPONSE_CODE(205);
            coap_add_option(response, COAP_OPTION_CONTENT_TYPE, coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);
            coap_add_data  (response,rdata.get_length(), rdata.copy_uchar());
        }
    SimpleResource::SimpleResource( EString name) : CoAPResource(name){ 
        response_data = "banana";
        register_handler(COAP_REQUEST_GET, SimpleResource::hello_handler);
    }

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

    init_rd_resources(coap_server.get_context()); 

    SimpleResource resource_one(EString("one"));
    resource_one.add_attribute(EString("ct"), EString("12"));
    SimpleResource resource_two(EString("two"));

    coap_server.add_resource(resource_one); 
    coap_server.add_resource(resource_two);  

    signal(SIGINT, handle_sigint);
    while (!quit) {
        coap_server.run();
    }   

    return 0;
}