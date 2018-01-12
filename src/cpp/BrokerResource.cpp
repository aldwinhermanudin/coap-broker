#include "BrokerResource.hpp"

namespace coap{

	namespace broker{
		namespace handler{
			void tae_handler(ProtocolDataUnit response_pdu){
			response_pdu.add_response_data(COAP_RESPONSE_CODE(403)); 
			return ;
			}

			void mr_handler(ProtocolDataUnit response_pdu){
				response_pdu.add_response_data(COAP_RESPONSE_CODE(400));
				return ; 
			}

			void uscf_handler(ProtocolDataUnit response_pdu){
				response_pdu.add_response_data(COAP_RESPONSE_CODE(406));
				return ; 
			}

			bool has_only_digits(const std::string s){
				return s.find_first_not_of( "0123456789" ) == std::string::npos;
			}

			bool is_ct_valid(ProtocolDataUnit request, unsigned short type){
					
				OptionList req_opt = request.get_option();
				if(req_opt.type_exist(COAP_OPTION_CONTENT_TYPE)) {
					
					int ct_value = req_opt.get_option(COAP_OPTION_CONTENT_TYPE).decode_value();
					if (ct_value == type) return true;
					else return false;
				}
				else{
					return false;
				}
			}

			namespace post{
				time_t abs_ma(OptionList option_data){

					time_t opt_topic_ma = 0;
					if (option_data.type_exist(COAP_OPTION_MAXAGE)) {
						/* decode Max-Age Option */
						opt_topic_ma = option_data.get_option(COAP_OPTION_MAXAGE).decode_value();
								
						/* if max-age must have a value of 1 or above 
						* if below 1 set topic max-age to 0 (infinite max-age )
						* else will set topic max-age to ( decode max-age + current time ) */
						if(opt_topic_ma < 1) {  
							return 0;
						}
						else{
							return time(NULL) + opt_topic_ma;
						} 
					}

					return 0;
					debug("topic max-age : %ld\n",opt_topic_ma);
				}

				void s_handler(Server broker_ctx, LinkFormat lf_data, ProtocolDataUnit response_pdu, time_t abs_topic_ma){

					Resource subtopic_resource(lf_data, COAP_RESOURCE_FLAGS_RELEASE_URI);
					subtopic_resource.register_handler(COAP_REQUEST_GET, BrokerResource::hnd_get_topic);
					subtopic_resource.register_handler(COAP_REQUEST_POST, BrokerResource::hnd_post_topic);
					subtopic_resource.register_handler(COAP_REQUEST_PUT, BrokerResource::hnd_put_topic);
					subtopic_resource.register_handler(COAP_REQUEST_DELETE, BrokerResource::hnd_delete_topic);
					subtopic_resource.set_observable(true);

					broker_ctx.add_resource(subtopic_resource);
					Option location(COAP_OPTION_LOCATION_PATH, lf_data.get_path());
					response_pdu.add_option(location);
					
					std::cout << "Resource name : " << subtopic_resource.get_resource()->uri.s << std::endl;
					std::cout << "Resource name size : " << subtopic_resource.get_resource()->uri.length << std::endl;

					BrokerResource::topic_db.add_topic(std::string(lf_data.get_path().get_string()), abs_topic_ma);
					response_pdu.add_response_data(COAP_RESPONSE_CODE(201)); 
					return ; 
					
				}

				bool is_mr(ProtocolDataUnit request, LinkFormat lf_data){
					
					if(is_ct_valid(request, COAP_MEDIATYPE_APPLICATION_LINK_FORMAT)){
						if(lf_data.is_attribute_exist(UString("ct"))){
							return false;
						}
					}

					return true;
				}


				bool is_uscf(LinkFormat lf_data){
					/* search for ct attribute in the new_resource created by parseLinkFormat*/
					//coap_attr_t* new_resource_attr = coap_find_attr(new_resource,(const unsigned char*) "ct", 2);
					Attribute content_type = lf_data.get_attribute(UString("ct"));
					
					/* check ct value, by using isdigit() and iterate to every char in new_resource ct attribute */
					if(has_only_digits(content_type.get_value().get_string())){
						int ct_value = std::stoi(content_type.get_value().get_string());
						if(ct_value >= 0 && ct_value <= 65535){ 
							return false;	/* jump to "Unsupported content format for topic" handler */
						}
					}
					return true;	
				}

