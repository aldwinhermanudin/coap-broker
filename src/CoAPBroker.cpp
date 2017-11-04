#include "CoAPBroker.hpp"

coap_context_t**  	CoAPBroker::global_ctx;
TopicDB     CoAPBroker::topic_db;
MQTTClient* CoAPBroker::global_client;
bool         CoAPBroker::mqtt_bridge = false;
time_t      CoAPBroker::earliest_topic_max_age = LONG_MAX;
time_t		CoAPBroker::earliest_data_max_age = LONG_MAX;
volatile    MQTTClient_deliveryToken CoAPBroker::deliveredtoken;

CoAPBroker::CoAPBroker(EString name, CoAPServer &server) : CoAPResource(name){
	global_ctx = &(server.get_context());
	register_handler(COAP_REQUEST_GET, hnd_get_broker);
	register_handler(COAP_REQUEST_POST, hnd_post_broker);
	add_attribute(EString("ct"), EString("40"));
	add_attribute(EString("rt"), EString("\"core.ps\""));
}

int	CoAPBroker::compareString(char* a, char* b){
	if (a == NULL || b == NULL) return 0;
	if (strcmp(a,b) == 0)	return 1;
	else return 0;
}

void
CoAPBroker::hnd_get_broker(coap_context_t *ctx, struct coap_resource_t *resource, 
              const coap_endpoint_t *local_interface, coap_address_t *peer, 
              coap_pdu_t *request, str *token, coap_pdu_t *response) 
{	
	unsigned char 				buf[3];
	int 						requested_query = 0;
	int 						requested_link_format_counter = 0;
	char* 						requested_link_format_data = NULL;
	coap_opt_t 					*option;
	coap_opt_iterator_t			 counter_opt_iter; 
	coap_option_iterator_init	(request, &counter_opt_iter, COAP_OPT_ALL); 
	
	while ((option = coap_option_next(&counter_opt_iter))) {
		if (counter_opt_iter.type == COAP_OPTION_URI_QUERY) {  
			requested_query++;
		}
	}	
	debug("Total Query : %d \n",requested_query);	
	
	RESOURCES_ITER(ctx->resources, r) {
		if((strlen((char*)(r->uri.s)) > 3)){
			if(r->uri.s[0] == 'p' && r->uri.s[1] == 's' && r->uri.s[2] == '/'){
				
				int 						found_query = 0;	
				coap_opt_iterator_t 		value_opt_iter; 
				coap_option_iterator_init	(request, &value_opt_iter, COAP_OPT_ALL);
				while ((option = coap_option_next(&value_opt_iter))) {
				   if (value_opt_iter.type == COAP_OPTION_URI_QUERY) {
					   char	*query_name,*query_value;
					   int 	status = parseOptionURIQuery((char*)coap_opt_value(option), coap_opt_length(option), &query_name, &query_value);
					   if (status == 1){
							coap_attr_t* temp_attr = coap_find_attr(r, (unsigned char*)query_name, strlen((char*)query_name));
							if(temp_attr != NULL){
								if(compareString((char*)temp_attr->value.s, (char*)query_value)){
									debug("%s attribute Match with value of %s in %s\n",query_name, query_value, r->uri.s);
									found_query++;
								}
								else{
									debug("%s attribute Found but not Match in %s\n",query_name, r->uri.s);
								}
							}
							else{
								debug("%s Attribute Not Found in %s\n",query_name, r->uri.s);
							}
							free(query_name);
							free(query_value);
					   }
					   else{
							debug("Malformed Request\n");
							free					(query_name);
							free					(query_value);
							free					(requested_link_format_data);
							response->hdr->code 	= COAP_RESPONSE_CODE(400);
							coap_add_data			(response, strlen(coap_response_phrase(response->hdr->code)),(unsigned char *)coap_response_phrase(response->hdr->code));
							return; 
						}
					}
				}
				if(requested_query == found_query){
					/* every matching resource will be concat to the master string here */ 
					size_t 							response_size = calculateResourceLF(r);
					size_t 							response_offset = 0;
					char 							response_data[response_size+1]; 
					response_data[response_size] 	= '\0';
					coap_print_link					(r, (unsigned char*) response_data, &response_size, &response_offset);
					debug							("Resource in Link Format : %s\n", response_data);
					debug							("Link Format size : %ld\n", strlen(response_data));
					debug							("Found Matching Resource with requested URI Query : %s\n", r->uri.s);
					requested_link_format_counter++;
					
					if(requested_link_format_counter == 1){
						dynamicConcatenate(&requested_link_format_data, response_data);
					}
					else {
                        char temp = ',';
						dynamicConcatenate(&requested_link_format_data, &temp);
						dynamicConcatenate(&requested_link_format_data, response_data);
					}
				}				
			}
		}
	}
	if (requested_link_format_counter > 0){

		debug("Requested Link Format Data 		: %s\n", requested_link_format_data);
		debug("Total Printed Link Format Size 	: %ld\n", strlen(requested_link_format_data));
		debug("Total Requested Resource  		: %ld\n", requested_link_format_counter);
	
		coap_block_t 			block;
		coap_add_option			(response, COAP_OPTION_CONTENT_TYPE, coap_encode_var_bytes(buf, COAP_MEDIATYPE_APPLICATION_LINK_FORMAT), buf);
		  
		if (request) { 
			if (coap_get_block(request, COAP_OPTION_BLOCK2, &block)) {
				int res = coap_write_block_opt(&block, COAP_OPTION_BLOCK2, response, strlen(requested_link_format_data));

				switch (res) {
					
					case -2:			
					free(requested_link_format_data); 
					response->hdr->code = COAP_RESPONSE_CODE(400);
					coap_add_data(response, strlen(coap_response_phrase(response->hdr->code)),(unsigned char *)coap_response_phrase(response->hdr->code));
					return;
					
					case -1:			 
					assert(0);
					 
					case -3:		
					free(requested_link_format_data);	 
					response->hdr->code = COAP_RESPONSE_CODE(500);
					coap_add_data(response, strlen(coap_response_phrase(response->hdr->code)),(unsigned char *)coap_response_phrase(response->hdr->code));
					return;
					
					default:			 
					;
				}			  
				coap_add_block(response, strlen(requested_link_format_data), (unsigned char*) requested_link_format_data, block.num, block.szx);
			} 
			
			else {
				if (!coap_add_data(response, strlen(requested_link_format_data), (unsigned char*) requested_link_format_data)) { 
					block.szx = 6;
					coap_write_block_opt(&block, COAP_OPTION_BLOCK2, response,strlen(requested_link_format_data));				
					coap_add_block(response, strlen(requested_link_format_data), (unsigned char *) requested_link_format_data,block.num, block.szx);	
				}
			}    
		}
		
		response->hdr->code 	= COAP_RESPONSE_CODE(205);		
		free(requested_link_format_data);
		return;
	}
	
	else {
		free(requested_link_format_data);
		response->hdr->code 		  = COAP_RESPONSE_CODE(404);
		coap_add_data(response, strlen(coap_response_phrase(response->hdr->code)),(unsigned char *)coap_response_phrase(response->hdr->code));
		return; 
	} 
}


