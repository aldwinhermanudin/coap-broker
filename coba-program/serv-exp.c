#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include "coap.h"
#include "sensorNode.h"

int queryToInt(char *query, unsigned int len) {
	unsigned int i;
	char x;
	int result;
	
	for (i = 0; i < len; i++) {
		if (query[i] >= 48 && query[i] <= 57) {
			break;
		}
	}
	result = query[i++] - '0';
	for (; i < len; i++) {
		result = result * 10 + (query[i] - '0');
	}
	return result;
} 

void queryValue(const coap_pdu_t *pdu) {
	unsigned char buf[COAP_MAX_PDU_SIZE]; /* need some space for output creation */
    size_t buf_len = 0; /* takes the number of bytes written to buf */
    int encode = 0, have_options = 0, i, j, age;
    coap_opt_iterator_t opt_iter;
    coap_opt_t *option;
    int content_format = -1;
    size_t data_len;
    unsigned char *data;

	coap_option_iterator_init((coap_pdu_t *)pdu, &opt_iter, COAP_OPT_ALL);
	
	while ((option = coap_option_next(&opt_iter))) {
       switch (opt_iter.type) {
			case COAP_OPTION_URI_QUERY:
				printf("coap_opt_value: %s\n", coap_opt_value(option));
				//return queryToInt(coap_opt_value(option), 
					//strlen(coap_opt_value(option)));	
			break;
	   }
    }
}

/**
 * This function prepares the index resource
 * 
 * a. coap_endpoint_t: Abstraction of virtual endpoint that can be attached to coap_context_t
 * b. coap_pdu_t: Header structure for CoAP PDUs (Protocol Data Units) 
 */ 
static void
index_handler(coap_context_t *ctx,
              struct coap_resource_t *resource,
              const coap_endpoint_t *local_interface,
              coap_address_t *peer,
              coap_pdu_t *request,
              str *token,
              coap_pdu_t *response) {

    const char* index = "Hello World!";
    unsigned char buf[3];
    response->hdr->code = COAP_RESPONSE_CODE(205); // Why 205?
    
    /* size_t coap_add_option 	( 	coap_pdu_t *  	pdu,
     * 		unsigned short  	type,
     * 		unsigned int  	len,
     * 		const unsigned char *  	data)
     * 
     * de-duplicate code with coap_add_option_later
     * Adds option of given type to pdu that is passed as first parameter.  	
     */
    coap_add_option(response,
                  COAP_OPTION_CONTENT_TYPE,
                  coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);
    coap_add_option(response,
                  COAP_OPTION_MAXAGE,
                  coap_encode_var_bytes(buf, 0x2ffff), buf);
    
	/* int coap_add_data 	( 	coap_pdu_t *  	pdu,
	 * 		unsigned int  	len,
	 * 		const unsigned char *  	data) 	
	 * 
	 * Adds given data to the pdu that is passed as first parameter.
	 * Note that the PDU's data is destroyed by coap_add_option(). 
	 * coap_add_data() must be called only once per PDU, otherwise the result is undefined. 
	 */
    coap_add_data(response, strlen(index), (unsigned char *)index);
}

static void
sensor_handler(coap_context_t *ctx,
              struct coap_resource_t *resource,
              const coap_endpoint_t *local_interface,
              coap_address_t *peer,
              coap_pdu_t *request,
              str *token,
              coap_pdu_t *response) {

    char index[5];
    unsigned short val;
    unsigned char buf[3];
    
    val = getADC(0);
    //val = 1000;
    sprintf(index, "%d", val);
    response->hdr->code = COAP_RESPONSE_CODE(205); // Why 205?
    
    coap_add_option(response,
                  COAP_OPTION_CONTENT_TYPE,
                  coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);
    coap_add_option(response,
                  COAP_OPTION_MAXAGE,
                  coap_encode_var_bytes(buf, 60), buf);
    
    coap_add_data(response, strlen(index), (unsigned char *)index);
}

static void
actuator_handler(coap_context_t *ctx,
              struct coap_resource_t *resource,
              const coap_endpoint_t *local_interface,
              coap_address_t *peer,
              coap_pdu_t *request,
              str *token,
              coap_pdu_t *response) {

    char index[7];
    unsigned short val;
    unsigned char buf[3];
    
    turnGPIO(3, 0);
    queryValue(request);
    //printf("value of query: %d\n", queryValue(request));
    strcpy(index, "sukses");
    response->hdr->code = COAP_RESPONSE_CODE(205); // Why 205?
    
    coap_add_option(response,
                  COAP_OPTION_CONTENT_TYPE,
                  coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);
    coap_add_option(response,
                  COAP_OPTION_MAXAGE,
                  coap_encode_var_bytes(buf, 60), buf);
    
    coap_add_data(response, strlen(index), (unsigned char *)index);
    printf("token: %s\n", token->s);
    printf("pdu request data: %s\n", request->data);
    printf("resource uri: %s\n", resource->uri.s);
    //printf("resource link attr value: %d\n", resource->link);
    printf("ambilOprion:\n");
    //ambilOption(request);
}

int main(int argc, char* argv[]){
    coap_context_t  *ctx; // The CoAP stack's global state
    coap_address_t serv_addr; // Multi-purpose address abstraction
    coap_resource_t *index; // buat diisi sama coap_resource_init
    fd_set readfds; // TODO: cari ini apa
    
    /* Prepare the CoAP server socket */ 
    coap_address_init(&serv_addr);
    serv_addr.addr.sin.sin_family = AF_INET6;
    serv_addr.addr.sin.sin_addr.s_addr = INADDR_ANY;
    serv_addr.addr.sin.sin_port = htons(5683); //This is the port
    ctx = coap_new_context(&serv_addr);
    if (!ctx)
        exit(EXIT_FAILURE);
    
    /* coap_resource_init(): Creates a new resource object and initializes the link field to the string of length len
     * coap_resource_init(const unsigned char * uri, 
     * 			size_t  len, 
     * 			int  flags )
     * 
     * uri	The URI path of the new resource.
     * len	The length of uri.
     * flags	Flags for memory management (in particular release of memory).
     */
    index = coap_resource_init(NULL, 0, 0);
    
    /* coap_register_handler(coap_resource_t *resource,
     * 			unsigned char method,
     * 			coap_method_handler_t handler)
     * 
     * resource	The resource for which the handler shall be registered.
     * method	The CoAP request method to handle.
     * handler	The handler to register with resource.  
     * 
     * Registers the specified handler as message handler for the request type method
     */
    coap_register_handler(index, COAP_REQUEST_GET, index_handler);
    
    /* void coap_add_resource 	(coap_context_t *  	context, 
     * 		coap_resource_t *  	resource) 	
     * 
     * Parameters
     * context	The context to use.
     * resource	The resource to store. 
     * 
     * Registers the given resource for context.
     * The resource must have been created by coap_resource_init(), 
     * the storage allocated for the resource will be released by coap_delete_resource().
     */
    coap_add_resource(ctx, index);
    
    index = coap_resource_init((unsigned char *)"sensor", strlen("sensor"), 0);
    coap_register_handler(index, COAP_REQUEST_GET, sensor_handler);
    coap_add_resource(ctx, index);
    
    index = coap_resource_init((unsigned char *)"actuator", strlen("actuator"), 0);
    coap_register_handler(index, COAP_REQUEST_GET, actuator_handler);
    coap_add_resource(ctx, index);
    printf("index uri: %s\n", index->uri.s);
    while (1) {
        FD_ZERO(&readfds);
        FD_SET( ctx->sockfd, &readfds );
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
    
    coap_free_context(ctx);
		
    return 0;
}

