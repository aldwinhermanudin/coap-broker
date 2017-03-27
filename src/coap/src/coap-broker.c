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
#include <pthread.h>    
#include <time.h>     
#include <semaphore.h> 
#include "MQTTClient.h"
#include <ctype.h>
   
#define LL_DATABASE
#define LF_PARSER
#define LIBCOAP_MOD
#define TESTING
#define MQTTTEST
 
#ifdef LIBCOAP_MOD
void coapDeleteAttr(coap_attr_t *attr) {
     if (!attr)
       return; 
     coap_free(attr->name.s); 
     coap_free(attr->value.s);
    
     coap_free_type(COAP_RESOURCEATTR, attr); 
   }           
void coapFreeResource(coap_resource_t *resource){
	 coap_attr_t *attr, *tmp;
     coap_subscription_t *obs, *otmp;
   
     assert(resource);
   
     /* delete registered attributes */
     LL_FOREACH_SAFE(resource->link_attr, attr, tmp) coapDeleteAttr(attr);
   
     if (resource->flags & COAP_RESOURCE_FLAGS_RELEASE_URI)
       coap_free(resource->uri.s);
   
     /* free all elements from resource->subscribers */
     // modifying this to re-create free resource. CHANGED : COAP_FREE_TYPE(subscription, obs) to coap_free(obs) 
     // check original coap_free_resource()
     LL_FOREACH_SAFE(resource->subscribers, obs, otmp) coap_free(obs);
 
     coap_free_type(COAP_RESOURCE, resource); 
}
	
#endif

#ifdef LL_DATABASE  
/* self-referential structure */
struct topicData {            
	char* path;
	char * data;		/* each topicData contains a string */
    time_t topic_ma;	// max-age for topic in time after epoch
    time_t data_ma;		// max-age for data in time after epoch
    struct topicData *nextPtr; /* pointer to next node*/ 
}; /* end structure topicData */

typedef struct topicData TopicData; /* synonym for struct topicData */
typedef TopicData *TopicDataPtr; /* synonym for TopicData* */

/* prototypes */
TopicDataPtr getTopic( TopicDataPtr *sPtr, char* path);
int			setTopic( TopicDataPtr *sPtr, 
					char* path, char* data, size_t data_size, 
					time_t topic_ma, time_t data_ma);
					
int 		topicExist(TopicDataPtr *sPtr, char* path );				
int			addTopic(TopicDataPtr *sPtr,
					char* path, size_t path_size,
					time_t topic_ma);
int			addTopicWEC(TopicDataPtr *sPtr,
					char* path, size_t path_size,
					time_t topic_ma);
int 		deleteTopic( TopicDataPtr *sPtr, char* path );					
int			updateTopicInfo(TopicDataPtr *sPtr,
					char* path, time_t topic_ma);
					
int			updateTopicData(TopicDataPtr *sPtr,
					char* path, time_t data_ma,
					char* data, size_t data_size);


int 		DBEmpty( TopicDataPtr sPtr );
void 		printDB( TopicDataPtr currentPtr );
void 		cleanDB( TopicDataPtr *sPtr );

int 		compareString(char* a, char* b){
	if (a == NULL || b == NULL) return 0;
	if (strcmp(a,b) == 0)	return 1;
	else return 0;
}

/* start function get */
TopicDataPtr getTopic( TopicDataPtr *sPtr, char* path)
{ 
    TopicDataPtr previousPtr = NULL; /* pointer to previous node in list */
    TopicDataPtr currentPtr = NULL;  /* pointer to current node in list */
    TopicDataPtr tempPtr = NULL;     /* temporary node pointer */
	
	if (path == NULL || DBEmpty(*sPtr)) { return NULL;} 
	
    /* delete first node */
  
    if ( compareString(( *sPtr )->path,path)) { 
        tempPtr = *sPtr; /* hold onto node being removed */
        return tempPtr;
    } /* end if */
    else { 
        previousPtr = *sPtr;
        currentPtr = ( *sPtr )->nextPtr;

        /* loop to find the correct location in the list */
        while ( currentPtr != NULL && !compareString(currentPtr->path,path) ) { 
            previousPtr = currentPtr;         /* walk to ...   */
            currentPtr = currentPtr->nextPtr; /* ... next node */  
        } /* end while */

        /* delete node at currentPtr */
        if ( currentPtr != NULL ) { 
            tempPtr = currentPtr;
            return tempPtr;
        } /* end if */
     
    } /* end else */

    return NULL;

} 

int setTopic( TopicDataPtr *sPtr, 
					char* path, char* data, size_t data_size, 
					time_t topic_ma, time_t data_ma)
{ 
	
	TopicDataPtr topic = getTopic(sPtr, path);
	if (topic != NULL){
		if( data_size > 0 && data != NULL){
			char* temp_data = malloc(sizeof(char) * (data_size + 2));
			snprintf(temp_data,sizeof(char) * (data_size + 1), "%s", data);
			
			if (topic->data != NULL){
				free( topic->data );
			}
			topic->data			= temp_data;
		}
		else {
			topic->data			= NULL;
		}
		
		topic->topic_ma		= topic_ma;
		topic->data_ma 		= data_ma;
		return 1;
	}
	
    return 0;

} 