void
CoAPBroker::hnd_post_broker(coap_context_t *ctx, struct coap_resource_t *resource, 
              const coap_endpoint_t *local_interface, coap_address_t *peer, 
              coap_pdu_t *request, str *token, coap_pdu_t *response) 
{

	coap_resource_t *new_resource = NULL;
		
	/* declare a safe variable for data */
	size_t size;
    unsigned char *data;
    unsigned char *data_safe;
	(void)coap_get_data(request, &size, &data);
	data_safe = (unsigned char*) coap_malloc(sizeof(char)*(size+2));
	snprintf((char*)data_safe,size+1, "%s", data);
	/* declare a safe variable for data */
	
	/* parse payload */
	int status = parseLinkFormat((char*)data_safe,resource, &new_resource);
	debug("Parser status : %d\n", status);
	/* parse payload */
	
	/* free the safe variable for data */
	coap_free(data_safe);
	/* free the safe variable for data */
	
	/* Iterator to get max_age value */
	time_t opt_topic_ma = 0;
	time_t abs_topic_ma = 0;
	int ma_opt_status = 0;

	int ct_opt_status = 0;	
	int ct_opt_val_integer = -1;

	coap_opt_t *option;
	coap_opt_iterator_t opt_iter; 
	coap_option_iterator_init(request, &opt_iter, COAP_OPT_ALL);
	while ((option = coap_option_next(&opt_iter))) {
		
		if (opt_iter.type == COAP_OPTION_CONTENT_TYPE && !ct_opt_status) { // !ct_opt_status means only take the first occurence of that option
			ct_opt_status = 1;
			ct_opt_val_integer =  coap_decode_var_bytes(coap_opt_value(option), coap_opt_length(option)); 
		}
		/* search for Max-Age Option field */
	   if (opt_iter.type == COAP_OPTION_MAXAGE && !ma_opt_status) {
			ma_opt_status = 1;  
			/* decode Max-Age Option */
			opt_topic_ma = coap_decode_var_bytes(coap_opt_value(option), coap_opt_length(option)); 
				
			/* if max-age must have a value of 1 or above 
			 * if below 1 set topic max-age to 0 (infinite max-age )
			 * else will set topic max-age to ( decode max-age + current time ) */
			if(opt_topic_ma < 1) {  
				abs_topic_ma = 0;
			}
			else{
				abs_topic_ma = time(NULL) + opt_topic_ma;
			}
	   }
	   if (ct_opt_status && ma_opt_status) { break;}
	}	
	debug("topic max-age : %ld\n",opt_topic_ma);
	debug("topic abs max-age : %ld\n",abs_topic_ma);
	/* Iterator to get max_age value */
	
	if (ct_opt_val_integer != COAP_MEDIATYPE_APPLICATION_LINK_FORMAT){
		debug("ct option is not link format\n"); 
		coapFreeResource(new_resource);
		status=0; /* jump to malformed request handler */
	} 
	
	/* Unsupported content format for topic. */
	if (status)	{
		/* search for ct attribute in the new_resource created by parseLinkFormat*/
		coap_attr_t* new_resource_attr = coap_find_attr(new_resource,(const unsigned char*) "ct", 2);
		
		/* if new_resource doesn't have ct attribute, jump to malformed request handler and free new_resource */
		if(new_resource_attr == NULL){
			debug("ct attribute not found\n"); 
			coapFreeResource(new_resource);
			status=0; /* jump to malformed request handler */
		}
		
		/* if new_resource does have ct attribute, check ct attribute validity. jump to 
		 * "Unsupported content format for topic" handler if ct isn't valid */
		else {
			int is_digit = 1;
			int ct_value_valid = 1;
			
			/* check ct value, by using isdigit() and iterate to every char in new_resource ct attribute */
			for(int i = 0; i < new_resource_attr->value.length;i++){
				if (!isdigit(new_resource_attr->value.s[i])){
					is_digit = 0;
					break;
				}
			}
			
			if(is_digit){
				int ct_value = atoi((char*)new_resource_attr->value.s);
				if(ct_value < 0 || ct_value > 65535){ 
					ct_value_valid = 0;	/* jump to "Unsupported content format for topic" handler */
				}
			}
			else { 
				ct_value_valid = 0;	/* jump to "Unsupported content format for topic" handler */
			}	
			
			/* "Unsupported content format for topic" handler */
			if ( !ct_value_valid ){
				coapFreeResource(new_resource); 
				response->hdr->code = COAP_RESPONSE_CODE(406);
				coap_add_data(response, strlen(coap_response_phrase(response->hdr->code)),(unsigned char *)coap_response_phrase(response->hdr->code));
				return;
			}			
		}	
	}	
	/* Unsupported content format for topic. */
	
	/* Topic already exists. */ 
	
	/* Iterate to every resource in coap ctx and compare 
	 * iterated resource uri to new-resource. Jump to 
	 * "Topic already exists" handler if both resource have the same uri */
	if (status){
		int found_resource = 0;
		RESOURCES_ITER(ctx->resources, r) {
			if(compareString((char*)r->uri.s,(char*) new_resource->uri.s)){
				found_resource = 1; /* Jump to "Topic already exists" handler if both resource have the same uri */
				break;
			}
		}
		
		/* "Topic already exists" handler */
		if(found_resource){
			if (abs_topic_ma < earliest_topic_max_age && abs_topic_ma != 0){
					earliest_topic_max_age = abs_topic_ma;
			}
			
			topic_db.get_topic(std::string((char*)new_resource->uri.s))->update_topic(abs_topic_ma);
			coapFreeResource(new_resource);
			response->hdr->code = COAP_RESPONSE_CODE(403); 
			coap_add_data(response, strlen(coap_response_phrase(response->hdr->code)),(unsigned char *)coap_response_phrase(response->hdr->code));
			return ;
		}
	}
	/* Topic already exists. */
	
	/* Successful Creation of the topic */
	if (status){
			if (abs_topic_ma < earliest_topic_max_age && abs_topic_ma != 0){
					earliest_topic_max_age = abs_topic_ma;
			}
			if(mqtt_bridge){
				MQTTClient_subscribe(*global_client, (char*)new_resource->uri.s, QOS);
			}
			coap_register_handler(new_resource, COAP_REQUEST_GET, hnd_get_topic);
			coap_register_handler(new_resource, COAP_REQUEST_POST, hnd_post_topic);
			coap_register_handler(new_resource, COAP_REQUEST_PUT, hnd_put_topic);
			coap_register_handler(new_resource, COAP_REQUEST_DELETE, hnd_delete_topic);
			new_resource->observable = 1;
			coap_add_resource(ctx, new_resource);
			coap_add_option(response, COAP_OPTION_LOCATION_PATH, new_resource->uri.length, new_resource->uri.s);
            topic_db.add_topic(std::string((char*) new_resource->uri.s, new_resource->uri.length), abs_topic_ma);
			response->hdr->code = COAP_RESPONSE_CODE(201) ;
			coap_add_data(response, strlen(coap_response_phrase(response->hdr->code)),(unsigned char *)coap_response_phrase(response->hdr->code));
			return ; 
	}
	/* Successful Creation of the topic */
	
	/* malformed request */
	/* don't need to free new_resource. Any error will be handle and freed in parseLinkFormat() */
	if (!status){
		response->hdr->code = COAP_RESPONSE_CODE(400);
		coap_add_data(response, strlen(coap_response_phrase(response->hdr->code)),(unsigned char *)coap_response_phrase(response->hdr->code));
		return ; 
	} 
	/* malformed request */
}

