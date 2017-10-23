#include <coap.h>
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
#define TESTMODE
#define TEMPMODE

#ifdef __GNUC__
#define UNUSED_PARAM __attribute__ ((unused))
#else /* not a GCC */
#define UNUSED_PARAM
#endif /* GCC */

#ifdef TESTMODE
#include <string.h>
struct database_kv
{
   char key[1024];
   char value[1024];
};

#define DB_SIZE 128
int database_kv_size = DB_SIZE;
struct database_kv db[DB_SIZE];
int database_kv_counter = 0;

void addToDatabase(struct database_kv* db_in, char * key, char* value){
	
	sprintf(db_in[database_kv_counter].key, "%s", key);
	sprintf(db_in[database_kv_counter].value, "%s", value);	
	database_kv_counter++;
}

void removeFromDatabase(struct database_kv* db_in, char * key){
	
	for(int i = 0; i < database_kv_size; i++){		
		if (strcmp(db_in[i].key,key) == 0){
			sprintf(db_in[i].key, " ");
			sprintf(db_in[i].value, " ");
			break;
		}
		
	}
}

char* getDatabaseValue( struct database_kv* db_in, char * search){
	for(int i = 0; i < database_kv_size; i++){
		
		if (strcmp(db_in[i].key,search) == 0){
			return db[i].value;
		}
		
	}
	return " ";
}

void setDatabaseValue( struct database_kv* db_in, char * key, char* value){
	for(int i = 0; i < database_kv_size; i++){
		
		if (strcmp(db_in[i].key,key) == 0){
			sprintf(db[i].value, "%s", value);	
		}
		
	}
}

int checkString (char * one, char * two){
	if (strcmp(one,two) == 0){
		return 1;
	}
	else {
		return 0;
	}
}

#endif

#ifdef TEMPMODE

	char pubsub_path[8] = "ps";
#endif
/*
 * The resource handler
 */ 
static void hello_handler(coap_context_t *ctx, struct coap_resource_t *resource, 
              const coap_endpoint_t *local_interface, coap_address_t *peer, 
              coap_pdu_t *request, str *token, coap_pdu_t *response);

static void hnd_get_name(coap_context_t *ctx, struct coap_resource_t *resource, 
              const coap_endpoint_t *local_interface, coap_address_t *peer, 
              coap_pdu_t *request, str *token, coap_pdu_t *response);

static void hnd_put_name(coap_context_t *ctx UNUSED_PARAM,
             struct coap_resource_t *resource UNUSED_PARAM,
             const coap_endpoint_t *local_interface UNUSED_PARAM,
             coap_address_t *peer UNUSED_PARAM,
             coap_pdu_t *request,
             str *token UNUSED_PARAM,
             coap_pdu_t *response);
             
static void hnd_delete_name(coap_context_t *ctx ,
                struct coap_resource_t *resource ,
                const coap_endpoint_t *local_interface ,
                coap_address_t *peer ,
                coap_pdu_t *request ,
                str *token ,
                coap_pdu_t *response ); 
                