int topicExist(TopicDataPtr *sPtr, char* path )
{ 
    TopicDataPtr topic = getTopic(sPtr, path);
	if (topic == NULL) return 0;
	else return 1;

} 

int	addTopic(TopicDataPtr *sPtr,
					char* path, size_t path_size,
					time_t topic_ma){
					
    TopicDataPtr newPtr = NULL;      /* pointer to new node */
    TopicDataPtr previousPtr = NULL; /* pointer to previous node in list */
    TopicDataPtr currentPtr = NULL;  /* pointer to current node in list */

    newPtr = malloc( sizeof( TopicData ) ); /* create node on heap */
	
    if ( newPtr != NULL ) { /* is space available */
		
		
		/* add data to new struct here */
		if (path_size > 0 && path != NULL){
			char* temp_path = malloc(sizeof(char) * (path_size + 2));		
			snprintf(temp_path,sizeof(char) * (path_size+1), "%s", path);
			
			newPtr->path = temp_path; /* place value in node */
			newPtr->data = NULL; /* place value in node */
			newPtr->data_ma = 0; /* place value in node */
			newPtr->topic_ma = topic_ma; /* place value in node */
			newPtr->nextPtr = NULL; /* node does not link to another node */
			/* add data to new struct here */
		}
		else {
			free(newPtr);
			return 0;
		}
		
        previousPtr = NULL;
        currentPtr = *sPtr;

        /* loop to find the correct location in the list */
        while ( currentPtr != NULL) { 
			//TODO: if data available, stop. update topic_ma and ct
			
            previousPtr = currentPtr;          /* walk to ...   */
            currentPtr = currentPtr->nextPtr;  /* ... next node */
        } /* end while */

        /* insert new node at beginning of list */
        if ( previousPtr == NULL ) { 
            newPtr->nextPtr = *sPtr;
            *sPtr = newPtr;
        } /* end if */
        else { /* insert new node between previousPtr and currentPtr */
            previousPtr->nextPtr = newPtr;
            newPtr->nextPtr = currentPtr;
        } /* end else */
		return 1;
    } /* end if */
    else {
        printf( "%s not inserted. No memory available.\n", path );
		return 0;
    } /* end else */

} 

int	addTopicWEC(TopicDataPtr *sPtr,
					char* path, size_t path_size,
					time_t topic_ma){
	
	TopicDataPtr temp = getTopic(sPtr, path);
	if(temp != NULL){
		
		return updateTopicInfo(sPtr,path, topic_ma);
	}
	else {
		return addTopic( sPtr, path, path_size, topic_ma);
	}
					
}
int deleteTopic( TopicDataPtr *sPtr, char* path)
{ 
    TopicDataPtr previousPtr = NULL; /* pointer to previous node in list */
    TopicDataPtr currentPtr = NULL;  /* pointer to current node in list */
    TopicDataPtr tempPtr = NULL;     /* temporary node pointer */

	if (path == NULL || DBEmpty(*sPtr)) { return 0;}
	
    /* delete first node */
    if ( compareString(( *sPtr )->path,path)) { 
        tempPtr = *sPtr; /* hold onto node being removed */
        *sPtr = ( *sPtr )->nextPtr; /* de-thread the node */
        free( tempPtr->data );
        free( tempPtr->path );
        free( tempPtr ); /* free the de-threaded node */
        return 1;
    } /* end if */
    else { 
        previousPtr = *sPtr;
        currentPtr = ( *sPtr )->nextPtr;

        /* loop to find the correct location in the list */
        while ( currentPtr != NULL && !compareString(currentPtr->path,path) ) { 
            previousPtr = currentPtr;         /* walk to ...   */
            currentPtr = currentPtr->nextPtr; /* ... next node */  
        } /* end while */

        /* delete node at currentPtr */
        if ( currentPtr != NULL ) { 
            tempPtr = currentPtr;
            previousPtr->nextPtr = currentPtr->nextPtr;
            free( tempPtr->data );
            free( tempPtr->path );
            free( tempPtr );
            return 1;
        } /* end if */
     
    } /* end else */

    return 0;

} /* end function delete */

int updateTopicInfo(TopicDataPtr *sPtr,
					char* path, time_t topic_ma){
						
	TopicDataPtr temp = getTopic(sPtr, path);
	if(temp != NULL){
		size_t data_size = 0;
		if (temp->data == NULL) {
			data_size = 0;
		}
		else {
			data_size = strlen(temp->data);
		}
		return setTopic(sPtr,path, temp->data, data_size, topic_ma, temp->data_ma);
	}
	else {
		return 0;
	}
	
} /* end function delete */