void CoAPBroker::hnd_get_topic(coap_context_t *ctx, struct coap_resource_t *resource, 
     const coap_endpoint_t *local_interface, coap_address_t *peer, 
     coap_pdu_t *request, str *token, coap_pdu_t *response){
           
	unsigned char buf[3];
	int status = 0; 
	int is_observe_notification_response = 0;
	int is_observe_registration_request = 0;
	int ct_attr_value = atoi((char*)(coap_find_attr(resource,(const unsigned char*) "ct", 2))->value.s);

	/* to get max_age value and observe*/
	unsigned int ct_opt_val_integer = -1;
	int ct_opt_status = 0;
	
	unsigned int obs_opt_val_integer = -1;
	int obs_opt_status = 0;

	if(request != NULL) { 
		debug("Request is NOT NULL \n");		
		coap_opt_t *option;
		coap_opt_iterator_t opt_iter; 
		coap_option_iterator_init(request, &opt_iter, COAP_OPT_ALL); 
					
		while ((option = coap_option_next(&opt_iter))) {
			if (opt_iter.type == COAP_OPTION_CONTENT_TYPE && !ct_opt_status) { // !ct_opt_status means only take the first occurence of that option
					ct_opt_status = 1;
					ct_opt_val_integer =  coap_decode_var_bytes(coap_opt_value(option), coap_opt_length(option)); 
			}
			if (opt_iter.type == COAP_OPTION_OBSERVE && !obs_opt_status ) { // !obs_opt_status means only take the first occurence of that option
					obs_opt_status = 1;
					obs_opt_val_integer =  coap_decode_var_bytes(coap_opt_value(option), coap_opt_length(option));
					debug("Observe GET value : %d\n", obs_opt_val_integer); 
			}
			if (ct_opt_status && obs_opt_status) { break;}
		}	
	}
	/* to get max_age value and observe*/

	if (coap_find_observer(resource, peer, token) && request == NULL){
		is_observe_notification_response = 1;
		debug("This is a Subscriber Notification Response \n");
	}
	else if (coap_find_observer(resource, peer, token) && request != NULL && obs_opt_val_integer == 0){
		is_observe_registration_request = 1;
		debug("This is a Subscriber Registration Request \n");
	}
	else{ 
		debug("This is a READ Request \n");
	}

	/* Unsupported Content Format */
	if ((ct_opt_status || (ct_opt_status && is_observe_registration_request)) && !is_observe_notification_response){
	
		if (ct_attr_value == ct_opt_val_integer){ 
			status = 1; 
		}		
		else{
			debug("requested ct : %d\n", ct_opt_val_integer);
			debug("available ct : %d\n", ct_attr_value);
			response->hdr->code 	= COAP_RESPONSE_CODE(415);
			coap_add_data(response, strlen(coap_response_phrase(response->hdr->code)),(unsigned char *)coap_response_phrase(response->hdr->code));
			return;
		}	
	}
	/* Unsupported Content Format */

	/* Bad Request */
	else if((!ct_opt_status || (!ct_opt_status && is_observe_registration_request)) && !is_observe_notification_response){
		response->hdr->code 	= COAP_RESPONSE_CODE(400);
		coap_add_data(response, strlen(coap_response_phrase(response->hdr->code)),(unsigned char *)coap_response_phrase(response->hdr->code));
		return;
	}
	/* Bad Request */

	TopicDataPtr temp = topic_db.copy_topic(std::string((char*) resource->uri.s));
	//TopicDataPtr temp = cloneTopic(&topicDB, resource->uri.s);
	/* It should never have this condition, ever. Just in Case. */
	/* Not Found */
	if (temp == nullptr ) {
		response->hdr->code 	= COAP_RESPONSE_CODE(404);
		coap_add_data(response, strlen(coap_response_phrase(response->hdr->code)),(unsigned char *)coap_response_phrase(response->hdr->code));
		return;
	}
	/* Not Found */

	if(status || is_observe_notification_response){
	
		/* No Content */
		if (temp == nullptr){
			if (coap_find_observer(resource, peer, token)) {
				coap_add_option(response, COAP_OPTION_OBSERVE, coap_encode_var_bytes(buf, ctx->observe), buf);
			}
			coap_add_option(response, COAP_OPTION_CONTENT_TYPE, coap_encode_var_bytes(buf, ct_attr_value), buf);
			response->hdr->code 	= COAP_RESPONSE_CODE(204);
			coap_add_data(response, strlen("No Content"),(unsigned char *)"No Content");
			//freeTopic(temp);  already using shared_ptr
			return;
		}
		/* No Content */
		
		/* No Content || Content */
		else {
			if (coap_find_observer(resource, peer, token)) {
				coap_add_option(response, COAP_OPTION_OBSERVE, coap_encode_var_bytes(buf, ctx->observe), buf);
			}		
			time_t remaining_maxage_time = temp->get_data_ma() - time(NULL);
			if (remaining_maxage_time < 0 && !(temp->get_data_ma() == 0)){
				coap_add_option(response, COAP_OPTION_CONTENT_TYPE, coap_encode_var_bytes(buf, ct_attr_value), buf);
				response->hdr->code 	= COAP_RESPONSE_CODE(204);
				coap_add_data(response, strlen("No Content"),(unsigned char *)"No Content");
				//freeTopic(temp);  already using shared_ptr
				return;
			}
			else{
				response->hdr->code 	= COAP_RESPONSE_CODE(205);
				coap_add_option(response, COAP_OPTION_CONTENT_TYPE, coap_encode_var_bytes(buf, ct_attr_value), buf);
				if ((!(temp->get_data_ma() == 0))){
					printf("Current time   : %ld\n",time(NULL));
					printf("Expired time   : %ld\n",temp->get_data_ma());
					printf("Remaining time : %ld\n",temp->get_data_ma() - time(NULL));
					coap_add_option(response, COAP_OPTION_MAXAGE,coap_encode_var_bytes(buf, remaining_maxage_time), buf);
				} 
				coap_add_data(response, strlen(temp->get_data().c_str()), (unsigned char*)temp->get_data().c_str());
				//freeTopic(temp);  already using shared_ptr
				return;
			}
		}
	/* No Content || Content */
	} 		 
}


