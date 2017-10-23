#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

extern "C" {
#include "coap.h"
#include <string.h>
}
#define TESTMODE
#define TEMPMODE

#ifdef __GNUC__
#define UNUSED_PARAM __attribute__ ((unused))
#else /* not a GCC */
#define UNUSED_PARAM
#endif /* GCC */

#ifdef TEMPMODE

	char broker_path[8] = "ps";
	char temp_topic_path[8] = "ps/nama";
#endif
/*
 * The resource handler
 */ 
static void hnd_get_name(coap_context_t *ctx, struct coap_resource_t *resource, 
              const coap_endpoint_t *local_interface, coap_address_t *peer, 
              coap_pdu_t *request, str *token, coap_pdu_t *response);

static void hnd_put_name(coap_context_t *ctx ,
             struct coap_resource_t *resource ,
             const coap_endpoint_t *local_interface ,
             coap_address_t *peer ,
             coap_pdu_t *request,
             str *token ,
             coap_pdu_t *response);
             
static void hnd_delete_name(coap_context_t *ctx ,
                struct coap_resource_t *resource ,
                const coap_endpoint_t *local_interface ,
                coap_address_t *peer ,
                coap_pdu_t *request ,
                str *token ,
                coap_pdu_t *response ); 
                
static void hnd_post_name(coap_context_t *ctx ,
                struct coap_resource_t *resource ,
                const coap_endpoint_t *local_interface ,
                coap_address_t *peer ,
                coap_pdu_t *request ,
                str *token ,
                coap_pdu_t *response );
                
static void hnd_get_topic(coap_context_t *ctx, struct coap_resource_t *resource, 
              const coap_endpoint_t *local_interface, coap_address_t *peer, 
              coap_pdu_t *request, str *token, coap_pdu_t *response);

static void hnd_put_topic(coap_context_t *ctx ,
             struct coap_resource_t *resource ,
             const coap_endpoint_t *local_interface ,
             coap_address_t *peer ,
             coap_pdu_t *request,
             str *token ,
             coap_pdu_t *response);
             
static void hnd_delete_topic(coap_context_t *ctx ,
                struct coap_resource_t *resource ,
                const coap_endpoint_t *local_interface ,
                coap_address_t *peer ,
                coap_pdu_t *request ,
                str *token ,
                coap_pdu_t *response ); 
                
static void hnd_post_topic(coap_context_t *ctx ,
                struct coap_resource_t *resource ,
                const coap_endpoint_t *local_interface ,
                coap_address_t *peer ,
                coap_pdu_t *request ,
                str *token ,
                coap_pdu_t *response );
             
static void hnd_get_broker(coap_context_t *ctx, struct coap_resource_t *resource, 
              const coap_endpoint_t *local_interface, coap_address_t *peer, 
              coap_pdu_t *request, str *token, coap_pdu_t *response);

static void hnd_put_broker(coap_context_t *ctx,
             struct coap_resource_t *resource,
             const coap_endpoint_t *local_interface,
             coap_address_t *peer,
             coap_pdu_t *request,
             str *token,
             coap_pdu_t *response);
             
static void hnd_post_broker(coap_context_t *ctx,
             struct coap_resource_t *resource,
             const coap_endpoint_t *local_interface,
             coap_address_t *peer,
             coap_pdu_t *request,
             str *token,
             coap_pdu_t *response);


