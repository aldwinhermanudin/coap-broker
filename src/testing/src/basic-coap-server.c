
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include "coap_config.h"
#include "resource.h"
#include "coap.h"


/* temporary storage for dynamic resource representations */
static int quit = 0;
/* SIGINT handler: set quit to 1 for graceful termination */
static void
handle_sigint(int signum) {
  quit = 1;
}

/*
 * The resource handler
 */ 
static void
hello_handler(coap_context_t *ctx, struct coap_resource_t *resource, 
              const coap_endpoint_t *local_interface, coap_address_t *peer, 
              coap_pdu_t *request, str *token, coap_pdu_t *response) 
{
	unsigned char buf[3];
	unsigned char response_data[]     = "Hello World!";
	response->hdr->code           = COAP_RESPONSE_CODE(205);
	coap_add_option(response, COAP_OPTION_CONTENT_TYPE, coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);
	coap_add_data  (response, strlen(response_data), (unsigned char *)response_data);
}

int main(int argc, char* argv[])
{
	coap_context_t*  ctx;
	coap_address_t   serv_addr;
	fd_set           readfds;    
	/* Prepare the CoAP server socket */ 
	coap_address_init(&serv_addr);
	serv_addr.addr.sin.sin_family      = AF_INET;
	serv_addr.addr.sin.sin_addr.s_addr = INADDR_ANY;
	serv_addr.addr.sin.sin_port        = htons(5683); //default port
	ctx                                = coap_new_context(&serv_addr);
	if (!ctx) exit(EXIT_FAILURE);
	
	char hello_name[] = "hello";
	/* Initialize the hello resource */
	
	coap_resource_t* hello_resource = coap_resource_init(hello_name, 5, 0);
	coap_register_handler(hello_resource, COAP_REQUEST_GET, hello_handler);
	coap_add_resource(ctx, hello_resource);
	/*Listen for incoming connections*/
	
	  signal(SIGINT, handle_sigint);
	while (!quit) {
		FD_ZERO(&readfds);
		FD_SET( ctx->sockfd, &readfds );
		int result = select( FD_SETSIZE, &readfds, 0, 0, NULL );
		if ( result < 0 ) /* socket error */
		{
			exit(EXIT_FAILURE);
		} 
		else if ( result > 0 && FD_ISSET( ctx->sockfd, &readfds )) /* socket read*/
		{	 
				coap_read( ctx );       
		} 
	}
	  coap_delete_all_resources(ctx);
	  coap_free_context(ctx);   
}