void CoAPBroker::hnd_put_topic(coap_context_t *ctx ,
             struct coap_resource_t *resource ,
             const coap_endpoint_t *local_interface ,
             coap_address_t *peer ,
             coap_pdu_t *request,
             str *token ,
             coap_pdu_t *response){
				 
	size_t size;
    unsigned char *data; 
	(void)coap_get_data(request, &size, &data);
	int ma_opt_status = 0;	// I think ma_opt_status is un-necessary. Just in case. //
	int ct_opt_status = 0;
	int status = 0;
	
	/* to get max_age and ct value */
	coap_opt_t *option;
	coap_opt_iterator_t opt_iter; 
	coap_option_iterator_init(request, &opt_iter, COAP_OPT_ALL);  
	unsigned int ct_opt_val_integer = 0;
	unsigned int ma_opt_val_integer = 0;
	time_t		 ma_opt_val_time_t 	= 0;
		
	while ((option = coap_option_next(&opt_iter))) {
	   if (opt_iter.type == COAP_OPTION_MAXAGE && !ma_opt_status) { 
				ma_opt_status = 1;
				ma_opt_val_integer = coap_decode_var_bytes(coap_opt_value(option), coap_opt_length(option)); 
				ma_opt_val_time_t = ma_opt_val_integer + time(NULL); 
	   }
	   if (opt_iter.type == COAP_OPTION_CONTENT_TYPE && !ct_opt_status) { 
				ct_opt_status = 1;
				ct_opt_val_integer = coap_decode_var_bytes(coap_opt_value(option), coap_opt_length(option));  
	   }
	}
	/* to get max_age and ct value */
	
	/* to print max_age value*/
	debug("Option Max-age   : %d\n",ma_opt_val_integer);
	debug("Absolute Max-age : %ld\n",ma_opt_val_time_t);
	/* to print max_age value*/
	
	/* Unsupported Content Format */
	if (ct_opt_status && (ma_opt_val_integer >= 0)){ // make sure max-age is above 0 //
		coap_attr_t* ct_attr = coap_find_attr(resource,(const unsigned char*) "ct", 2);
		int ct_attr_value = atoi((char*) ct_attr->value.s);
		
		debug("requested ct : %d\n", ct_opt_val_integer);
		debug("available ct : %d\n", ct_attr_value);
		
		if (ct_attr_value != ct_opt_val_integer){ 
			response->hdr->code 	= COAP_RESPONSE_CODE(415);
			coap_add_data(response, strlen(coap_response_phrase(response->hdr->code)),(unsigned char *)coap_response_phrase(response->hdr->code));
			return;
		}	
	}
	/* Unsupported Content Format */
	
	/* Bad Request */
	else {
		response->hdr->code 	= COAP_RESPONSE_CODE(400);
		coap_add_data(response, strlen(coap_response_phrase(response->hdr->code)),(unsigned char *)coap_response_phrase(response->hdr->code));
		return;
	}
	/* Bad Request */
	
	TopicDataPtr temp = topic_db.copy_topic(std::string((char*)resource->uri.s));
	//TopicDataPtr temp = cloneTopic(&topicDB, resource->uri.s);
	/* Not Found */ // It should never have this condition, ever. Just in Case. //
	if (temp == NULL ) {
		response->hdr->code 	= COAP_RESPONSE_CODE(404);
		coap_add_data(response, strlen(coap_response_phrase(response->hdr->code)),(unsigned char *)coap_response_phrase(response->hdr->code));
		return;
	}
	/* Not Found */

	//if (updateTopicData(&topicDB, resource->uri.s, ma_opt_val_time_t, data, size)){
	if (topic_db.get_topic(std::string((char*)resource->uri.s))->update_topic(std::string((char*)data),ma_opt_val_time_t)){
		if (ma_opt_val_time_t < earliest_data_max_age && ma_opt_val_time_t != 0){
			earliest_data_max_age = ma_opt_val_time_t;
		}
		if(mqtt_bridge){		
			MQTTClient_message pubmsg = MQTTClient_message_initializer;
			MQTTClient_deliveryToken token;
			pubmsg.payload = data;
			pubmsg.payloadlen = size;
			pubmsg.qos = QOS;
			pubmsg.retained = 0;
			deliveredtoken = 0;
			MQTTClient_publishMessage(*global_client, (char*)resource->uri.s, &pubmsg, &token);
			debug("Waiting for publication of %s on topic %s for client with ClientID: %s\n", data, resource->uri.s, CLIENTID); // printing unsafe data //
		}
		resource->dirty = 1;
		coap_check_notify(ctx);
		response->hdr->code = COAP_RESPONSE_CODE(204);
		coap_add_data(response, strlen(coap_response_phrase(response->hdr->code)),(unsigned char *)coap_response_phrase(response->hdr->code)); 
		// freeTopic(temp); already use shared_ptr
		return;
	} 
}

               
void CoAPBroker::hnd_post_topic(coap_context_t *ctx ,
                struct coap_resource_t *resource ,
                const coap_endpoint_t *local_interface ,
                coap_address_t *peer ,
                coap_pdu_t *request ,
                str *token ,
                coap_pdu_t *response ){
	
	coap_resource_t *new_resource = NULL;
		
	/* declare a safe variable for data */
	size_t size;
    unsigned char *data;
    unsigned char *data_safe;
	(void)coap_get_data(request, &size, &data);
	data_safe = (unsigned char*) coap_malloc(sizeof(char)*(size+2));
	snprintf((char*)data_safe,size+1, "%s", (char*)data);
	/* declare a safe variable for data */
	
	/* parse payload */
	int status = parseLinkFormat((char*) data_safe,resource, &new_resource);
	debug("Parser status : %d\n", status);
	/* parse payload */
	
	/* free the safe variable for data */
	coap_free(data_safe);
	/* free the safe variable for data */
	
	/* Iterator to get max_age value */
	time_t opt_topic_ma = 0;
	time_t abs_topic_ma = 0;
	int ma_opt_status = 0;

	int ct_opt_status = 0;	
	int ct_opt_val_integer = -1;

	coap_opt_t *option;
	coap_opt_iterator_t opt_iter; 
	coap_option_iterator_init(request, &opt_iter, COAP_OPT_ALL);
	while ((option = coap_option_next(&opt_iter))) {
		
		if (opt_iter.type == COAP_OPTION_CONTENT_TYPE && !ct_opt_status) { // !ct_opt_status means only take the first occurence of that option
			ct_opt_status = 1;
			ct_opt_val_integer =  coap_decode_var_bytes(coap_opt_value(option), coap_opt_length(option)); 
		}
		/* search for Max-Age Option field */
	   if (opt_iter.type == COAP_OPTION_MAXAGE && !ma_opt_status) {
			ma_opt_status = 1;  
			/* decode Max-Age Option */
			opt_topic_ma = coap_decode_var_bytes(coap_opt_value(option), coap_opt_length(option)); 
				
			/* if max-age must have a value of 1 or above 
			 * if below 1 set topic max-age to 0 (infinite max-age )
			 * else will set topic max-age to ( decode max-age + current time ) */
			if(opt_topic_ma < 1) {  
				abs_topic_ma = 0;
			}
			else{
				abs_topic_ma = time(NULL) + opt_topic_ma;
			}
	   }
	   if (ct_opt_status && ma_opt_status) { break;}
	}	
	debug("topic max-age : %ld\n",opt_topic_ma);
	debug("topic abs max-age : %ld\n",abs_topic_ma);
	/* Iterator to get max_age value */
	
	if (ct_opt_val_integer != COAP_MEDIATYPE_APPLICATION_LINK_FORMAT){
		debug("ct option is not link format\n"); 
		coapFreeResource(new_resource);
		status=0; /* jump to malformed request handler */
	} 
	
	/* Unsupported content format for topic. */
	if (status)	{
		/* search for ct attribute in the new_resource created by parseLinkFormat*/
		coap_attr_t* new_resource_attr = coap_find_attr(new_resource,(const unsigned char*) "ct", 2);
		
		/* if new_resource doesn't have ct attribute, jump to malformed request handler and free new_resource */
		if(new_resource_attr == NULL){
			debug("ct attribute not found\n"); 
			coapFreeResource(new_resource);
			status=0; /* jump to malformed request handler */
		}
		
		/* if new_resource does have ct attribute, check ct attribute validity. jump to 
		 * "Unsupported content format for topic" handler if ct isn't valid */
		else {
			int is_digit = 1;
			int ct_value_valid = 1;
			
			/* check ct value, by using isdigit() and iterate to every char in new_resource ct attribute */
			for(int i = 0; i < new_resource_attr->value.length;i++){
				if (!isdigit(new_resource_attr->value.s[i])){
					is_digit = 0;
					break;
				}
			}
			
			if(is_digit){
				int ct_value = atoi((char*)new_resource_attr->value.s);
				if(ct_value < 0 || ct_value > 65535){ 
					ct_value_valid = 0;	/* jump to "Unsupported content format for topic" handler */
				}
			}
			else { 
				ct_value_valid = 0;	/* jump to "Unsupported content format for topic" handler */
			}	
			
			/* "Unsupported content format for topic" handler */
			if ( !ct_value_valid ){
				coapFreeResource(new_resource); 
				response->hdr->code = COAP_RESPONSE_CODE(406);
				coap_add_data(response, strlen(coap_response_phrase(response->hdr->code)),(unsigned char *)coap_response_phrase(response->hdr->code));
				return;
			}			
		}	
	}	
	/* Unsupported content format for topic. */
	
	/* Topic already exists. */ 
	
	/* Iterate to every resource in coap ctx and compare 
	 * iterated resource uri to new-resource. Jump to 
	 * "Topic already exists" handler if both resource have the same uri */
	if (status){
		int found_resource = 0;
		RESOURCES_ITER(ctx->resources, r) {
			if(compareString((char*)r->uri.s, (char*)new_resource->uri.s)){
				found_resource = 1; /* Jump to "Topic already exists" handler if both resource have the same uri */
				break;
			}
		}
		
		/* "Topic already exists" handler */
		if(found_resource){
			if (abs_topic_ma < earliest_topic_max_age && abs_topic_ma != 0){
					earliest_topic_max_age = abs_topic_ma;
			}
			
			//updateTopicInfo(&topicDB, new_resource->uri.s, abs_topic_ma);
			topic_db.get_topic(std::string((char*)new_resource->uri.s))->update_topic(abs_topic_ma);
			coapFreeResource(new_resource);
			response->hdr->code = COAP_RESPONSE_CODE(403); 
			coap_add_data(response, strlen(coap_response_phrase(response->hdr->code)),(unsigned char *)coap_response_phrase(response->hdr->code));
			return ;
		}
	}
	/* Topic already exists. */
	
	/* Successful Creation of the topic */
	if (status){
			if (abs_topic_ma < earliest_topic_max_age && abs_topic_ma != 0){
					earliest_topic_max_age = abs_topic_ma;
			}
			if(mqtt_bridge){
				MQTTClient_subscribe(*global_client, (char*)new_resource->uri.s, QOS);
			}
			coap_register_handler(new_resource, COAP_REQUEST_GET, hnd_get_topic);
			coap_register_handler(new_resource, COAP_REQUEST_POST, hnd_post_topic);
			coap_register_handler(new_resource, COAP_REQUEST_PUT, hnd_put_topic);
			coap_register_handler(new_resource, COAP_REQUEST_DELETE, hnd_delete_topic);
			new_resource->observable = 1;
			coap_add_resource(ctx, new_resource);
			coap_add_option(response, COAP_OPTION_LOCATION_PATH, new_resource->uri.length, new_resource->uri.s);
			topic_db.add_topic(std::string((char*)new_resource->uri.s, new_resource->uri.length), abs_topic_ma);
			//addTopicWEC(&topicDB, new_resource->uri.s, new_resource->uri.length, abs_topic_ma);
			response->hdr->code = COAP_RESPONSE_CODE(201) ;
			coap_add_data(response, strlen(coap_response_phrase(response->hdr->code)),(unsigned char *)coap_response_phrase(response->hdr->code));
			return ; 
	}
	/* Successful Creation of the topic */
	
	/* malformed request */
	/* don't need to free new_resource. Any error will be handle and freed in parseLinkFormat() */
	if (!status){
		response->hdr->code = COAP_RESPONSE_CODE(400);
		coap_add_data(response, strlen(coap_response_phrase(response->hdr->code)),(unsigned char *)coap_response_phrase(response->hdr->code));
		return ; 
	} 
	/* malformed request */
} 