int main(int argc, char* argv[])
{
	coap_context_t*  ctx;
	coap_address_t   serv_addr;
	coap_resource_t* observable_resource;
	coap_resource_t* broker_resource;
	fd_set           readfds;    
	
	/* Prepare the CoAP server socket */ 
	coap_address_init(&serv_addr);
	serv_addr.addr.sin.sin_family      = AF_INET6;
	serv_addr.addr.sin.sin_addr.s_addr = INADDR_ANY;
	serv_addr.addr.sin.sin_port        = htons(5683); //default port
	ctx                                = coap_new_context(&serv_addr);
	if (!ctx) exit(EXIT_FAILURE);
	/* Prepare the CoAP server socket */ 
	
	/* Initialize the observable resource */
	observable_resource = coap_resource_init((unsigned char *)temp_topic_path, strlen(temp_topic_path), 0);
	coap_register_handler(observable_resource, COAP_REQUEST_GET, hnd_get_name);
	coap_register_handler(observable_resource, COAP_REQUEST_PUT, hnd_put_name);
	coap_register_handler(observable_resource, COAP_REQUEST_DELETE, hnd_delete_name);
	coap_register_handler(observable_resource, COAP_REQUEST_POST, hnd_post_name);
	observable_resource->observable = 1;
	coap_add_resource(ctx, observable_resource);	
	//addToDatabase(db, temp_topic_path, "Akbar");
	/* Initialize the observable resource */	
	
	/* Initialize the observable resource */
	broker_resource = coap_resource_init(broker_path, strlen(broker_path), 0);
	coap_register_handler(broker_resource, COAP_REQUEST_GET, hnd_get_broker);
	coap_register_handler(broker_resource, COAP_REQUEST_POST, hnd_post_broker);
	coap_add_attr(broker_resource, (unsigned char *)"ct", 2, (unsigned char *)"40", strlen("40"), 0); //40 :link
	coap_add_attr(broker_resource, (unsigned char *)"rt", 2, (unsigned char *)"core.ps", strlen("core.ps"), 0);
	coap_add_resource(ctx, broker_resource);	
	//addToDatabase(db, broker_path, "Aldwin");
	/* Initialize the observable resource */	
	
	
	
	/*Listen for incoming connections*/	
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
        
        // coap_check_notify is needed if there is a observable resource
        coap_check_notify(ctx);
    }
    
    coap_free_context(ctx);  
}

/*
 * The resource handler
 */ 

static void
hnd_get_name(coap_context_t *ctx, struct coap_resource_t *resource, 
              const coap_endpoint_t *local_interface, coap_address_t *peer, 
              coap_pdu_t *request, str *token, coap_pdu_t *response) 
{
	unsigned char buf[3];
	//int resource_available = (!checkString(getDatabaseValue(db,temp_topic_path), " "));
	int resource_available = 1;
	
	//const char* response_data     = getDatabaseValue(db,temp_topic_path);
	response->hdr->code           = resource_available ? COAP_RESPONSE_CODE(205) : COAP_RESPONSE_CODE(404);
	
	if (coap_find_observer(resource, peer, token)) {
    /* FIXME: need to check for resource->dirty? */
		coap_add_option(response, COAP_OPTION_OBSERVE, coap_encode_var_bytes(buf, ctx->observe), buf);
	} 
	
	if (resource_available){	
		coap_add_option(response, COAP_OPTION_CONTENT_TYPE, coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);
		//coap_add_data  (response, 3, "LOL");
	}
		
}

static void
hnd_put_name(coap_context_t *ctx UNUSED_PARAM,
             struct coap_resource_t *resource UNUSED_PARAM,
             const coap_endpoint_t *local_interface UNUSED_PARAM,
             coap_address_t *peer UNUSED_PARAM,
             coap_pdu_t *request,
             str *token UNUSED_PARAM,
             coap_pdu_t *response) {
  size_t size;
  unsigned char *data;

  /* FIXME: re-set my_clock_base to clock_offset if my_clock_base == 0
   * and request is empty. When not empty, set to value in request payload
   * (insist on query ?ticks). Return Created or Ok.
   */

  /* if my_clock_base was deleted, we pretend to have no such resource */
  response->hdr->code = COAP_RESPONSE_CODE(204);
	
  // set dirty to 1 to signal libcoap engine, to send notification to subscriber
  resource->dirty = 1;

  /* coap_get_data() sets size to 0 on error */
  (void)coap_get_data(request, &size, &data);
  //setDatabaseValue(db, temp_topic_path, data);
}

static void
hnd_delete_name(coap_context_t *ctx ,
                struct coap_resource_t *resource ,
                const coap_endpoint_t *local_interface ,
                coap_address_t *peer ,
                coap_pdu_t *request ,
                str *token ,
                coap_pdu_t *response ) {

//removeFromDatabase(db, temp_topic_path);

// this will delete the coap_resource_t
coap_delete_resource(ctx, resource->key);
}

static void
hnd_post_name(coap_context_t *ctx UNUSED_PARAM,
                struct coap_resource_t *resource UNUSED_PARAM,
                const coap_endpoint_t *local_interface UNUSED_PARAM,
                coap_address_t *peer UNUSED_PARAM,
                coap_pdu_t *request UNUSED_PARAM,
                str *token UNUSED_PARAM,
                coap_pdu_t *response UNUSED_PARAM) {
					
response->hdr->code = COAP_RESPONSE_CODE(204);
//addToDatabase(db, temp_topic_path, "Akbar");
}