/* start function set */
int updateTopicData(TopicDataPtr *sPtr,
					char* path, time_t data_ma,
					char* data, size_t data_size){
   
	TopicDataPtr temp = getTopic(sPtr, path);
	if(temp != NULL){
		
		return setTopic(sPtr,path, data, data_size, temp->topic_ma, data_ma);
	}
	else {
		return 0;
	}

} /* end function delete */


/* Return 1 if the list is empty, 0 otherwise */
int DBEmpty( TopicDataPtr sPtr )
{ 
    return sPtr == NULL;

} /* end function isEmpty */

/* Print the list */
void printDB( TopicDataPtr currentPtr )
{ 

    /* if list is empty */
    if ( currentPtr == NULL ) {
        printf( "List is empty.\n\n" );
    } /* end if */
    else { 
        printf( "The list is:\n" );

        /* while not the end of the list */
        while ( currentPtr != NULL ) { 
            printf( "%s %s %d %d --> ",currentPtr->path, currentPtr->data,
            (int)currentPtr->topic_ma, (int)currentPtr->data_ma);
            currentPtr = currentPtr->nextPtr;   
        } /* end while */

        printf( "NULL\n\n" );
    } /* end else */

} /* end function printList */

/* Clean the list */
void cleanDB( TopicDataPtr *sPtr )
{ 
 
 while(!DBEmpty(*sPtr)){ 
	TopicDataPtr tempPtr = NULL;     /* temporary node pointer */
        tempPtr = *sPtr; /* hold onto node being removed */
        *sPtr = ( *sPtr )->nextPtr; /* de-thread the node */
        free( tempPtr->data );
        free( tempPtr->path );
        free( tempPtr ); /* free the de-threaded node */ 
    } /* end if */

} /* end function printList */

#endif

#ifdef LF_PARSER
/* restrict doesn't work properly*/
#define RESTRICT_CHAR
/* Link Format Parser starts here */

int optionValidation(char* source){ 
  char * pch;
  int counter = 0;
  pch=strchr(source,'=');
  while (pch!=NULL)
  {
    counter++;
    pch=strchr(pch+1,'=');
  }
  if (strlen(source) < 3 || source[0] == '=' || source[strlen(source)-1] == '=') return 0;
  return counter;
}

int calOptionSize(char* source, int* type, int* data){
	
	if (optionValidation(source) == 1){
		*type = (int) strcspn(source, "=");
		char* pch = strchr(source,'=')+1;
		*data = (int) strlen(pch);
		return 1;			
	}
	return 0;
} 

int parseOption(char * source, char* type, char* data){
 
		if (optionValidation(source) == 1){
			
			int counter = strcspn(source, "=");
			char* pch = strchr(source,'=');
			if (pch != NULL){
				strncpy(type,source,counter);
				type[counter] = '\0';
		
				/*only allow alphanum and '-'*/
				#ifdef RESTRICT_CHAR
				for(int i = 0; i < strlen(type);i++){
					if(!isalnum(type[i]) && type[i] != '-'){
						return 0;
					}
				}
				#endif
				/*only allow alphanum and '-' */				
				
				if(pch[1] == '"')
					strcpy(data,pch+2);
				else
					strcpy(data,pch+1);
					
				if(pch[strlen(pch+1)] == '"')
					data[strlen(data)-1] = '\0';
					
				/*only allow alphanum and '-'*/
				#ifdef RESTRICT_CHAR	
				for(int i = 0; i < strlen(data);i++){
					if(!isalnum(data[i]) && data[i] != '-'){
						return 0;
					}
				}
				#endif				
				/*only allow alphanum and '-'*/						
				
				return 1;
			}
		}
		return 0;
}
 
int calPathSize(char* source){
	
	if (source[0] == '<' && source[1] != '/' && source[strlen(source)-2] != '/' && source[strlen(source)-1] == '>' ){
		int path_size = strlen(source)-2;
		if (path_size > 0) return path_size;
		else return 0;
	}
	return 0;
}
int parsePath(char* source, char* path){
	
	if (calPathSize(source)){ 
		int path_size = strlen(source)-2;
		strncpy(path, source+1, path_size);
		path[path_size] = '\0';
		
		/*only allow alphanum and '/' and '-'*/	
		#ifdef RESTRICT_CHAR
		for(int i = 0; i < path_size;i++){
			if(!isalnum(path[i]) && path[i] != '/' && path[i] != '-'){
				return 0;
			}
		}
		#endif
		/*only allow alphanum and '/' and '-'*/
		 
		return 1;
	}
	return 0;
}

int optionRegister(coap_resource_t **resource , char* temp_str){
	
	int type_size, data_size;
	int upper_status = calOptionSize(temp_str,&type_size, &data_size);
	if (upper_status && type_size > 0 && data_size > 0){
		char* opt_type = malloc(sizeof(char) * (type_size+1));
		char* opt_data = malloc(sizeof(char) * (data_size+1));
		int status = parseOption(temp_str, opt_type,opt_data);
		if(status){
			coap_add_attr(*resource, opt_type, strlen(opt_type), opt_data, strlen(opt_data), 0);
		}
		else {
			free(opt_type);
			free(opt_data);
		}
		// TODO:free resource attr if status error
		return status;
	}
	return 0;
}