void CoAPBroker::hnd_delete_topic(coap_context_t *ctx ,
                struct coap_resource_t *resource ,
                const coap_endpoint_t *local_interface ,
                coap_address_t *peer ,
                coap_pdu_t *request ,
                str *token ,
                coap_pdu_t *response ){
	int counter = 0;
	char* deleted_sub_uri = (char*) malloc(sizeof(char) *(resource->uri.length+3));
	snprintf(deleted_sub_uri, (resource->uri.length+1)+1, "%s/", resource->uri.s);
	
	counter = topic_db.delete_topic(std::string((char*)resource->uri.s));
	//counter = deleteTopic(&topicDB, resource->uri.s);
	
	if (counter){
		if(mqtt_bridge){
			MQTTClient_unsubscribe(*global_client,(char*) resource->uri.s);
		}
		RESOURCES_DELETE(ctx->resources, resource);
		coapFreeResource(resource);
	
		RESOURCES_ITER(ctx->resources, r) {
			if (strstr((char*)r->uri.s, deleted_sub_uri) != NULL){ 
				//if (deleteTopic(&topicDB, r->uri.s)){
				if (topic_db.delete_topic(std::string((char*)r->uri.s))){
					if(mqtt_bridge){
						MQTTClient_unsubscribe(*global_client, (char*)r->uri.s);
					}
					RESOURCES_DELETE(ctx->resources, r);
					coapFreeResource(r);	// modified coap_free_resource
					counter++;
				} 
			}
		}
	}
	if(counter){
		response->hdr->code = COAP_RESPONSE_CODE(202);
	}
	else {
		response->hdr->code = COAP_RESPONSE_CODE(404);
	}
	
	int counter_digit = 0;
	char* payload_data;
	int counter_temp = counter?counter : 1;
	while(counter_temp != 0)
    { 
        counter_temp /= 10;
        ++counter_digit;
    }    
	payload_data = (char*)malloc(sizeof(int) * (counter_digit+2));
	snprintf(payload_data,counter_digit+1,"%d", counter);
	
	unsigned char buf[3];
	coap_add_option(response, COAP_OPTION_CONTENT_TYPE, coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);
	coap_add_data  (response, strlen(payload_data), (unsigned char*)payload_data);	
		
	free(payload_data);
	free(deleted_sub_uri);
}