				bool is_tae(Server broker_ctx, LinkFormat lf_data){
					if(broker_ctx.is_resource_exist(lf_data.get_path())){
						return true;
					}
					return false;
				}
			}
		}
		namespace sub{
			namespace handler{
				namespace put{
					
					int abs_ct_value(OptionList request_opt){
						unsigned int ct_opt_val_integer = 0;
						if (request_opt.type_exist(COAP_OPTION_CONTENT_TYPE)) { 
							ct_opt_val_integer = request_opt.get_option(COAP_OPTION_CONTENT_TYPE).decode_value();		
						}
					
						return ct_opt_val_integer;
					}
					time_t abs_ma_value(OptionList request_opt){
						time_t ma_opt_val_integer = 0;
						if (request_opt.type_exist(COAP_OPTION_MAXAGE)) {

							ma_opt_val_integer = request_opt.get_option(COAP_OPTION_MAXAGE).decode_value(); 
							ma_opt_val_integer = ma_opt_val_integer + time(NULL); 
						}
						return ma_opt_val_integer;
					}
					bool is_br(OptionList request_opt){
						if (!request_opt.type_exist(COAP_OPTION_CONTENT_TYPE)){ 
							return true;
						}
						return false;
					}

					bool is_uscf(Resource parent_resource, OptionList request_opt){
						if (request_opt.type_exist(COAP_OPTION_CONTENT_TYPE)){ 

							int resource_ct_value = std::stoi(parent_resource.find_attribute(UString("ct")).get_value().get_string());
							int request_ct_value = request_opt.get_option(COAP_OPTION_CONTENT_TYPE).decode_value();
							
							debug("requested ct : %d\n", request_ct_value);
							debug("available ct : %d\n", resource_ct_value);
							
							if (resource_ct_value != request_ct_value){  
								return true;
							}	
						}
						return false;	
					}
					bool is_nf(Resource parent_resource){
						TopicDataPtr temp = BrokerResource::topic_db.copy_topic( parent_resource.get_name().get_string() );
						//TopicDataPtr temp = cloneTopic(&topicDB, resource->uri.s);
						/* Not Found */ // It should never have this condition, ever. Just in Case. //
						if (temp == nullptr ) {
							return true;
						}
						return false;
					}

					bool is_s(Resource parent_resource, UString request_payload, time_t ma_opt_val_time_t){
						return BrokerResource::topic_db.get_topic(parent_resource.get_name().get_string())->update_topic(request_payload.get_string(),ma_opt_val_time_t);
					}

				}
				namespace get{

					bool is_observe_notify_response(Resource current_resource, Address peer_addr, UString token_data, ProtocolDataUnit request){
						if (current_resource.is_observer_exist(peer_addr, token_data) && request.is_empty()){
							return true;
							debug("This is a Subscriber Notification Response \n");
						}
						else return false;
					}
					bool is_observe_regist_request(Resource current_resource,Address peer_addr, UString token_data, ProtocolDataUnit request){
						int obs_opt_val_integer = -1;
						if(!request.is_empty()) { 
													
							if(request.get_option().type_exist(COAP_OPTION_OBSERVE)){
								obs_opt_val_integer = request.get_option().get_option(COAP_OPTION_OBSERVE).decode_value();
							}
						}
						if (current_resource.is_observer_exist(peer_addr, token_data) && !request.is_empty() && obs_opt_val_integer == 0){
							return true;
							debug("This is a Subscriber Registration Request \n");
						}
						return false;
					}