int pathRegister(coap_resource_t *old_resource, coap_resource_t **new_resource , char* temp_str){
	
	if (calPathSize(temp_str) > 0){
		int total_size = old_resource->uri.length + calPathSize(temp_str) + 2; // supposed to be 1, but for safety reason I add 1 more byte
		char* rel_path = malloc(sizeof(char) * (calPathSize(temp_str)+1)); 
		int status = parsePath(temp_str, rel_path);
		if(status){
			char* abs_path = malloc(sizeof(char) * (total_size + 1));
			char* parent_path = malloc(sizeof(char) * (old_resource->uri.length + 2));
			snprintf(parent_path,old_resource->uri.length+1, "%s", old_resource->uri.s);
			sprintf(abs_path,"%s/%s", parent_path, rel_path);
			free(parent_path);
			*new_resource = coap_resource_init(abs_path, strlen(abs_path), COAP_RESOURCE_FLAGS_RELEASE_URI); 
		}
		free(rel_path);
		return status;
	}
	return 0;
}

int parseLinkFormat(char* str, coap_resource_t* old_resource, coap_resource_t** resource){ 
	  char * pch;
	  int last_counter = 0;
	  int counter = 0;
	  int master_status = 0;
	  
	  /* Error Checker*/ 
	  if(strchr(str,',') != NULL) return 0;
	  if(str[0] == ';') return 0;
	  /* Error Checker*/
	  
	  pch=strchr(str,';');
	  while (pch!=NULL)
	  {
		counter = pch-str;
		
		int size = counter-last_counter;
		
		// first element of link format
		if(str[last_counter] != ';'){
			
			char* temp_str = malloc(sizeof(char) * (size+1));
			strncpy(temp_str, str+last_counter, size);
			temp_str[size] = '\0'; 
				int status = pathRegister(old_resource, resource,temp_str);
			free(temp_str);
			if (!status) return 0;
			else master_status = 1;
		}
		
		// in between first and last element of link format
		else{
			char* temp_str = malloc(sizeof(char) * (size));
			strncpy(temp_str, str+last_counter+1, size-1);
			temp_str[size-1] = '\0';
				int status = optionRegister(resource,temp_str); 
			//TODO : fix this free(), somehow it works now
			free(temp_str);
			if (!status) {
				coapFreeResource(*resource);
				return 0;
			}
			else master_status = 1;
		}
		
		// last element of link format
		pch=strchr(pch+1,';');
		if (pch == NULL){
			size = strlen(str+counter+1);
			char* temp_str = malloc(sizeof(char) * (size+1));
			strncpy(temp_str, str+counter+1, size);
			temp_str[size] = '\0';
				int status = optionRegister(resource,temp_str); 
			//TODO : fix this free(), somehow it works now
			free(temp_str); 
			if (!status) {
				coapFreeResource(*resource);
				return 0;
			}
			else master_status = 1;
		}
				
		last_counter = counter;
	  }
	  return master_status;
}
 
/* Link Format Parser ends here */

#endif



#ifdef TESTING

	coap_context_t**  	global_ctx;
	MQTTClient* 		global_client;
	TopicDataPtr 		topicDB = NULL; /* initially there are no nodes */
	
	static int quit = 0;
	static void	handle_sigint(int signum) {
	  quit = 1;
	  // TODO: Need more testing
	  RESOURCES_ITER((*global_ctx)->resources, r) {
		deleteTopic(&topicDB, r->uri.s);
		MQTTClient_unsubscribe(*global_client, r->uri.s);
		RESOURCES_DELETE((*global_ctx)->resources, r);
		coapFreeResource(r);
	  } 
	
	  coap_free_context((*global_ctx));  
	  MQTTClient_disconnect((*global_client), 10000);
      MQTTClient_destroy(global_client);
	}
	
	void topicMaxAgeMonitor( TopicDataPtr currentPtr ){
 
		/* if list is empty */
		if ( currentPtr == NULL ) {
			printf( "List is empty.\n\n" );
		} /* end if */
		else { 
			printf( "The list is:\n" );

			/* while not the end of the list */
			while ( currentPtr != NULL ) { 
				
				printf( "%s\t\t%ld\t%ld | %s\n ",currentPtr->path, currentPtr->topic_ma, time(NULL), currentPtr->topic_ma < time(NULL) && currentPtr->topic_ma != 0? "Expired. Deleting..." : "Valid");
				if (currentPtr->topic_ma < time(NULL) && currentPtr->topic_ma != 0){
					char* deleted_uri_topic = malloc(sizeof(char) *(strlen(currentPtr->path)+2));
					sprintf(deleted_uri_topic, "%s", currentPtr->path);
					RESOURCES_ITER((*global_ctx)->resources, r) {
						if (compareString(r->uri.s, deleted_uri_topic)){
							if (deleteTopic(&topicDB, r->uri.s)){
								MQTTClient_unsubscribe(*global_client, r->uri.s);
								RESOURCES_DELETE((*global_ctx)->resources, r);
								coapFreeResource(r);
								break;
							} 
						}
					}
					free(deleted_uri_topic);
				}
				
				currentPtr = currentPtr->nextPtr;   
			} /* end while */

			printf( "NULL\n\n" );
		} /* end else */ 
	}