void CoAPBroker::topicDataMAMonitor(){
	
	TopicDataPtr currentPtr = topic_db.get_head();
	time_t master_time = time(NULL);
	/* if list is empty */
	if ( currentPtr == nullptr ) {
		debug( "List is empty.\n\n" );
	} /* end if */
	else if (earliest_data_max_age < master_time) {  
			/* while not the end of the list */
			
		time_t next_earliest_data_ma = LONG_MAX;
		while ( currentPtr != nullptr ) {
			currentPtr->node_r_lock(); 
			//topicNodeRLock(currentPtr);
			TopicDataPtr nextPtr = currentPtr->nextPtr;
			
			currentPtr->data_r_lock();
			//topicDataRLock(currentPtr);
			if(currentPtr->get_data_ma() != 0 && currentPtr->get_data_ma() != earliest_data_max_age && currentPtr->get_data_ma() < next_earliest_data_ma){
				next_earliest_data_ma = currentPtr->get_data_ma();
			}
			
			time_t currrent_time = time(NULL);
									
			debug( "%s\t\t%ld\t%ld | %s\n ",currentPtr->get_path(), currentPtr->get_data_ma(), currrent_time, currentPtr->get_data_ma() < currrent_time && currentPtr->get_data_ma() != 0? "Data Expired. Deleting..." : "Data Valid");
			
			if(currentPtr->get_data_ma() < currrent_time && currentPtr->get_data_ma() != 0){
				currentPtr->data_unlock();
				currentPtr->node_unlock();
				//topicDataUnlock(currentPtr);				
				//topicNodeUnlock(currentPtr);
				topic_db.delete_topic(std::string(currentPtr->get_path()));
				//deleteTopicData(&topicDB, currentPtr->path);
			}
			else {				
				currentPtr->data_unlock();
				currentPtr->node_unlock();
				//topicDataUnlock(currentPtr);				
				//topicNodeUnlock(currentPtr);
			}
			
			currentPtr = nextPtr;   
		} /* end while */ 
		earliest_data_max_age = next_earliest_data_ma;
		
	} /* end else */ 

	else{
		debug( "No Data Max-Age Timeout yet.\n" );
		if (earliest_data_max_age == LONG_MAX){
			debug( "All data has infinite max-age\n");
		}
		else {
			debug( "Remaining seconds to earliest data max-age %ld.\n", earliest_data_max_age - master_time );
		}
	}
}