static void hnd_post_name(coap_context_t *ctx UNUSED_PARAM,
                struct coap_resource_t *resource UNUSED_PARAM,
                const coap_endpoint_t *local_interface UNUSED_PARAM,
                coap_address_t *peer UNUSED_PARAM,
                coap_pdu_t *request UNUSED_PARAM,
                str *token UNUSED_PARAM,
                coap_pdu_t *response UNUSED_PARAM);
             
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
	coap_resource_t* resource;
	coap_resource_t* observable_resource;
	coap_resource_t* broker_resource;
	fd_set           readfds;    
	struct timeval tv, *timeout;
    int result;
    coap_tick_t now;
    coap_queue_t *nextpdu;
	
	/* Prepare the CoAP server socket */ 
	coap_address_init(&serv_addr);
	serv_addr.addr.sin.sin_family      = AF_INET6;
	serv_addr.addr.sin.sin_addr.s_addr = INADDR_ANY;
	serv_addr.addr.sin.sin_port        = htons(5683); //default port
	ctx                                = coap_new_context(&serv_addr);
	if (!ctx) exit(EXIT_FAILURE);
	/* Prepare the CoAP server socket */ 
	
	
	/* Initialize the hello resource */
	resource = coap_resource_init((unsigned char *)"hello", strlen("hello"), 0);
	coap_register_handler(resource, COAP_REQUEST_GET, hello_handler);
	coap_add_resource(ctx, resource);
	/* Initialize the hello resource */
	
	
	/* Initialize the observable resource */
	observable_resource = coap_resource_init((unsigned char *)"name", strlen("name"), 0);
	coap_register_handler(observable_resource, COAP_REQUEST_GET, hnd_get_name);
	coap_register_handler(observable_resource, COAP_REQUEST_PUT, hnd_put_name);
	coap_register_handler(observable_resource, COAP_REQUEST_DELETE, hnd_delete_name);
	coap_register_handler(observable_resource, COAP_REQUEST_POST, hnd_post_name);
	observable_resource->observable = 1;
	coap_add_resource(ctx, observable_resource);	
	addToDatabase(db, "name", "Akbar");
	/* Initialize the observable resource */	
	
	/* Initialize the observable resource */
	broker_resource = coap_resource_init((unsigned char *)"broker", strlen("broker"), 0);
	coap_register_handler(broker_resource, COAP_REQUEST_GET, hnd_get_broker);
	coap_register_handler(broker_resource, COAP_REQUEST_POST, hnd_post_broker);
	broker_resource->observable = 1;
	coap_add_resource(ctx, broker_resource);	
	addToDatabase(db, "broker", "Aldwin");
	/* Initialize the observable resource */	
	
	
	
	/*Listen for incoming connections*/	
	while (1) {
		FD_ZERO(&readfds);
		FD_SET( ctx->sockfd, &readfds );

		nextpdu = coap_peek_next( ctx );

		coap_ticks(&now);
		while ( nextpdu && nextpdu->t <= now ) {
		  coap_retransmit( ctx, coap_pop_next( ctx ) );
		  nextpdu = coap_peek_next( ctx );
		}

		if ( nextpdu && nextpdu->t <= now + COAP_RESOURCE_CHECK_TIME ) {
		  /* set timeout if there is a pdu to send before our automatic
			 timeout occurs */
		  tv.tv_usec = ((nextpdu->t - now) % COAP_TICKS_PER_SECOND) * 1000000 / COAP_TICKS_PER_SECOND;
		  tv.tv_sec = (nextpdu->t - now) / COAP_TICKS_PER_SECOND;
		  timeout = &tv;
		} else {
		  tv.tv_usec = 0;
		  tv.tv_sec = COAP_RESOURCE_CHECK_TIME;
		  timeout = &tv;
		}
		result = select( FD_SETSIZE, &readfds, 0, 0, timeout );

		if ( result < 0 ) {     /* error */
		  if (errno != EINTR)
			perror("select");
		  } else if ( result > 0 ) {  /* read from socket */
			if ( FD_ISSET( ctx->sockfd, &readfds ) ) {
			  coap_read( ctx ); /* read received data */
			  /* coap_dispatch( ctx );  /\* and dispatch PDUs from receivequeue *\/ */
			}
		  } else {            /* timeout */
			/* coap_check_resource_list( ctx ); */
		}
     coap_check_notify(ctx);
    }
    
    coap_free_context(ctx);  
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
	const char* response_data     = "Hello World!";
	response->hdr->code           = COAP_RESPONSE_CODE(205);
	coap_add_option(response, COAP_OPTION_CONTENT_TYPE, coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);
	coap_add_data  (response, strlen(response_data), (unsigned char *)response_data);
}