#endif

#ifdef MQTTTEST
	#define ADDRESS     "tcp://localhost:1883"
	#define CLIENTID    "CoAPBroker" 
	#define QOS         1
	#define TIMEOUT     10000L

	volatile MQTTClient_deliveryToken deliveredtoken;
		
	void delivered(void *context, MQTTClient_deliveryToken dt)
	{
		printf("Message with token value %d delivery confirmed\n", dt);
		deliveredtoken = dt;
	}

	int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
	{
		int i;
		char* payloadptr;
		
		
		RESOURCES_ITER(((*global_ctx))->resources, r) {
			if(compareString(r->uri.s, topicName)){
				TopicDataPtr 	temp_data = getTopic(&topicDB,r->uri.s);
				char*			temp_payload = malloc(sizeof(char)*(message->payloadlen + 2));
				snprintf(temp_payload, (message->payloadlen)+1, "%s", (char*)message->payload);
				printf("Data from DB : %s\n", temp_data->data);
				printf("Data from MQTT : %s\n", temp_payload);
				if(!compareString(temp_data->data,temp_payload)){
					updateTopicData(&topicDB,topicName,0,message->payload,message->payloadlen);
					r->dirty = 1;
					/* this can be changed to coap_notify_observers, but coap_notify_observers is static. so either
					 * create a similir function or change resource.c 
					 */
					coap_check_notify((*global_ctx));
				}
				free(temp_payload);
				break;
			}
		}
		
		MQTTClient_freeMessage(&message);
		MQTTClient_free(topicName);
		return 1;
	}

	void connlost(void *context, char *cause)
	{
		printf("\nConnection lost\n");
		printf("     cause: %s\n", cause);
	}

#endif
 
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
              
static void hnd_post_broker(coap_context_t *ctx,
             struct coap_resource_t *resource,
             const coap_endpoint_t *local_interface,
             coap_address_t *peer,
             coap_pdu_t *request,
             str *token,
             coap_pdu_t *response);
             
static void hnd_delete_broker(coap_context_t *ctx,
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
	coap_resource_t* broker_resource;
	fd_set           readfds;    
	global_ctx	= &ctx;
	char broker_path[8] = "ps";
	
	/* MQTT Client Init */
	MQTTClient client;
    global_client = &client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;

    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);       
    }
	/* MQTT Client Init */
	
	/* turn on debug an printf() */
	coap_log_t log_level = LOG_DEBUG;
	coap_set_log_level(log_level);
	/* turn on debug an printf() */
		
	/* Prepare the CoAP server socket */ 
	coap_address_init(&serv_addr);
	serv_addr.addr.sin.sin_family      = AF_INET6;
	serv_addr.addr.sin.sin_addr.s_addr = INADDR_ANY;
	serv_addr.addr.sin.sin_port        = htons(5683); //default port
	ctx                                = coap_new_context(&serv_addr);
	if (!ctx) exit(EXIT_FAILURE);
	/* Prepare the CoAP server socket */ 	
	
	/* Initialize the observable resource */
	broker_resource = coap_resource_init(broker_path, strlen(broker_path), 0);
	coap_register_handler(broker_resource, COAP_REQUEST_GET, hnd_get_broker);
	coap_register_handler(broker_resource, COAP_REQUEST_POST, hnd_post_broker);
	coap_register_handler(broker_resource, COAP_REQUEST_DELETE, hnd_delete_broker);
	coap_add_attr(broker_resource, (unsigned char *)"ct", 2, (unsigned char *)"40", strlen("40"), 0); //40 :link
	coap_add_attr(broker_resource, (unsigned char *)"rt", 2, (unsigned char *)"core.ps", strlen("core.ps"), 0);
	coap_add_resource(ctx, broker_resource);
	/* Initialize the observable resource */	
	
	
	signal(SIGINT, handle_sigint);
	/*Listen for incoming connections*/	
	while (!quit) {
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
        //printDB(topicDB);
        topicMaxAgeMonitor(topicDB);
        
        // coap_check_notify is needed if there is a observable resource
        // either put here or in put handler
        //coap_check_notify(ctx);
    }
    
    coap_free_context(ctx);  
	MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}

