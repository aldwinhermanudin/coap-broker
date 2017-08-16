#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include "coap.h"
#include "sensorNode.h"

int queryValToInt(char *query, unsigned int len) {
	unsigned int i;
	int result;
	
	for (i = 0; i < len; i++) {
		if (query[i] >= 48 && query[i] <= 57) {
			break;
		}
	}
	result = query[i++] - '0';
	for (; i < len; i++) {
		if (query[i] < '0' || query[i] > '9')
			break;
		result = result * 10 + (query[i] - '0');
	}
	return result;
} 

int queryToInt(const coap_pdu_t *pdu) {
	coap_opt_iterator_t opt_iter;
    coap_opt_t *option;
    
	coap_option_iterator_init((coap_pdu_t *)pdu, &opt_iter, COAP_OPT_ALL);
	
	while ((option = coap_option_next(&opt_iter))) {
       switch (opt_iter.type) {
			case COAP_OPTION_URI_QUERY:
				return queryValToInt(coap_opt_value(option), 
					strlen(coap_opt_value(option)));
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
adc_handler(coap_context_t *ctx,
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
    sprintf(index, "%d", val);
    response->hdr->code = COAP_RESPONSE_CODE(205); // Why 205?
    
    coap_add_option(response,
                  COAP_OPTION_CONTENT_TYPE,
                  coap_encode_var_bytes(buf, COAP_MEDIATYPE_APPLICATION_JSON), buf);
    coap_add_option(response,
                  COAP_OPTION_MAXAGE,
                  coap_encode_var_bytes(buf, 30), buf);
    
    coap_add_data(response, strlen(index), (unsigned char *)index);
}

static void
flowmeter_handler(coap_context_t *ctx,
              struct coap_resource_t *resource,
              const coap_endpoint_t *local_interface,
              coap_address_t *peer,
              coap_pdu_t *request,
              str *token,
              coap_pdu_t *response) {

    char index[5];
    unsigned short val;
    unsigned char buf[3];
    
    val = getFlowmeter();
    sprintf(index, "%d", val);
    response->hdr->code = COAP_RESPONSE_CODE(205); // Why 205?
    
    coap_add_option(response,
                  COAP_OPTION_CONTENT_TYPE,
                  coap_encode_var_bytes(buf, COAP_MEDIATYPE_APPLICATION_JSON), buf);
    coap_add_option(response,
                  COAP_OPTION_MAXAGE,
                  coap_encode_var_bytes(buf, 30), buf);
    
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
    sprintf(index, "%d", val);
    response->hdr->code = COAP_RESPONSE_CODE(205); // Why 205?
    
    coap_add_option(response,
                  COAP_OPTION_CONTENT_TYPE,
                  coap_encode_var_bytes(buf, COAP_MEDIATYPE_APPLICATION_JSON), buf);
    coap_add_option(response,
                  COAP_OPTION_MAXAGE,
                  coap_encode_var_bytes(buf, 30), buf);
    
    coap_add_data(response, strlen(index), (unsigned char *)index);
}

static void
valve_handler(coap_context_t *ctx,
              struct coap_resource_t *resource,
              const coap_endpoint_t *local_interface,
              coap_address_t *peer,
              coap_pdu_t *request,
              str *token,
              coap_pdu_t *response) {

    char index[7];
    unsigned int val;
    unsigned char buf[3];
    
    val = queryToInt(request);
    turnGPIO(3, val);
    
    sprintf(index, "%d", val);
    response->hdr->code = COAP_RESPONSE_CODE(205); // Why 205?
    
    coap_add_option(response,
                  COAP_OPTION_CONTENT_TYPE,
                  coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);
    coap_add_option(response,
                  COAP_OPTION_MAXAGE,
                  coap_encode_var_bytes(buf, 30), buf);
    
    coap_add_data(response, strlen(index), (unsigned char *)index);
}

static void
actuator_gpio_handler(coap_context_t *ctx,
              struct coap_resource_t *resource,
              const coap_endpoint_t *local_interface,
              coap_address_t *peer,
              coap_pdu_t *request,
              str *token,
              coap_pdu_t *response) {

    char index[7];
    unsigned int val;
    unsigned char buf[3];
    
    val = queryToInt(request);
    turnGPIO(3, val);
    
    //printf("value of query: %d\n", queryValue(request));
    //strcpy(index, "sukses");
    sprintf(index, "%d", val);
    response->hdr->code = COAP_RESPONSE_CODE(205); // Why 205?
    
    coap_add_option(response,
                  COAP_OPTION_CONTENT_TYPE,
                  coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);
    coap_add_option(response,
                  COAP_OPTION_MAXAGE,
                  coap_encode_var_bytes(buf, 30), buf);
    
    coap_add_data(response, strlen(index), (unsigned char *)index);
    //printf("token: %s\n", token->s);
    //printf("pdu request data: %s\n", request->data);
    //printf("resource uri: %s\n", resource->uri.s);
    //printf("resource link attr value: %d\n", resource->link);
    //printf("ambilOprion:\n");
    //ambilOption(request);
}

static void
actuator_pwm_handler(coap_context_t *ctx,
              struct coap_resource_t *resource,
              const coap_endpoint_t *local_interface,
              coap_address_t *peer,
              coap_pdu_t *request,
              str *token,
              coap_pdu_t *response) {

    char index[7];
    unsigned int val;
    unsigned char buf[5];
    
    val = queryToInt(request);
    setPWM(9, val);
    
    //printf("value of query: %d\n", queryValue(request));
    sprintf(index, "%d", val);
    response->hdr->code = COAP_RESPONSE_CODE(205); // Why 205?
    
    coap_add_option(response,
                  COAP_OPTION_CONTENT_TYPE,
                  coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);
    coap_add_option(response,
                  COAP_OPTION_MAXAGE,
                  coap_encode_var_bytes(buf, 30), buf);
    
    coap_add_data(response, strlen(index), (unsigned char *)index);
    //printf("token: %s\n", token->s);
    //printf("pdu request data: %s\n", request->data);
    //printf("resource uri: %s\n", resource->uri.s);
    //printf("resource link attr value: %d\n", resource->link);
    //printf("ambilOprion:\n");
    //ambilOption(request);
}

int main(int argc, char* argv[]){
    coap_context_t  *ctx; // The CoAP stack's global state
    coap_address_t serv_addr; // Multi-purpose address abstraction
    coap_resource_t *r; // buat diisi sama coap_resource_init
    fd_set readfds; // TODO: cari ini apa
    
    printf("tunggu...\n");
    initSerial("/dev/ttyUSB0", B9600);
    printf("selesai init uart\n");
    /* Prepare the CoAP server socket */ 
    coap_address_init(&serv_addr);
    serv_addr.addr.sin.sin_family = AF_INET6;
    serv_addr.addr.sin.sin_addr.s_addr = INADDR_ANY;
    serv_addr.addr.sin.sin_port = htons(5683); //This is the port
    ctx = coap_new_context(&serv_addr);
    if (!ctx)
        exit(EXIT_FAILURE);
    
	r = coap_resource_init(NULL, 0, 0);
    coap_register_handler(r, COAP_REQUEST_GET, index_handler);
    coap_add_attr(r, (unsigned char *)"ct", 2, (unsigned char *)"0", 1, 0);
	coap_add_attr(r, (unsigned char *)"title", 5, (unsigned char *)"\"General Info\"", 14, 0);
    coap_add_resource(ctx, r);
    
    r = coap_resource_init((unsigned char *)"adc", strlen("adc"), 0);
    coap_add_attr(r, (unsigned char *)"ct", 2, (unsigned char *)"50", strlen("50"), 0); //50: json
	coap_add_attr(r, (unsigned char *)"title", 5, (unsigned char *)"\"Sensor ADC, for testing only\"", strlen("\"Sensor ADC, for testing only\""), 0);
	coap_add_attr(r, (unsigned char *)"rt", 2, (unsigned char *)"\"sensor-adc\"", strlen("\"sensor-adc\""), 0); //50: json
    coap_register_handler(r, COAP_REQUEST_GET, adc_handler);
    coap_add_resource(ctx, r);
    
    r = coap_resource_init((unsigned char *)"flowmeter", strlen("flowmeter"), 0);
    coap_add_attr(r, (unsigned char *)"ct", 2, (unsigned char *)"50", strlen("50"), 0); //50: json
	coap_add_attr(r, (unsigned char *)"title", 5, (unsigned char *)"\"Flowmeter Sensor\"", strlen("\"Flowmeter Sensor\""), 0);
	coap_add_attr(r, (unsigned char *)"rt", 2, (unsigned char *)"\"sensor-flowmeter\"", strlen("\"sensor-flowmeter\""), 0); //50: json
    coap_register_handler(r, COAP_REQUEST_GET, flowmeter_handler);
    coap_add_resource(ctx, r);
    
    r = coap_resource_init((unsigned char *)"gpio", strlen("gpio"), 0);
    coap_add_attr(r, (unsigned char *)"ct", 2, (unsigned char *)"0", strlen("0"), 0); 
	coap_add_attr(r, (unsigned char *)"title", 5, (unsigned char *)"\"Set GPIO\"", strlen("\"Set GPIO\""), 0);
    coap_register_handler(r, COAP_REQUEST_GET, actuator_gpio_handler);
    coap_add_resource(ctx, r);
    
    r = coap_resource_init((unsigned char *)"valve", strlen("valve"), 0);
    coap_add_attr(r, (unsigned char *)"ct", 2, (unsigned char *)"0", strlen("0"), 0); 
	coap_add_attr(r, (unsigned char *)"title", 5, (unsigned char *)"\"Set valve\"", strlen("\"Set valve\""), 0);
    coap_register_handler(r, COAP_REQUEST_GET, valve_handler);
    coap_add_resource(ctx, r);
    
    r = coap_resource_init((unsigned char *)"pwm", strlen("pwm"), 0);
    coap_add_attr(r, (unsigned char *)"ct", 2, (unsigned char *)"0", strlen("0"), 0); //50: json
	coap_add_attr(r, (unsigned char *)"title", 5, (unsigned char *)"\"Aktuator PWM\"", strlen("\"Aktuator PWM\""), 0);
    coap_register_handler(r, COAP_REQUEST_GET, actuator_pwm_handler);
    coap_add_resource(ctx, r);
    
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