static void
hnd_get_name(coap_context_t *ctx, struct coap_resource_t *resource, 
              const coap_endpoint_t *local_interface, coap_address_t *peer, 
              coap_pdu_t *request, str *token, coap_pdu_t *response) 
{
	unsigned char buf[3];
	int resource_available = (!checkString(getDatabaseValue(db,"name"), " "));
	
	const char* response_data     = getDatabaseValue(db,"name");
	response->hdr->code           = resource_available ? COAP_RESPONSE_CODE(205) : COAP_RESPONSE_CODE(404);
	
	if (coap_find_observer(resource, peer, token)) {
    /* FIXME: need to check for resource->dirty? */
		coap_add_option(response, COAP_OPTION_OBSERVE, coap_encode_var_bytes(buf, ctx->observe), buf);
	} 
	
	if (resource_available){	
		coap_add_option(response, COAP_OPTION_CONTENT_TYPE, coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);
		coap_add_data  (response, strlen(response_data), (unsigned char *)response_data);
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
  setDatabaseValue(db, "name", data);
}

static void
hnd_delete_name(coap_context_t *ctx ,
                struct coap_resource_t *resource ,
                const coap_endpoint_t *local_interface ,
                coap_address_t *peer ,
                coap_pdu_t *request ,
                str *token ,
                coap_pdu_t *response ) {

removeFromDatabase(db, "name");

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
addToDatabase(db, "name", "Akbar");
}

static void
hnd_get_broker(coap_context_t *ctx, struct coap_resource_t *resource, 
              const coap_endpoint_t *local_interface, coap_address_t *peer, 
              coap_pdu_t *request, str *token, coap_pdu_t *response) 
{
	unsigned char buf[3];
	int resource_available = (!checkString(getDatabaseValue(db,"broker"), " "));
	
	const char* response_data     = getDatabaseValue(db,"broker");
	response->hdr->code           = resource_available ? COAP_RESPONSE_CODE(201) : COAP_RESPONSE_CODE(404);
		
	if (coap_find_observer(resource, peer, token)) {
    /* FIXME: need to check for resource->dirty? */
		coap_add_option(response, COAP_OPTION_OBSERVE, coap_encode_var_bytes(buf, ctx->observe), buf);
	} 
	
	if (resource_available){	
		// option order matters!
		coap_add_option(response, COAP_OPTION_CONTENT_TYPE, coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);
		coap_add_option(response, COAP_OPTION_MAXAGE,coap_encode_var_bytes(buf, 20), buf); // mac-age in seconds, so 30 seconds (to mars. #pun)
		coap_add_data  (response, strlen(response_data), (unsigned char *)response_data);
	}
		
}

static void
hnd_post_broker(coap_context_t *ctx, struct coap_resource_t *resource, 
              const coap_endpoint_t *local_interface, coap_address_t *peer, 
              coap_pdu_t *request, str *token, coap_pdu_t *response) 
{
	//run "coap-client -t 20 -O 14,110 -m post coap://[::1]/broker" this to test this part of the code
	/* to get max_age value and ct */
	coap_opt_t *option;
	coap_opt_iterator_t opt_iter; 
	coap_option_iterator_init(request, &opt_iter, COAP_OPT_ALL);
	unsigned char* max_age_val;
	unsigned short max_age_len;
	unsigned char* ct_opt_val;
	unsigned short ct_opt_len;
	
	while ((option = coap_option_next(&opt_iter))) {
		// changing COAP_OPTION_MAXAGE into any coap Option will return the selected option type
	   if (opt_iter.type == COAP_OPTION_MAXAGE) { 
				max_age_val = coap_opt_value(option);
				max_age_len = coap_opt_length(option);
				//break;
	   }
	   
	   if (opt_iter.type == COAP_OPTION_CONTENT_TYPE) { 
				ct_opt_val = coap_opt_value(option);
				ct_opt_len = coap_opt_length(option);
				//break;
	   }
	}
	/* to get max_age value and ct*/
	response->hdr->code = COAP_RESPONSE_CODE(204);
	
	// setting Location Path is as easy as this. not sure if this is correcct though
	coap_add_option(response, COAP_OPTION_LOCATION_PATH, strlen("ps/broker"), "ps/broker");
	
	unsigned char result[32];
	// decode ct below
	sprintf(result, "ct:%d ma:%d", coap_decode_var_bytes(ct_opt_val, ct_opt_len), coap_decode_var_bytes(max_age_val,max_age_len));
	sprintf(result, "ct:%d ma:%s", coap_decode_var_bytes(ct_opt_val, ct_opt_len), max_age_val);
	coap_add_data  (response, strlen(result), (unsigned char *)result);
	setDatabaseValue(db, "broker", result);
}