static void
hnd_get_broker(coap_context_t *ctx, struct coap_resource_t *resource, 
              const coap_endpoint_t *local_interface, coap_address_t *peer, 
              coap_pdu_t *request, str *token, coap_pdu_t *response) 
{
	unsigned char buf[3];
	unsigned char response_data[1024];
	size_t response_size = sizeof(response_data);
	size_t response_offset = 0;
	RESOURCES_ITER(ctx->resources, r) {
		response_size = sizeof(response_data);
		response_offset = 0;
		coap_print_link(r, response_data, &response_size, &response_offset);
	}
  

	response->hdr->code 		  = COAP_RESPONSE_CODE(205); 
	// option order matters!
	coap_add_option(response, COAP_OPTION_CONTENT_TYPE, coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);
	coap_add_option(response, COAP_OPTION_MAXAGE,coap_encode_var_bytes(buf, 20), buf); // max-age in seconds, so 30 seconds (to mars. #pun)
	coap_add_data  (response, response_size, response_data);
	//coap_add_data  (response, resource->uri.length, resource->uri.s);		
}

static void
hnd_post_broker(coap_context_t *ctx, struct coap_resource_t *resource, 
              const coap_endpoint_t *local_interface, coap_address_t *peer, 
              coap_pdu_t *request, str *token, coap_pdu_t *response) 
{

	coap_resource_t *new_resource = NULL;
	
	/* parse payload */
	size_t size;
    unsigned char *data;
    unsigned char *data_safe;
	(void)coap_get_data(request, &size, &data);
	data_safe = coap_malloc(sizeof(char)*(size+2));
	snprintf(data_safe,size+1, "%s", data);
	int ct_value_valid = 1;
	//int status = 0;
	int status = parseLinkFormat(data_safe,resource, &new_resource);
	/* parse payload */
	
	coap_free(data_safe);
	
	/* to get max_age value */
	coap_opt_t *option;
	coap_opt_iterator_t opt_iter; 
	coap_option_iterator_init(request, &opt_iter, COAP_OPT_ALL);
	unsigned char* max_age_val;
	unsigned short max_age_len;	 
	int rel_topic_ma;
	int topic_ma_status = 0;
	time_t topic_ma = 0;
	
	while ((option = coap_option_next(&opt_iter))) {
		// changing COAP_OPTION_MAXAGE into any coap Option will return the selected option type
	   if (opt_iter.type == COAP_OPTION_MAXAGE) { 
				topic_ma_status = 1;
				max_age_len = coap_opt_length(option);
				max_age_val = malloc(sizeof(char) * (max_age_len+2));
				snprintf(max_age_val, max_age_len +1, "%s", coap_opt_value(option));
				rel_topic_ma = atoi(max_age_val);
				free(max_age_val);
				break;
	   }
	}
	
	if(topic_ma_status){
		if(rel_topic_ma < 1) { // if max-age input is 0 or below
			topic_ma = 0;	// set topic_ma to 0, which means topic never expires
		}
		else{
			topic_ma = time(NULL) + rel_topic_ma;
		}
	}
	else{
		topic_ma = 0;
	}
	printf("topic max-age : %d\n",rel_topic_ma);
	printf("topic abs max-age : %ld\n",topic_ma);
	/* to get max_age value*/
	
	printf("Parser status : %d", status);
	
	/* Unsupported content format for topic. */
	if (status)	{
		coap_attr_t* new_resource_attr = coap_find_attr(new_resource,(const unsigned char*) "ct", 2);
		
		if(new_resource_attr == NULL){
			printf("ct not found\n"); 
			coapFreeResource(new_resource);
			status=0; // malformed request indicator
		}
		
		else {
			int is_digit = 1;
			for(int i = 0; i < new_resource_attr->value.length;i++){
				if (!isdigit(new_resource_attr->value.s[i])){
					is_digit = 0;
					break;
				}
			}
			
			if(is_digit){
				int ct_value = atoi(new_resource_attr->value.s);
				if(ct_value < 0 || ct_value > 65535){ 
					status = 0;
					ct_value_valid = 0;	
				}
			}
			else { 
				status = 0;
				ct_value_valid = 0;	
			}		
		}	
		
		if ( !ct_value_valid ){
			response->hdr->code = COAP_RESPONSE_CODE(406);
			coapFreeResource(new_resource); 
			return;
		}	
	}	
	/* Unsupported content format for topic. */
	
	/* Topic already exists. */ 
	if (status){
		int found_resource = 0;
		RESOURCES_ITER(ctx->resources, r) {
			if(compareString(r->uri.s, new_resource->uri.s)){
				found_resource = 1;
				break;
			}
		}
		
		
		if(found_resource){
			updateTopicInfo(&topicDB, new_resource->uri.s, topic_ma);
			coapFreeResource(new_resource);
			response->hdr->code = COAP_RESPONSE_CODE(403); 
			return ;
		}
	}
	/* Topic already exists. */
	
	/* Successful Creation of the topic */
	if (status){		 	
			MQTTClient_subscribe(*global_client, new_resource->uri.s, QOS);
			coap_register_handler(new_resource, COAP_REQUEST_GET, hnd_get_topic);
			coap_register_handler(new_resource, COAP_REQUEST_POST, hnd_post_topic);
			coap_register_handler(new_resource, COAP_REQUEST_PUT, hnd_put_topic);
			coap_register_handler(new_resource, COAP_REQUEST_DELETE, hnd_delete_topic);
			new_resource->observable = 1;
			coap_add_resource(ctx, new_resource);
			coap_add_option(response, COAP_OPTION_LOCATION_PATH, new_resource->uri.length, new_resource->uri.s);
			addTopicWEC(&topicDB, new_resource->uri.s, new_resource->uri.length, topic_ma);
			response->hdr->code = COAP_RESPONSE_CODE(201) ;
			return ; 
	}
	/* Successful Creation of the topic */
	
	/* malformed request */
	if (!status){
		response->hdr->code = COAP_RESPONSE_CODE(400);
		return ; 
	} 
	/* malformed request */
}