void CoAPBroker::topicMAMonitor(){
	
	TopicDataPtr currentPtr = topic_db.get_head();
	time_t master_time = time(NULL);
	/* if list is empty */
	if ( currentPtr == nullptr ) {
		debug( "List is empty.\n\n" );
	} /* end if */
	else if (earliest_topic_max_age < master_time) {  
			/* while not the end of the list */
			
		time_t next_earliest_topic_ma = LONG_MAX;
		while ( currentPtr != nullptr ) { 
			currentPtr->node_r_lock();
			//topicNodeRLock(currentPtr);
			TopicDataPtr nextPtr = currentPtr->nextPtr;
			
			currentPtr->data_r_lock();
			//topicDataRLock(currentPtr);
			if(currentPtr->get_topic_ma() != 0 && currentPtr->get_topic_ma() != earliest_topic_max_age && currentPtr->get_topic_ma() < next_earliest_topic_ma){
				next_earliest_topic_ma = currentPtr->get_topic_ma();
			}
			
			time_t currrent_time = time(NULL);
			
			debug( "%s\t\t%ld\t%ld | %s\n ",currentPtr->get_path(), currentPtr->get_topic_ma(), currrent_time, currentPtr->get_topic_ma() < currrent_time && currentPtr->get_topic_ma() != 0? "Topic Expired. Deleting..." : "Topic Valid");
						
			if (currentPtr->get_topic_ma() < currrent_time && currentPtr->get_topic_ma() != 0){
				char* deleted_uri_topic = (char*) malloc(sizeof(char) * ((currentPtr->get_path().length())+2));
				sprintf(deleted_uri_topic, "%s", currentPtr->get_path().c_str());
				RESOURCES_ITER((*global_ctx)->resources, r) {
					if (compareString((char*)r->uri.s, deleted_uri_topic)){
						currentPtr->data_unlock();
						currentPtr->node_unlock();
						//topicDataUnlock(currentPtr);
						//topicNodeUnlock(currentPtr);
						//if (deleteTopic(&topicDB, r->uri.s)){
						if (topic_db.delete_topic(std::string((char*)r->uri.s))){
							if(mqtt_bridge){
								MQTTClient_unsubscribe(*global_client, (char*)r->uri.s);
							}
							RESOURCES_DELETE((*global_ctx)->resources, r);
							coapFreeResource(r);
							break;
						} 
					}
				}
				free(deleted_uri_topic);
			}
			else {
				
				currentPtr->data_unlock();
				currentPtr->node_unlock();
				//topicDataUnlock(currentPtr);
				//topicNodeUnlock(currentPtr);
			}
			
			currentPtr = nextPtr;   
		} /* end while */ 
		earliest_topic_max_age = next_earliest_topic_ma;
		
	} /* end else */ 

	else{
		debug( "No Topic Max-Age Timeout yet.\n" );
		if (earliest_topic_max_age == LONG_MAX){
			debug( "All topic has infinite max-age\n");
		}
		else {
			debug( "Remaining seconds to earliest topic max-age %ld.\n", earliest_topic_max_age - master_time );
		}
	}
}
  