					bool is_ct_exist(ProtocolDataUnit request_pdu){
						if(!request_pdu.is_empty()) {   
							if( request_pdu.get_option().type_exist(COAP_OPTION_CONTENT_TYPE)){
								return true;
							}
						}
						return false;
					}
					bool is_uscf(Resource current_resource,Address peer_addr, UString token_data, ProtocolDataUnit request){

						if ((is_ct_exist(request) || (is_ct_exist(request) && broker::sub::handler::get::is_observe_regist_request(current_resource, peer_addr, token_data, request))) 
								&& !broker::sub::handler::get::is_observe_notify_response(current_resource, peer_addr, token_data, request)){
									return true;
						}
						return false;
					}
					bool is_br(Resource current_resource,Address peer_addr, UString token_data, ProtocolDataUnit request){
						if((!is_ct_exist(request) || (!is_ct_exist(request) && broker::sub::handler::get::is_observe_regist_request(current_resource, peer_addr, token_data, request))) 
								&& !broker::sub::handler::get::is_observe_notify_response(current_resource, peer_addr, token_data, request)){
									return true;
						}
						return false;
					}
					bool is_ct_valid(Resource current_resource, ProtocolDataUnit request){
						int res_ct = std::stoi(current_resource.find_attribute(UString("ct")).get_value().get_string()); 
						if(is_ct_exist(request)){
							int req_ct = request.get_option().get_option(COAP_OPTION_CONTENT_TYPE).decode_value();
							if (res_ct == req_ct){
								return true;
							}
						}
						return false;
						
					}
				} 
			}
		}
	}


TopicDB     BrokerResource::topic_db;
time_t      BrokerResource::earliest_topic_max_age = LONG_MAX;
time_t		BrokerResource::earliest_data_max_age = LONG_MAX;

BrokerResource::BrokerResource(coap::UString name) : Resource(name, COAP_RESOURCE_FLAGS_RELEASE_URI){
	register_handler(COAP_REQUEST_GET, hnd_get_broker);
	register_handler(COAP_REQUEST_POST, hnd_post_broker);
	add_attribute(Attribute(coap::UString("ct"), coap::UString("40"),0));
	add_attribute(Attribute(coap::UString("rt"), coap::UString("\"core.ps\""),0));
}

int	BrokerResource::compareString(char* a, char* b){
	if (a == NULL || b == NULL) return 0;
	if (strcmp(a,b) == 0)	return 1;
	else return 0;
}

void
BrokerResource::hnd_get_broker(coap_context_t *ctx, struct coap_resource_t *resource, 
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
BrokerResource::hnd_post_broker(coap_context_t *ctx, struct coap_resource_t *resource, 
              const coap_endpoint_t *local_interface, coap_address_t *peer, 
              coap_pdu_t *request, str *token, coap_pdu_t *response) 
{
	//coap::HandlerData handler_data(ctx, resource, local_interface, peer, request, token, response);
	coap::Server broker_ctx(ctx);
	coap::Resource parent_resource(resource);

	/* declare a safe variable for data */
	ProtocolDataUnit request_pdu(request);
	ProtocolDataUnit response_pdu(response);
	UString request_payload = request_pdu.get_data();
	/* declare a safe variable for data */
	
	/* parse payload */
	LinkFormat lf_data(request_payload, COAP_ATTR_FLAGS_RELEASE_NAME | COAP_ATTR_FLAGS_RELEASE_VALUE); 
	lf_data.set_path(UString(parent_resource.get_name().get_string() + std::string("/") + std::string(lf_data.get_path().get_string())));

	int status = lf_data.is_valid();
	debug("Parser status : %d\n", status);
		
	coap::OptionList option_data(request);
	time_t abs_topic_ma = broker::handler::post::abs_ma(option_data);
	
	debug("topic abs max-age : %ld\n",abs_topic_ma);
	
	/* malformed request */
	if (broker::handler::post::is_mr(request_pdu,lf_data)){
		debug("ct option is not link format\n"); 
		broker::handler::mr_handler(response_pdu);
		return;
	} 
	/* malformed request */

	/* unsupported content format for topic. */
	if(broker::handler::post::is_uscf(lf_data)){
		debug("UCSF Error");
		broker::handler::uscf_handler(response_pdu);
		return;
	}
	/* unsupported content format for topic. */
	
	/* Topic already exists. */ 
	/* "Topic already exists" handler if both resource have the same uri */
	if(broker::handler::post::is_tae(broker_ctx,lf_data)){
		if (abs_topic_ma < earliest_topic_max_age && abs_topic_ma != 0) earliest_topic_max_age = abs_topic_ma;
		
		topic_db.get_topic(lf_data.get_path().get_string())->update_topic(abs_topic_ma);
		broker::handler::tae_handler(response_pdu);
		return ;
	}
	/* Topic already exists. */

	/* Successful Creation of the topic */
	if (abs_topic_ma < earliest_topic_max_age && abs_topic_ma != 0){
		earliest_topic_max_age = abs_topic_ma;
	}

	/* create resource payload */
	broker::handler::post::s_handler(broker_ctx, lf_data, response_pdu, abs_topic_ma);
	/* Successful Creation of the topic */
}

void BrokerResource::hnd_get_topic(coap_context_t *ctx, struct coap_resource_t *resource, 
     const coap_endpoint_t *local_interface, coap_address_t *peer, 
     coap_pdu_t *request, str *token, coap_pdu_t *response){
    
	Server broker_ctx(ctx);
	Resource current_resource(resource);
	ProtocolDataUnit request_pdu(request);
	ProtocolDataUnit response_pdu(response);

	int ct_attr_value = std::stoi(current_resource.find_attribute(UString("ct")).get_value().get_string());
	/* to get max_age value and observe*/

	Address peer_addr(peer);
	UString token_data(token->s, token->length);
	 

	/* Unsupported Content Format */
	if ( broker::sub::handler::get::is_uscf(current_resource, peer_addr, token_data, request_pdu)){
	
		if (!broker::sub::handler::get::is_ct_valid(current_resource, request_pdu)){ 
			response_pdu.add_response_data(COAP_RESPONSE_CODE(415)); 
			return;
		}	
	}
	/* Unsupported Content Format */

	/* Bad Request */
	if(broker::sub::handler::get::is_br(current_resource, peer_addr, token_data, request_pdu)){
		response_pdu.add_response_data(COAP_RESPONSE_CODE(400));
		return;
	}
	/* Bad Request */

	//TopicDataPtr temp = cloneTopic(&topicDB, resource->uri.s);
	/* It should never have this condition, ever. Just in Case. */
	/* Not Found */
	TopicDataPtr temp = topic_db.copy_topic(std::string((char*) resource->uri.s));
	if (temp == nullptr ) {
		response_pdu.add_response_data(COAP_RESPONSE_CODE(404));
		return;
	}
	/* Not Found */

	if(broker::sub::handler::get::is_ct_valid(current_resource, request_pdu) || broker::sub::handler::get::is_observe_notify_response(current_resource, peer_addr, token_data, request_pdu)){
	
		/* No Content */
		if (temp == nullptr){
			if (current_resource.is_observer_exist(peer_addr, token_data)) {
				response_pdu.add_option(Option(COAP_OPTION_OBSERVE, Option::encode_data(broker_ctx.get_obs_value())));
			}
			response_pdu.add_option(Option(COAP_OPTION_CONTENT_TYPE, Option::encode_data(ct_attr_value)));
			response_pdu.add_response_data(COAP_RESPONSE_CODE(204), UString("No Content"));
			return;
		}
		/* No Content */
		
		/* No Content || Content */
		else {
			if (current_resource.is_observer_exist(peer_addr, token_data)) {
				response_pdu.add_option(Option(COAP_OPTION_OBSERVE, Option::encode_data(broker_ctx.get_obs_value())));
			}		
			time_t remaining_maxage_time = temp->get_data_ma() - time(NULL);
			if (remaining_maxage_time < 0 && !(temp->get_data_ma() == 0)){
				response_pdu.add_option(Option(COAP_OPTION_CONTENT_TYPE, Option::encode_data(ct_attr_value)));
				response_pdu.add_response_data(COAP_RESPONSE_CODE(204), UString("No Content"));
				//freeTopic(temp);  already using shared_ptr
				return;
			}
			else{
				if ((!(temp->get_data_ma() == 0))){
					debug("Current time   : %ld\n",time(NULL));
					debug("Expired time   : %ld\n",temp->get_data_ma());
					debug("Remaining time : %ld\n",temp->get_data_ma() - time(NULL));
					response_pdu.add_option(Option(COAP_OPTION_MAXAGE, Option::encode_data(remaining_maxage_time)));
				}
				response_pdu.add_option(Option(COAP_OPTION_CONTENT_TYPE, Option::encode_data(ct_attr_value)));
				response_pdu.add_response_data(COAP_RESPONSE_CODE(205),UString(temp->get_data().c_str()));
				return;
			}
		}
	/* No Content || Content */
	} 		 
}


void BrokerResource::hnd_put_topic(coap_context_t *ctx ,
             struct coap_resource_t *resource ,
             const coap_endpoint_t *local_interface ,
             coap_address_t *peer ,
             coap_pdu_t *request,
             str *token ,
             coap_pdu_t *response){

	coap::Server broker_ctx(ctx);
	coap::Resource parent_resource(resource);

	/* declare a safe variable for data */
	ProtocolDataUnit request_pdu(request);
	ProtocolDataUnit response_pdu(response);
	UString request_payload = request_pdu.get_data();

	OptionList request_opt(request);
	/* to get max_age and ct value */
	unsigned int ct_opt_val_integer = broker::sub::handler::put::abs_ct_value(request_opt);
	time_t		 ma_opt_val_time_t 	= broker::sub::handler::put::abs_ma_value(request_opt); 

	/* to print max_age value*/
	debug("Option Max-age   : %d\n",request_opt.get_option(COAP_OPTION_MAXAGE).decode_value());
	debug("Absolute Max-age : %ld\n",ma_opt_val_time_t);
	/* to print max_age value*/
	
	/* Bad Request */
	if(broker::sub::handler::put::is_br(request_opt)) {
		response_pdu.add_response_data(COAP_RESPONSE_CODE(400)); 
		return;
	}	
	/* Bad Request */
	
	/* Unsupported Content Format */
	else if(broker::sub::handler::put::is_uscf(parent_resource, request_opt)){
		response_pdu.add_response_data(COAP_RESPONSE_CODE(415)); 
		return;
	}
	/* Unsupported Content Format */
	
	/* Not Found */ // It should never have this condition, ever. Just in Case. // 
	if(broker::sub::handler::put::is_nf(parent_resource)){

		response_pdu.add_response_data(COAP_RESPONSE_CODE(404)); 
		return;
	} 
	/* Not Found */

	/* Success */
	if(broker::sub::handler::put::is_s(parent_resource, request_payload, ma_opt_val_time_t)){
		if (ma_opt_val_time_t < earliest_data_max_age && ma_opt_val_time_t != 0){
			earliest_data_max_age = ma_opt_val_time_t;
		}
		parent_resource.set_dirty(true);
		broker_ctx.notify();
		response_pdu.add_response_data(COAP_RESPONSE_CODE(204)); 
		return;
	}
	/* Success */
}

               
void BrokerResource::hnd_post_topic(coap_context_t *ctx ,
                struct coap_resource_t *resource ,
                const coap_endpoint_t *local_interface ,
                coap_address_t *peer ,
                coap_pdu_t *request ,
                str *token ,
                coap_pdu_t *response ){
	
	hnd_post_broker(ctx, resource, local_interface, peer, request, token, response);
} 

void BrokerResource::hnd_delete_topic(coap_context_t *ctx ,
                struct coap_resource_t *resource ,
                const coap_endpoint_t *local_interface ,
                coap_address_t *peer ,
                coap_pdu_t *request ,
                str *token ,
                coap_pdu_t *response ){

	Server broker_ctx(ctx);
	Resource current_resource(resource);
	ProtocolDataUnit response_pdu(response);
	std::function<bool(Resource,Resource)> comparator = [](Resource comparison, Resource comparator){
			UString subtopic(comparison.get_name().get_string() + std::string("/"));
			if(comparator.get_name().has_substr(subtopic)){
				if(topic_db.delete_topic(comparator.get_name().get_string())){
					return true;
				}
			}
			return false;
		};
	int counter = 0;
	
	debug("Resource name : %s\n",current_resource.get_name().get_string().c_str());
	counter = topic_db.delete_topic(current_resource.get_name().get_string());
	
	if (counter){
		counter += broker_ctx.delete_resource(current_resource, comparator);
		broker_ctx.delete_resource(current_resource);

		response_pdu.add_response_data(COAP_RESPONSE_CODE(202));
	}
	else {
		response_pdu.add_response_data(COAP_RESPONSE_CODE(404));
	}

	debug(" Counter : %d\n",counter);
}



void BrokerResource::topicDataMAMonitor(){
	
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

void BrokerResource::topicMAMonitor(coap_context_t* global_ctx){
	
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
				RESOURCES_ITER((global_ctx)->resources, r) {
					if (compareString((char*)r->uri.s, deleted_uri_topic)){
						currentPtr->data_unlock();
						currentPtr->node_unlock();
						//topicDataUnlock(currentPtr);
						//topicNodeUnlock(currentPtr);
						//if (deleteTopic(&topicDB, r->uri.s)){
						if (topic_db.delete_topic(std::string((char*)r->uri.s))){
							RESOURCES_DELETE((global_ctx)->resources, r);
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
}