static void hnd_delete_broker(coap_context_t *ctx,
             struct coap_resource_t *resource,
             const coap_endpoint_t *local_interface,
             coap_address_t *peer,
             coap_pdu_t *request,
             str *token,
             coap_pdu_t *response){
				 
	handle_sigint(0);
}
static void hnd_get_topic(coap_context_t *ctx, struct coap_resource_t *resource, 
              const coap_endpoint_t *local_interface, coap_address_t *peer, 
              coap_pdu_t *request, str *token, coap_pdu_t *response){
					
		unsigned char buf[3];
		int status = 0;
		/* to get max_age value and ct */
		coap_opt_t *option;
		coap_opt_iterator_t opt_iter; 
		coap_option_iterator_init(request, &opt_iter, COAP_OPT_ALL); 
		unsigned char* ct_opt_val;
		unsigned short ct_opt_len;
		
		while ((option = coap_option_next(&opt_iter))) {
		   if (opt_iter.type == COAP_OPTION_CONTENT_TYPE) { 
					ct_opt_val = coap_opt_value(option);
					ct_opt_len = coap_opt_length(option);
					break;
		   }
		}
		/* to get max_age value and ct*/
		
		/* check if topic has the same ct */
		if (coap_find_attr(resource,ct_opt_val, ct_opt_len) == NULL){ status = 0;}
		/* check if topic has the same ct */
		 
		TopicDataPtr temp = getTopic(&topicDB, resource->uri.s);
		if (temp == NULL || temp->data == NULL) {
			response->hdr->code 	= COAP_RESPONSE_CODE(404);
		}
		else {
			response->hdr->code 	= COAP_RESPONSE_CODE(205);
			if (coap_find_observer(resource, peer, token)) {
			/* FIXME: need to check for resource->dirty? */
				coap_add_option(response, COAP_OPTION_OBSERVE, coap_encode_var_bytes(buf, ctx->observe), buf);
			} 
			coap_add_option(response, COAP_OPTION_CONTENT_TYPE, coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);
			coap_add_option(response, COAP_OPTION_MAXAGE,coap_encode_var_bytes(buf, 20), buf); // mac-age in seconds, so 30 seconds (to mars. #pun)
			coap_add_data  (response, strlen(temp->data), temp->data);
		}
		// option order matters!

}

static void hnd_put_topic(coap_context_t *ctx ,
             struct coap_resource_t *resource ,
             const coap_endpoint_t *local_interface ,
             coap_address_t *peer ,
             coap_pdu_t *request,
             str *token ,
             coap_pdu_t *response){
				 
	size_t size;
    unsigned char *data; 
	(void)coap_get_data(request, &size, &data);
	int status = updateTopicData(&topicDB, resource->uri.s, 0, data, size);
	if (status){
		MQTTClient_message pubmsg = MQTTClient_message_initializer;
		MQTTClient_deliveryToken token;
		pubmsg.payload = data;
		pubmsg.payloadlen = size;
		pubmsg.qos = QOS;
		pubmsg.retained = 0;
		deliveredtoken = 0;
		MQTTClient_publishMessage(*global_client, resource->uri.s, &pubmsg, &token);
		printf("Waiting for publication of %s\n"
            "on topic %s for client with ClientID: %s\n",
        data, resource->uri.s, CLIENTID);
		resource->dirty = 1;
		/* this can be changed to coap_notify_observers, but coap_notify_observers is static. so either
		* create a similir function or change resource.c 
		*/
		coap_check_notify(ctx);
	}
	response->hdr->code = status ? COAP_RESPONSE_CODE(201) : COAP_RESPONSE_CODE(400);
}