void CoAPBroker::delivered(void *context, MQTTClient_deliveryToken dt){

	debug("Message with token value %d delivery confirmed\n", dt);
	deliveredtoken = dt;
}

int CoAPBroker::msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message){
	int i;
	char* payloadptr;
	
	RESOURCES_ITER(((*global_ctx))->resources, r) {
		if(compareString((char*) r->uri.s, topicName)){
			//TopicDataPtr 	temp_data = cloneTopic(&topicDB,r->uri.s);
			TopicDataPtr 	temp_data = topic_db.copy_topic(std::string((char*)r->uri.s));
			char*			temp_payload = (char*)malloc(sizeof(char)*(message->payloadlen + 2));
			snprintf(temp_payload, (message->payloadlen)+1, "%s", (char*)message->payload);
			debug("Data from DB : %s\n", temp_data->get_data().c_str());
			debug("Data from MQTT : %s\n", temp_payload);
			EString temp_estring = temp_data->get_data();
			if(!compareString(temp_estring.get_char().get(),temp_payload)){
				topic_db.get_topic(std::string(topicName))->update_topic(std::string((char*)message->payload,message->payloadlen), 0);
				//updateTopicData(&topicDB,topicName,0,message->payload,message->payloadlen);
				r->dirty = 1; 
				coap_check_notify((*global_ctx));
			}
			free(temp_payload);
			//freeTopic(temp_data); already use shared_ptr
			
			MQTTClient_freeMessage(&message);
			MQTTClient_free(topicName);
			return 1;
		}
	}
		
	MQTTClient_freeMessage(&message);
	MQTTClient_free(topicName);
	return 0;
}

void CoAPBroker::connlost(void *context, char *cause){
	
	debug("\nConnection lost cause: %s\n", cause);
}

bool CoAPBroker::initialize_mqtt_bridge(){

	if(!mqtt_bridge){
		MQTTClient	client;
		mqtt_bridge = true;
		char mqtt_address[]    				= "tcp://localhost:1883";
		global_client 						= &client;
		MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
		int rc 								= 0 ;

		MQTTClient_create			(&client, mqtt_address, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
		conn_opts.keepAliveInterval = 20;
		conn_opts.cleansession 		= 1;
		MQTTClient_setCallbacks		(client, NULL, connlost, msgarrvd, delivered);

		if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
		{
			debug("Failed to connect, return code %d\n", rc);
			return false;       
		}
	}
	return true;
	/* MQTT Client Init */
}