static void
hnd_get_broker(coap_context_t *ctx, struct coap_resource_t *resource, 
              const coap_endpoint_t *local_interface, coap_address_t *peer, 
              coap_pdu_t *request, str *token, coap_pdu_t *response) 
{
	unsigned char buf[3];
	//int resource_available = (!checkString(getDatabaseValue(db,broker_path), " "));
	int resource_available = 1;
	
	//const char* response_data     = getDatabaseValue(db,broker_path);
	response->hdr->code           = resource_available ? COAP_RESPONSE_CODE(201) : COAP_RESPONSE_CODE(404);
	
	if (resource_available){	
		// option order matters!
		coap_add_option(response, COAP_OPTION_CONTENT_TYPE, coap_encode_var_bytes(buf, COAP_MEDIATYPE_APPLICATION_LINK_FORMAT), buf);
		coap_add_option(response, COAP_OPTION_MAXAGE,coap_encode_var_bytes(buf, 20), buf); // mac-age in seconds, so 30 seconds (to mars. #pun)
		//coap_add_data  (response, 3, "LOL");
	}
		
}

static void
hnd_post_broker(coap_context_t *ctx, struct coap_resource_t *resource, 
              const coap_endpoint_t *local_interface, coap_address_t *peer, 
              coap_pdu_t *request, str *token, coap_pdu_t *response) 
{
	coap_resource_t *new_resource;
	size_t size;
    unsigned char *data;

	response->hdr->code = COAP_RESPONSE_CODE(201);
	
	(void)coap_get_data(request, &size, &data);
	
	/* this part deals with creating resource */
	// TODO: if payload legit do below
	// parse *data above to get loc
	unsigned char *loc;
	loc = (unsigned char *)coap_malloc(size);
	memcpy(loc, data, size);
	
	// get dummy_ct_var from parsing *data above
	char dummy_ct_var[2] = "40";
	
	unsigned char *ct_value;
	size_t ct_size = 2;
	ct_value = (unsigned char *)coap_malloc(ct_size);
	memcpy(ct_value, dummy_ct_var, ct_size);
		
	//everytime we init resource, the mem address of all the parameter given are not copied, rather are pointed (using a pointer). so never coap_free(loc)
	new_resource = coap_resource_init(loc, size, 0);
	coap_register_handler(new_resource, COAP_REQUEST_GET, hnd_get_topic);
	coap_register_handler(new_resource, COAP_REQUEST_POST, hnd_post_topic);
	coap_register_handler(new_resource, COAP_REQUEST_PUT, hnd_put_topic);
	coap_register_handler(new_resource, COAP_REQUEST_DELETE, hnd_delete_topic);
	coap_add_attr(new_resource, (unsigned char *)"ct", 2, ct_value, ct_size, 0);
	new_resource->observable = 1;
	coap_add_resource(ctx, new_resource);
	/*  this part deals with creating resource  */
	
	// setting Location Path is as easy as this. not sure if this is correcct though
	coap_add_option(response, COAP_OPTION_LOCATION_PATH, size, data);
	
	// debug stuff
	char result[128] = "LOL";
	sprintf(result, "%s %d",loc, (int)size);
	//setDatabaseValue(db, broker_path, result); 
}

static void hnd_get_topic(coap_context_t *ctx, struct coap_resource_t *resource, 
              const coap_endpoint_t *local_interface, coap_address_t *peer, 
              coap_pdu_t *request, str *token, coap_pdu_t *response){
					
response->hdr->code = COAP_RESPONSE_CODE(205);
}

static void hnd_put_topic(coap_context_t *ctx ,
             struct coap_resource_t *resource ,
             const coap_endpoint_t *local_interface ,
             coap_address_t *peer ,
             coap_pdu_t *request,
             str *token ,
             coap_pdu_t *response){
					
response->hdr->code = COAP_RESPONSE_CODE(205);
}
            
static void hnd_delete_topic(coap_context_t *ctx ,
                struct coap_resource_t *resource ,
                const coap_endpoint_t *local_interface ,
                coap_address_t *peer ,
                coap_pdu_t *request ,
                str *token ,
                coap_pdu_t *response ){
					
response->hdr->code = COAP_RESPONSE_CODE(205);
}
                
static void hnd_post_topic(coap_context_t *ctx ,
                struct coap_resource_t *resource ,
                const coap_endpoint_t *local_interface ,
                coap_address_t *peer ,
                coap_pdu_t *request ,
                str *token ,
                coap_pdu_t *response ){
					
response->hdr->code = COAP_RESPONSE_CODE(205);
}