static void hnd_delete_topic(coap_context_t *ctx ,
                struct coap_resource_t *resource ,
                const coap_endpoint_t *local_interface ,
                coap_address_t *peer ,
                coap_pdu_t *request ,
                str *token ,
                coap_pdu_t *response ){
	int counter = 0;
	char* deleted_sub_uri = malloc(sizeof(char) *(resource->uri.length+3));
	snprintf(deleted_sub_uri, (resource->uri.length+1)+1, "%s/", resource->uri.s);
	
	counter = deleteTopic(&topicDB, resource->uri.s);
	
	if (counter){
		MQTTClient_unsubscribe(*global_client, resource->uri.s);
		RESOURCES_DELETE(ctx->resources, resource);
		coapFreeResource(resource);
	
		RESOURCES_ITER(ctx->resources, r) {
			if (strstr(r->uri.s, deleted_sub_uri) != NULL){ 
				if (deleteTopic(&topicDB, r->uri.s)){
					MQTTClient_unsubscribe(*global_client, r->uri.s);
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
	payload_data = malloc(sizeof(int) * (counter_digit+2));
	snprintf(payload_data,counter_digit+1,"%d", counter);
	
	unsigned char buf[3];
	coap_add_option(response, COAP_OPTION_CONTENT_TYPE, coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);
	coap_add_data  (response, strlen(payload_data), payload_data);	
		
	free(payload_data);
	free(deleted_sub_uri);
}
                
static void hnd_post_topic(coap_context_t *ctx ,
                struct coap_resource_t *resource ,
                const coap_endpoint_t *local_interface ,
                coap_address_t *peer ,
                coap_pdu_t *request ,
                str *token ,
                coap_pdu_t *response ){
coap_resource_t *new_resource = NULL;
	
	/* parse payload */
	size_t size;
    unsigned char *data;
    unsigned char *data_safe;
	(void)coap_get_data(request, &size, &data);
	data_safe = coap_malloc(sizeof(char)*(size+2));
	snprintf(data_safe,size+1, "%s", data);
	//int status = 0;
	int status = parseLinkFormat(data_safe,resource, &new_resource);
	/* parse payload */
	
	/* to get max_age value and ct */
	coap_opt_t *option;
	coap_opt_iterator_t opt_iter; 
	coap_option_iterator_init(request, &opt_iter, COAP_OPT_ALL);
	unsigned char* max_age_val;
	unsigned short max_age_len;	 
	int rel_topic_ma;
	int topic_ma_status = 0;
	time_t topic_ma = 0;
	
	while ((option = coap_option_next(&opt_iter))) {
		// changing COAP_OPTION_MAXAGE into any coap Option will return the selected option type
	   if (opt_iter.type == COAP_OPTION_MAXAGE) { 
				topic_ma_status = 1;
				max_age_len = coap_opt_length(option);
				max_age_val = malloc(sizeof(char) * (max_age_len+2));
				snprintf(max_age_val, max_age_len +1, "%s", coap_opt_value(option));
				rel_topic_ma = atoi(max_age_val);
				free(max_age_val);
				break;
	   }
	}
	
	if(topic_ma_status){
		if(rel_topic_ma < 1) { // if max-age input is 0 or below
			topic_ma = 0;	// set topic_ma to 0, which means topic never expires
		}
		else{
			topic_ma = time(NULL) + rel_topic_ma;
		}
	}
	else{
		topic_ma = 0;
	}
	printf("topic max-age : %d\n",rel_topic_ma);
	printf("topic abs max-age : %ld\n",topic_ma);
	/* to get max_age value and ct*/
	
	printf("Parser status : %d", status);
	
	/* check whether resource contain ct */
	if (status)	{
		if(coap_find_attr(new_resource,(const unsigned char*) "ct", 2) == NULL){
			printf("ct not found\n"); 
			coapFreeResource(new_resource);
			status=0;
		}		
	}	
	/* check whether resource contain ct */
	 
	if (status){
		int found_resource = 0;
		RESOURCES_ITER(ctx->resources, r) {
			if(compareString(r->uri.s, new_resource->uri.s)){
				found_resource = 1;
				break;
			}
		}
		
		if(found_resource){
			updateTopicInfo(&topicDB, new_resource->uri.s, topic_ma);
			coapFreeResource(new_resource);
			response->hdr->code = COAP_RESPONSE_CODE(403);
		}
		else{		
			MQTTClient_subscribe(*global_client, new_resource->uri.s, QOS);
			coap_register_handler(new_resource, COAP_REQUEST_GET, hnd_get_topic);
			coap_register_handler(new_resource, COAP_REQUEST_POST, hnd_post_topic);
			coap_register_handler(new_resource, COAP_REQUEST_PUT, hnd_put_topic);
			coap_register_handler(new_resource, COAP_REQUEST_DELETE, hnd_delete_topic);
			new_resource->observable = 1;
			coap_add_resource(ctx, new_resource);
			coap_add_option(response, COAP_OPTION_LOCATION_PATH, new_resource->uri.length, new_resource->uri.s);
			addTopicWEC(&topicDB, new_resource->uri.s, new_resource->uri.length, topic_ma);
			response->hdr->code = COAP_RESPONSE_CODE(201) ;
		}
	}
	else{
		response->hdr->code = COAP_RESPONSE_CODE(400);
	}
	coap_free(data_safe);
}

