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
#include <limits.h> 
#include <coap.h> 

#define LL_DATABASE
#define LF_PARSER
#define LIBCOAP_MOD
#define GLOBAL_DATA
#define MQTT_CLIENT
#define COAP_RD
#define OTHER_FUNCTION
#define COAP_RESOURCE_CHECK_TIME 2

#ifdef COAP_RD

#define RD_ROOT_STR   ((unsigned char *)"rd")
#define RD_ROOT_SIZE  2

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

typedef struct rd_t {
  UT_hash_handle hh;      /**< hash handle (for internal use only) */
  coap_key_t key;         /**< the actual key bytes for this resource */

  size_t etag_len;        /**< actual length of @c etag */
  unsigned char etag[8];  /**< ETag for current description */

  str data;               /**< points to the resource description  */
} rd_t;

rd_t *resources = NULL;

static inline rd_t *
rd_new(void) {
  rd_t *rd;
  rd = (rd_t *)coap_malloc(sizeof(rd_t));
  if (rd)
    memset(rd, 0, sizeof(rd_t));

  return rd;
}

static inline void
rd_delete(rd_t *rd) {
  if (rd) {
    coap_free(rd->data.s);
    coap_free(rd);
  }
}

static void
hnd_get_resource(coap_context_t  *ctx ,
                 struct coap_resource_t *resource,
                 const coap_endpoint_t *local_interface ,
                 coap_address_t *peer ,
                 coap_pdu_t *request ,
                 str *token ,
                 coap_pdu_t *response) {
  rd_t *rd = NULL;
  unsigned char buf[3];

  HASH_FIND(hh, resources, resource->key, sizeof(coap_key_t), rd);

  response->hdr->code = COAP_RESPONSE_CODE(205);

  coap_add_option(response,
                  COAP_OPTION_CONTENT_TYPE,
                  coap_encode_var_bytes(buf,
                                        COAP_MEDIATYPE_APPLICATION_LINK_FORMAT),
                                        buf);

  if (rd && rd->etag_len)
    coap_add_option(response, COAP_OPTION_ETAG, rd->etag_len, rd->etag);

  if (rd && rd->data.s)
    coap_add_data(response, rd->data.length, rd->data.s);
}

static void
hnd_put_resource(coap_context_t  *ctx ,
                 struct coap_resource_t *resource ,
                 const coap_endpoint_t *local_interface ,
                 coap_address_t *peer ,
                 coap_pdu_t *request ,
                 str *token ,
                 coap_pdu_t *response) {
#if 1
  response->hdr->code = COAP_RESPONSE_CODE(501);
#else /* FIXME */
  coap_opt_iterator_t opt_iter;
  coap_opt_t *token, *etag;
  coap_pdu_t *response;
  size_t size = sizeof(coap_hdr_t);
  int type = (request->hdr->type == COAP_MESSAGE_CON)
    ? COAP_MESSAGE_ACK : COAP_MESSAGE_NON;
  rd_t *rd = NULL;
  unsigned char code;     /* result code */
  unsigned char *data;
  str tmp;

  HASH_FIND(hh, resources, resource->key, sizeof(coap_key_t), rd);
  if (rd) {
    /* found resource object, now check Etag */
    etag = coap_check_option(request, COAP_OPTION_ETAG, &opt_iter);
    if (!etag || (COAP_OPT_LENGTH(etag) != rd->etag_len)
        || memcmp(COAP_OPT_VALUE(etag), rd->etag, rd->etag_len) != 0) {

      if (coap_get_data(request, &tmp.length, &data)) {

        tmp.s = (unsigned char *)coap_malloc(tmp.length);
        if (!tmp.s) {
          debug("hnd_put_rd: cannot allocate storage for new rd\n");
          code = COAP_RESPONSE_CODE(503);
          goto finish;
        }

        coap_free(rd->data.s);
        rd->data.s = tmp.s;
        rd->data.length = tmp.length;
        memcpy(rd->data.s, data, rd->data.length);
      }
    }

    if (etag) {
      rd->etag_len = min(COAP_OPT_LENGTH(etag), sizeof(rd->etag));
      memcpy(rd->etag, COAP_OPT_VALUE(etag), rd->etag_len);
    }

    code = COAP_RESPONSE_CODE(204);
    /* FIXME: update lifetime */

    } else {

    code = COAP_RESPONSE_CODE(503);
  }

  finish:
  /* FIXME: do not create a new response but use the old one instead */
  response = coap_pdu_init(type, code, request->hdr->id, size);

  if (!response) {
    debug("cannot create response for message %d\n", request->hdr->id);
    return;
  }

  if (request->hdr->token_length)
    coap_add_token(response, request->hdr->token_length, request->hdr->token);

  if (coap_send(ctx, peer, response) == COAP_INVALID_TID) {
    debug("hnd_get_rd: cannot send response for message %d\n",
    request->hdr->id);
  }
  coap_delete_pdu(response);
#endif
}

static void
hnd_delete_resource(coap_context_t  *ctx,
                    struct coap_resource_t *resource,
                    const coap_endpoint_t *local_interface ,
                    coap_address_t *peer ,
                    coap_pdu_t *request ,
                    str *token ,
                    coap_pdu_t *response) {
  rd_t *rd = NULL;

  HASH_FIND(hh, resources, resource->key, sizeof(coap_key_t), rd);
  if (rd) {
    HASH_DELETE(hh, resources, rd);
    rd_delete(rd);
  }
  /* FIXME: link attributes for resource have been created dynamically
   * using coap_malloc() and must be released. */
  coap_delete_resource(ctx, resource->key);

  response->hdr->code = COAP_RESPONSE_CODE(202);
}

static void
hnd_get_rd(coap_context_t  *ctx ,
           struct coap_resource_t *resource ,
           const coap_endpoint_t *local_interface ,
           coap_address_t *peer ,
           coap_pdu_t *request ,
           str *token ,
           coap_pdu_t *response) {
  unsigned char buf[3];

  response->hdr->code = COAP_RESPONSE_CODE(205);

  coap_add_option(response,
                  COAP_OPTION_CONTENT_TYPE,
                  coap_encode_var_bytes(buf,
                                        COAP_MEDIATYPE_APPLICATION_LINK_FORMAT),
                                        buf);

  coap_add_option(response,
                  COAP_OPTION_MAXAGE,
                  coap_encode_var_bytes(buf, 0x2ffff), buf);
}

static int
parse_param(unsigned char *search,
            size_t search_len,
            unsigned char *data,
            size_t data_len,
            str *result) {

  if (result)
    memset(result, 0, sizeof(str));

  if (!search_len)
    return 0;

  while (search_len <= data_len) {

    /* handle parameter if found */
    if (memcmp(search, data, search_len) == 0) {
      data += search_len;
      data_len -= search_len;

      /* key is only valid if we are at end of string or delimiter follows */
      if (!data_len || *data == '=' || *data == '&') {
        while (data_len && *data != '=') {
          ++data; --data_len;
        }

        if (data_len > 1 && result) {
          /* value begins after '=' */

          result->s = ++data;
          while (--data_len && *data != '&') {
            ++data; result->length++;
          }
        }

        return 1;
      }
    }

    /* otherwise proceed to next */
    while (--data_len && *data++ != '&')
      ;
  }

  return 0;
}

static void
add_source_address(struct coap_resource_t *resource,
                   coap_address_t *peer) {
#define BUFSIZE 64
  char *buf;
  size_t n = 1;

  buf = (char *)coap_malloc(BUFSIZE);
  if (!buf)
    return;

  buf[0] = '"';

  switch(peer->addr.sa.sa_family) {

  case AF_INET:
    /* FIXME */
    break;

  case AF_INET6:
    n += snprintf(buf + n, BUFSIZE - n,
      "[%02x%02x:%02x%02x:%02x%02x:%02x%02x" \
      ":%02x%02x:%02x%02x:%02x%02x:%02x%02x]",
      peer->addr.sin6.sin6_addr.s6_addr[0],
      peer->addr.sin6.sin6_addr.s6_addr[1],
      peer->addr.sin6.sin6_addr.s6_addr[2],
      peer->addr.sin6.sin6_addr.s6_addr[3],
      peer->addr.sin6.sin6_addr.s6_addr[4],
      peer->addr.sin6.sin6_addr.s6_addr[5],
      peer->addr.sin6.sin6_addr.s6_addr[6],
      peer->addr.sin6.sin6_addr.s6_addr[7],
      peer->addr.sin6.sin6_addr.s6_addr[8],
      peer->addr.sin6.sin6_addr.s6_addr[9],
      peer->addr.sin6.sin6_addr.s6_addr[10],
      peer->addr.sin6.sin6_addr.s6_addr[11],
      peer->addr.sin6.sin6_addr.s6_addr[12],
      peer->addr.sin6.sin6_addr.s6_addr[13],
      peer->addr.sin6.sin6_addr.s6_addr[14],
      peer->addr.sin6.sin6_addr.s6_addr[15]);

    if (peer->addr.sin6.sin6_port != htons(COAP_DEFAULT_PORT)) {
      n +=
      snprintf(buf + n, BUFSIZE - n, ":%d", peer->addr.sin6.sin6_port);
    }
    break;
    default:
    ;
  }

  if (n < BUFSIZE)
    buf[n++] = '"';

  coap_add_attr(resource,
                (unsigned char *)"A",
                1,
                (unsigned char *)buf,
                n,
                COAP_ATTR_FLAGS_RELEASE_VALUE);
#undef BUFSIZE
}

static rd_t *
make_rd(coap_address_t *peer , coap_pdu_t *pdu) {
  rd_t *rd;
  unsigned char *data;
  coap_opt_iterator_t opt_iter;
  coap_opt_t *etag;

  rd = rd_new();

  if (!rd) {
    debug("hnd_get_rd: cannot allocate storage for rd\n");
    return NULL;
  }

  if (coap_get_data(pdu, &rd->data.length, &data)) {
    rd->data.s = (unsigned char *)coap_malloc(rd->data.length);
    if (!rd->data.s) {
      debug("hnd_get_rd: cannot allocate storage for rd->data\n");
      rd_delete(rd);
      return NULL;
    }
    memcpy(rd->data.s, data, rd->data.length);
  }

  etag = coap_check_option(pdu, COAP_OPTION_ETAG, &opt_iter);
  if (etag) {
    rd->etag_len = min(COAP_OPT_LENGTH(etag), sizeof(rd->etag));
    memcpy(rd->etag, COAP_OPT_VALUE(etag), rd->etag_len);
  }

  return rd;
}

static void
hnd_post_rd(coap_context_t  *ctx,
            struct coap_resource_t *resource ,
            const coap_endpoint_t *local_interface ,
            coap_address_t *peer,
            coap_pdu_t *request,
            str *token ,
            coap_pdu_t *response) {
  coap_resource_t *r;
  coap_opt_iterator_t opt_iter;
  coap_opt_t *query;
#define LOCSIZE 68
  unsigned char *loc;
  size_t loc_size;
  str h = {0, NULL}, ins = {0, NULL}, rt = {0, NULL}, lt = {0, NULL}; /* store query parameters */
  unsigned char *buf;

  loc = (unsigned char *)coap_malloc(LOCSIZE);
  if (!loc) {
    response->hdr->code = COAP_RESPONSE_CODE(500);
    return;
  }
  memcpy(loc, RD_ROOT_STR, RD_ROOT_SIZE);

  loc_size = RD_ROOT_SIZE;
  loc[loc_size++] = '/';

  /* store query parameters for later use */
  query = coap_check_option(request, COAP_OPTION_URI_QUERY, &opt_iter);
  if (query) {
    parse_param((unsigned char *)"h", 1,
    COAP_OPT_VALUE(query), COAP_OPT_LENGTH(query), &h);
    parse_param((unsigned char *)"ins", 3,
    COAP_OPT_VALUE(query), COAP_OPT_LENGTH(query), &ins);
    parse_param((unsigned char *)"lt", 2,
    COAP_OPT_VALUE(query), COAP_OPT_LENGTH(query), &lt);
    parse_param((unsigned char *)"rt", 2,
    COAP_OPT_VALUE(query), COAP_OPT_LENGTH(query), &rt);
  }

  if (h.length) {   /* client has specified a node name */
    memcpy(loc + loc_size, h.s, min(h.length, LOCSIZE - loc_size - 1));
    loc_size += min(h.length, LOCSIZE - loc_size - 1);

    if (ins.length && loc_size > 1) {
      loc[loc_size++] = '-';
      memcpy((char *)(loc + loc_size),
      ins.s, min(ins.length, LOCSIZE - loc_size - 1));
      loc_size += min(ins.length, LOCSIZE - loc_size - 1);
    }

  } else {      /* generate node identifier */
    loc_size +=
      snprintf((char *)(loc + loc_size), LOCSIZE - loc_size - 1,
               "%x", request->hdr->id);

    if (loc_size > 1) {
      if (ins.length) {
        loc[loc_size++] = '-';
        memcpy((char *)(loc + loc_size),
                ins.s,
                min(ins.length, LOCSIZE - loc_size - 1));
        loc_size += min(ins.length, LOCSIZE - loc_size - 1);
      } else {
        coap_tick_t now;
        coap_ticks(&now);

        loc_size += snprintf((char *)(loc + loc_size),
                             LOCSIZE - loc_size - 1,
                             "-%x",
                             (unsigned int)(now & (unsigned int)-1));
      }
    }
  }

  /* TODO:
   *   - use lt to check expiration
   */

  r = coap_resource_init(loc, loc_size, COAP_RESOURCE_FLAGS_RELEASE_URI);
  coap_register_handler(r, COAP_REQUEST_GET, hnd_get_resource);
  coap_register_handler(r, COAP_REQUEST_PUT, hnd_put_resource);
  coap_register_handler(r, COAP_REQUEST_DELETE, hnd_delete_resource);

  if (ins.s) {
    buf = (unsigned char *)coap_malloc(ins.length + 2);
    if (buf) {
      /* add missing quotes */
      buf[0] = '"';
      memcpy(buf + 1, ins.s, ins.length);
      buf[ins.length + 1] = '"';
      coap_add_attr(r,
                    (unsigned char *)"ins",
                    3,
                    buf,
                    ins.length + 2,
                    COAP_ATTR_FLAGS_RELEASE_VALUE);
    }
  }

  if (rt.s) {
    buf = (unsigned char *)coap_malloc(rt.length + 2);
    if (buf) {
      /* add missing quotes */
      buf[0] = '"';
      memcpy(buf + 1, rt.s, rt.length);
      buf[rt.length + 1] = '"';
      coap_add_attr(r,
                    (unsigned char *)"rt",
                    2,
                    buf,
                    rt.length + 2,COAP_ATTR_FLAGS_RELEASE_VALUE);
    }
  }

  add_source_address(r, peer);

  {
    rd_t *rd;
    rd = make_rd(peer, request);
    if (rd) {
      coap_hash_path(loc, loc_size, rd->key);
      HASH_ADD(hh, resources, key, sizeof(coap_key_t), rd);
    } else {
      /* FIXME: send error response and delete r */
    }
  }

  coap_add_resource(ctx, r);


  /* create response */

  response->hdr->code = COAP_RESPONSE_CODE(201);

  { /* split path into segments and add Location-Path options */
    unsigned char _b[LOCSIZE];
    unsigned char *b = _b;
    size_t buflen = sizeof(_b);
    int nseg;

    nseg = coap_split_path(loc, loc_size, b, &buflen);
    while (nseg--) {
      coap_add_option(response,
                      COAP_OPTION_LOCATION_PATH,
                      COAP_OPT_LENGTH(b),
                      COAP_OPT_VALUE(b));
      b += COAP_OPT_SIZE(b);
    }
  }
}

static void
init_rd_resources(coap_context_t *ctx) {
  coap_resource_t *r;

  r = coap_resource_init(RD_ROOT_STR, RD_ROOT_SIZE, 0);
  coap_register_handler(r, COAP_REQUEST_GET, hnd_get_rd);
  coap_register_handler(r, COAP_REQUEST_POST, hnd_post_rd);

  coap_add_attr(r, (unsigned char *)"ct", 2, (unsigned char *)"40", 2, 0);
  coap_add_attr(r, (unsigned char *)"rt", 2, (unsigned char *)"\"core.rd\"", 9, 0);
  coap_add_attr(r, (unsigned char *)"ins", 2, (unsigned char *)"\"default\"", 9, 0);

  coap_add_resource(ctx, r);

}
#endif 
 
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
    pthread_rwlock_t  node_lock;
    pthread_rwlock_t  data_lock;
}; /* end structure topicData */

typedef struct topicData TopicData; /* synonym for struct topicData */
typedef TopicData* TopicDataPtr; /* synonym for TopicData* */

/* prototypes */
TopicDataPtr getTopic( TopicDataPtr *sPtr, char* path);
TopicDataPtr cloneTopic( TopicDataPtr *sPtr, char* path);
int			freeTopic(TopicDataPtr topic);
	
int			getTopicPath(TopicDataPtr topic, char ** path);
int			getTopicData(TopicDataPtr topic, char ** data);
time_t 		getTopicMA(TopicDataPtr topic);
time_t 		getTopicDataMA(TopicDataPtr topic);
int 		topicNodeRLock(TopicDataPtr topic);
int 		topicNodeWLock(TopicDataPtr topic);
int 		topicNodeUnlock(TopicDataPtr topic);
int 		topicDataRLock(TopicDataPtr topic);
int 		topicDataWLock(TopicDataPtr topic);
int 		topicDataUnlock(TopicDataPtr topic);
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
int 		deleteTopicData( TopicDataPtr *sPtr, char* path );					
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
TopicDataPtr getTopic( TopicDataPtr *sPtr, char* path){ 
	
    TopicDataPtr previousPtr = NULL; /* pointer to previous node in list */
    TopicDataPtr currentPtr = NULL;  /* pointer to current node in list */
    TopicDataPtr tempPtr = NULL;     /* temporary node pointer */
	
	if (path == NULL || DBEmpty(*sPtr)) { 
		return NULL;
	} 
	else { 
		pthread_rwlock_rdlock(&(( *sPtr )->node_lock));
	}
	
    if ( compareString(( *sPtr )->path,path)) { 
        tempPtr = *sPtr;
        pthread_rwlock_unlock(&(( *sPtr )->node_lock)); 
        return tempPtr;
    } 
    
    else { 
        previousPtr = *sPtr;
        currentPtr = ( *sPtr )->nextPtr;
		pthread_rwlock_unlock(&(( *sPtr )->node_lock));
				
        /* loop to find the correct location in the list */
        while ( currentPtr != NULL ) { 
			pthread_rwlock_rdlock(&(currentPtr->node_lock));
			if (compareString(currentPtr->path,path)){
				break;
			}
			
            previousPtr = currentPtr;         /* walk to ...   */
            currentPtr = currentPtr->nextPtr; /* ... next node */  
            pthread_rwlock_unlock(&(previousPtr->node_lock));
        } /* end while */

        /* delete node at currentPtr */
        if ( currentPtr != NULL ) { 
            tempPtr = currentPtr;
            pthread_rwlock_unlock(&(currentPtr->node_lock));
            return tempPtr;
        } /* end if */
		else{
			return NULL;
		}
    } /* end else */
} 


TopicDataPtr 	cloneTopic(TopicDataPtr *sPtr, char* path){ 
	
    TopicDataPtr previousPtr = NULL; /* pointer to previous node in list */
    TopicDataPtr currentPtr = NULL;  /* pointer to current node in list */
    TopicDataPtr tempPtr = NULL;     /* temporary node pointer */
	
	if (path == NULL || DBEmpty(*sPtr)) { 
		return NULL;
	} 
	else { 
		pthread_rwlock_rdlock(&(( *sPtr )->node_lock));
	}
	
    if ( compareString(( *sPtr )->path,path)) { 
        tempPtr = *sPtr;
		
		TopicDataPtr newPtr = NULL;      /* pointer to new node */
		newPtr = malloc( sizeof( TopicData ) ); /* create node on heap */
			
		if ( newPtr != NULL ) { /* is space available */
			int path_size = strlen(tempPtr->path);
			char* temp_path = malloc(sizeof(char) * (path_size + 2));		
			snprintf(temp_path,sizeof(char) * (path_size+1), "%s", tempPtr->path);
			
			char* temp_data;
			if (tempPtr->data != NULL){
				int data_size = strlen(tempPtr->data);
				temp_data = malloc(sizeof(char) * (data_size + 2));		
				snprintf(temp_data,sizeof(char) * (data_size+1), "%s", tempPtr->data);
			}
			else {
				temp_data = NULL;
			}
			
			newPtr->path = temp_path; /* place value in node */
			newPtr->data = temp_data; /* place value in node */
			newPtr->data_ma = tempPtr->data_ma; /* place value in node */
			newPtr->topic_ma = tempPtr->topic_ma; /* place value in node */
			newPtr->nextPtr = NULL; /* node does not link to another node */
			pthread_rwlock_init(&(newPtr->node_lock), NULL);
			pthread_rwlock_init(&(newPtr->data_lock), NULL);
			
			pthread_rwlock_unlock(&(( *sPtr )->node_lock));
			return newPtr;
		}
		
		else {
			free(newPtr); /* not needed */
			pthread_rwlock_unlock(&(( *sPtr )->node_lock));
			return NULL;
		}
    } 
    
    else { 
        previousPtr = *sPtr;
        currentPtr = ( *sPtr )->nextPtr;
		pthread_rwlock_unlock(&(( *sPtr )->node_lock));
				
        /* loop to find the correct location in the list */
        while ( currentPtr != NULL ) { 
			pthread_rwlock_rdlock(&(currentPtr->node_lock));
			if (compareString(currentPtr->path,path)){
				break;
			}
			
            previousPtr = currentPtr;         /* walk to ...   */
            currentPtr = currentPtr->nextPtr; /* ... next node */  
            pthread_rwlock_unlock(&(previousPtr->node_lock));
        } /* end while */

        /* delete node at currentPtr */
        if ( currentPtr != NULL ) { 
            tempPtr = currentPtr;
            
            TopicDataPtr newPtr = NULL;      /* pointer to new node */
			newPtr = malloc( sizeof( TopicData ) ); /* create node on heap */
				
			if ( newPtr != NULL ) { /* is space available */
				int path_size = strlen(tempPtr->path);
				char* temp_path = malloc(sizeof(char) * (path_size + 2));		
				snprintf(temp_path,sizeof(char) * (path_size+1), "%s", tempPtr->path);
				
				char* temp_data;
				if (tempPtr->data != NULL){
					int data_size = strlen(tempPtr->data);
					temp_data = malloc(sizeof(char) * (data_size + 2));		
					snprintf(temp_data,sizeof(char) * (data_size+1), "%s", tempPtr->data);
				}
				else {
					temp_data = NULL;
				}
				
				newPtr->path = temp_path; /* place value in node */
				newPtr->data = temp_data; /* place value in node */
				newPtr->data_ma = tempPtr->data_ma; /* place value in node */
				newPtr->topic_ma = tempPtr->topic_ma; /* place value in node */
				newPtr->nextPtr = NULL; /* node does not link to another node */
				pthread_rwlock_init(&(newPtr->node_lock), NULL);
				pthread_rwlock_init(&(newPtr->data_lock), NULL);
				
				pthread_rwlock_unlock(&(currentPtr->node_lock));
				return newPtr;
			}
			
			else {
				free(newPtr); /* not needed */
				pthread_rwlock_unlock(&(currentPtr->node_lock));
				return NULL;
			}
        } /* end if */
		else{
			return NULL;
		}
    } /* end else */
} 

int		freeTopic(TopicDataPtr topic){
	
	if (topic != NULL){
		free( topic->data );
		free( topic->path );
		pthread_rwlock_destroy(&(topic->node_lock));
		pthread_rwlock_destroy(&(topic->data_lock));
		free( topic );
		return 1;
	}
	else {
		return 0;
	}
}
int 		getTopicPath(TopicDataPtr topic, char ** path ){
	
	if(topic != NULL) {
		topicNodeRLock(topic);
		topicDataRLock(topic);
		
		int path_size = strlen(topic->path);
		*path = malloc(sizeof(char) * (path_size+2));
		snprintf(*path,sizeof(char) * (path_size+1), "%s", topic->path);
		
		
		topicDataUnlock(topic);
		topicNodeUnlock(topic);
		
		return 1;
	}
	else {
		return 0;
	}
	
}
int 		getTopicData(TopicDataPtr topic, char ** data){
	
	if(topic != NULL) {		
		topicNodeRLock(topic);
		topicDataRLock(topic);
		
		int data_size = strlen(topic->data);
		*data = malloc(sizeof(char) * (data_size+2));
		snprintf(*data,sizeof(char) * (data_size+1), "%s", topic->data);
		
		topicDataUnlock(topic);
		topicNodeUnlock(topic);
		
		return 1;
	}
	else {
		return 0;
	}
}
time_t 		getTopicMA(TopicDataPtr topic){

	if(topic != NULL) {		
		topicNodeRLock(topic);
		topicDataRLock(topic);
		
		time_t topic_ma = topic->topic_ma;
		
		topicDataUnlock(topic);
		topicNodeUnlock(topic);
		
		return topic_ma;
	}
	else {
		return -1;
	}
	
}
time_t 		getTopicDataMA(TopicDataPtr topic){
	
	if(topic != NULL) {		
		topicNodeRLock(topic);
		topicDataRLock(topic);
		
		time_t data_ma = topic->data_ma;
		
		topicDataUnlock(topic);
		topicNodeUnlock(topic);
		
		return data_ma;
	}
	else {
		return -1;
	}
}

int 		topicNodeRLock(TopicDataPtr topic){	
	return pthread_rwlock_rdlock(&(topic->node_lock));
}

int 		topicNodeWLock(TopicDataPtr topic){
	return pthread_rwlock_wrlock(&(topic->node_lock));
}

int 		topicNodeUnlock(TopicDataPtr topic){
	return pthread_rwlock_unlock(&(topic->node_lock));
}

int 		topicDataRLock(TopicDataPtr topic){
	return pthread_rwlock_rdlock(&(topic->data_lock));
}

int 		topicDataWLock(TopicDataPtr topic){
	return pthread_rwlock_wrlock(&(topic->data_lock));
}

int 		topicDataUnlock(TopicDataPtr topic){
	return pthread_rwlock_unlock(&(topic->data_lock));
}

int setTopic( TopicDataPtr *sPtr, 
					char* path, char* data, size_t data_size, 
					time_t topic_ma, time_t data_ma)
{ 
	/* need to change getTopic() this, can lead to thread-unsafe */
	TopicDataPtr topic = getTopic(sPtr, path);
	if (topic != NULL){
		topicNodeRLock(topic);
		topicDataWLock(topic); 
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
		topicDataUnlock(topic);
		topicNodeUnlock(topic);
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
			pthread_rwlock_init(&(newPtr->node_lock), NULL);
			pthread_rwlock_init(&(newPtr->data_lock), NULL);
			
			/*int lock_init = pthread_rwlock_init(&(newPtr->node_lock), NULL);
			printf("Lock init : %s\n", lock_init == 0? "success" : "failed"); */
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
			
			pthread_rwlock_rdlock(&(currentPtr->node_lock));
			
            previousPtr = currentPtr;          /* walk to ...   */
            currentPtr = currentPtr->nextPtr;  /* ... next node */
            
            pthread_rwlock_unlock(&(previousPtr->node_lock));
        } /* end while */
		
		/* we should add read-write lock for linked-list head */
        /* insert new node at beginning of list */
        if ( previousPtr == NULL ) { 
			
            newPtr->nextPtr = *sPtr;
            *sPtr = newPtr;
        } /* end if */
        else { /* insert new node between previousPtr and currentPtr */
			pthread_rwlock_wrlock(&(previousPtr->node_lock));
            previousPtr->nextPtr = newPtr;
            newPtr->nextPtr = currentPtr;
            pthread_rwlock_unlock(&(previousPtr->node_lock));
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

	if (path == NULL || DBEmpty(*sPtr)) { 
		return 0;
	}
	else { 
		pthread_rwlock_rdlock(&(( *sPtr )->node_lock));
	}
	
    /* delete first node */
    if ( compareString(( *sPtr )->path,path)) { 
		pthread_rwlock_unlock(&(( *sPtr )->node_lock));
		
		pthread_rwlock_wrlock(&(( *sPtr )->node_lock));
        tempPtr = *sPtr; /* hold onto node being removed */
        *sPtr = ( *sPtr )->nextPtr; /* de-thread the node */
        pthread_rwlock_unlock(&(tempPtr->node_lock));
        
        free( tempPtr->data );
        free( tempPtr->path );
        pthread_rwlock_destroy(&(tempPtr->node_lock));
        pthread_rwlock_destroy(&(tempPtr->data_lock));
        free( tempPtr ); /* free the de-threaded node */
        return 1;
    } /* end if */
    else { 
        previousPtr = *sPtr;
        currentPtr = ( *sPtr )->nextPtr;
		pthread_rwlock_unlock(&(( *sPtr )->node_lock));
		
        /* loop to find the correct location in the list */
        while ( currentPtr != NULL ) { 
			pthread_rwlock_rdlock(&(currentPtr->node_lock)); 
			if (compareString(currentPtr->path,path)){
				break;
			}
            previousPtr = currentPtr;         /* walk to ...   */
            currentPtr = currentPtr->nextPtr; /* ... next node */
            pthread_rwlock_unlock(&(previousPtr->node_lock));  
        } /* end while */

        /* delete node at currentPtr */
        if ( currentPtr != NULL ) { 
			
            tempPtr = currentPtr;
			pthread_rwlock_unlock(&(currentPtr->node_lock));
			
			pthread_rwlock_wrlock(&(previousPtr->node_lock));
            pthread_rwlock_wrlock(&(currentPtr->node_lock));
            
            previousPtr->nextPtr = currentPtr->nextPtr;
            
            pthread_rwlock_unlock(&(currentPtr->node_lock));
            pthread_rwlock_unlock(&(previousPtr->node_lock));
            free( tempPtr->data );
            free( tempPtr->path );
            pthread_rwlock_destroy(&(tempPtr->node_lock));
			pthread_rwlock_destroy(&(tempPtr->data_lock));
            free( tempPtr );
            return 1;
        } /* end if */
		else{
			return 0;
		}
    } /* end else */
} /* end function delete */

int deleteTopicData( TopicDataPtr *sPtr, char* path)
{ 
	TopicDataPtr topic = getTopic(sPtr, path);
	if (topic != NULL){
		topicNodeRLock(topic);
		topicDataWLock(topic); 
		free( topic->data );
		topic->data_ma		= 0;
		topic->data			= NULL;
		topicDataUnlock(topic);
		topicNodeUnlock(topic); 
		return 1;
	}
	
    return 0;

} 

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
			topicNodeRLock(currentPtr);
			topicDataRLock(currentPtr);
            
            printf( "%s %s %d %d --> ",currentPtr->path, currentPtr->data,
            (int)currentPtr->topic_ma, (int)currentPtr->data_ma);            
            TopicDataPtr previousPtr = currentPtr;
            currentPtr = currentPtr->nextPtr;   
            
            topicDataUnlock(previousPtr);
            topicNodeUnlock(previousPtr);
        } /* end while */

        printf( "NULL\n\n" );
    } /* end else */

} /* end function printList */

/* Clean the list */
void cleanDB( TopicDataPtr *sPtr ){ 
 
	while(!DBEmpty(*sPtr)){ 
		TopicDataPtr tempPtr = NULL;     /* temporary node pointer */
        tempPtr = *sPtr; /* hold onto node being removed */
        
		topicNodeWLock(tempPtr);
        *sPtr = ( *sPtr )->nextPtr; /* de-thread the node */
        topicNodeUnlock(tempPtr); 
        
        free( tempPtr->data );
        free( tempPtr->path );
        pthread_rwlock_destroy(&(tempPtr->node_lock));
        pthread_rwlock_destroy(&(tempPtr->data_lock));
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

int parseOptionURIQuery(char* option_value, unsigned short option_length, char** query_name, char** query_value){
	char* temp_uri_query = malloc(sizeof(char) * (option_length+2));
	snprintf(temp_uri_query, option_length+1, "%s", option_value);
	
	int name_size, value_size;
	int upper_status = calOptionSize(temp_uri_query,&name_size, &value_size);
	if (upper_status && name_size > 0 && value_size > 0){
		*query_name = malloc(sizeof(char) * (name_size+1));
		*query_value = malloc(sizeof(char) * (value_size+1));
		int status = parseOption(temp_uri_query, *query_name, *query_value);
		if(status){
			free(temp_uri_query);
			return 1;
		}
		
		else{
			free(temp_uri_query);
			return 0;
		}
	}
	
	else {
		free(temp_uri_query);
		return -1;
	}
}

void dynamicConcatenate(char **str, char *str2) {
    char *tmp = NULL;

    // Reset *str
    if ( *str != NULL && str2 == NULL ) {
        free(*str);
        *str = NULL;
        return;
    }

	if ( str2 == NULL ) {
		printf("Error Source Empty\n");
        return;
    }

    // Initial copy
    if (*str == NULL) {
        *str = malloc( (strlen(str2)+1) * sizeof(char) );
        strcpy(*str, str2);
    }
    else { // Append
        tmp = malloc ( (strlen(*str)+1 )* sizeof(char) );
        strcpy(tmp, *str);
        *str = (char *) realloc((*str) ,  (strlen(*str)+strlen(str2)+1) * sizeof(char) );
        sprintf(*str, "%s%s", tmp, str2);
        free(tmp);
    }

} 
 
int calculateResourceLF(coap_resource_t* resource){
	if(resource != NULL){
		int lf_size = 0;
		/* </ uri > . added additional 3 char of <,/, and > */ 
		lf_size += resource->uri.length + 3;
		 
		coap_attr_t *attr; 
		LL_FOREACH(resource->link_attr, attr) {
			debug("Attr of %s with value of %s\n", attr->name.s, attr->value.s);
			/* ; name = value . added 2 additional char of ; and = */ 
			lf_size += (attr->name.length) + (attr->value.length) + 2;
		}
		if (resource->observable){
			/* if observable add 4 char of ";obs" */
			lf_size += 4;
		}
		return lf_size;
	}
}

/* Link Format Parser ends here */
#endif

#ifdef GLOBAL_DATA

	coap_context_t**  	global_ctx;
	MQTTClient* 		global_client;
	TopicDataPtr 		topicDB = NULL; /* initially there are no nodes */
	char 				broker_path[8] = "ps";
	
	time_t 				earliest_topic_max_age = LONG_MAX;
	time_t 				earliest_data_max_age = LONG_MAX;
		
	static int quit = 0;
	static void	handleSIGINT(int signum) {
		quit = 1;
		
		RESOURCES_ITER((*global_ctx)->resources, r) {
			if(!compareString(r->uri.s, broker_path)){
				deleteTopic(&topicDB, r->uri.s);
				MQTTClient_unsubscribe(*global_client, r->uri.s);
				RESOURCES_DELETE((*global_ctx)->resources, r);
				coapFreeResource(r);
			}
		}	 
		coap_free_context((*global_ctx));  
		MQTTClient_disconnect((*global_client), 10000);
		MQTTClient_destroy(global_client);
		exit(0);
	}
#endif

#ifdef OTHER_FUNCTION

void topicDataMAMonitor( TopicDataPtr currentPtr ){
	
	time_t master_time = time(NULL);
	/* if list is empty */
	if ( currentPtr == NULL ) {
		debug( "List is empty.\n\n" );
	} /* end if */
	else if (earliest_data_max_age < master_time) {  
			/* while not the end of the list */
			
		time_t next_earliest_data_ma = LONG_MAX;
		while ( currentPtr != NULL ) { 
			topicNodeRLock(currentPtr);
			TopicDataPtr nextPtr = currentPtr->nextPtr;
			
			topicDataRLock(currentPtr);
			if(currentPtr->data_ma != 0 && currentPtr->data_ma != earliest_data_max_age && currentPtr->data_ma < next_earliest_data_ma){
				next_earliest_data_ma = currentPtr->data_ma;
			}
			
			time_t currrent_time = time(NULL);
									
			debug( "%s\t\t%ld\t%ld | %s\n ",currentPtr->path, currentPtr->data_ma, currrent_time, currentPtr->data_ma < currrent_time && currentPtr->data_ma != 0? "Data Expired. Deleting..." : "Data Valid");
			
			if(currentPtr->data_ma < currrent_time && currentPtr->data_ma != 0){
				topicDataUnlock(currentPtr);				
				topicNodeUnlock(currentPtr);
				deleteTopicData(&topicDB, currentPtr->path);
			}
			else {				
				topicDataUnlock(currentPtr);
				topicNodeUnlock(currentPtr);
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

void topicMAMonitor( TopicDataPtr currentPtr ){
	
	time_t master_time = time(NULL);
	/* if list is empty */
	if ( currentPtr == NULL ) {
		debug( "List is empty.\n\n" );
	} /* end if */
	else if (earliest_topic_max_age < master_time) {  
			/* while not the end of the list */
			
		time_t next_earliest_topic_ma = LONG_MAX;
		while ( currentPtr != NULL ) { 
			topicNodeRLock(currentPtr);
			TopicDataPtr nextPtr = currentPtr->nextPtr;
			
			topicDataRLock(currentPtr);
			if(currentPtr->topic_ma != 0 && currentPtr->topic_ma != earliest_topic_max_age && currentPtr->topic_ma < next_earliest_topic_ma){
				next_earliest_topic_ma = currentPtr->topic_ma;
			}
			
			time_t currrent_time = time(NULL);
			
			debug( "%s\t\t%ld\t%ld | %s\n ",currentPtr->path, currentPtr->topic_ma, currrent_time, currentPtr->topic_ma < currrent_time && currentPtr->topic_ma != 0? "Topic Expired. Deleting..." : "Topic Valid");
						
			if (currentPtr->topic_ma < currrent_time && currentPtr->topic_ma != 0){
				char* deleted_uri_topic = malloc(sizeof(char) *(strlen(currentPtr->path)+2));
				sprintf(deleted_uri_topic, "%s", currentPtr->path);
				RESOURCES_ITER((*global_ctx)->resources, r) {
					if (compareString(r->uri.s, deleted_uri_topic)){
						topicDataUnlock(currentPtr);
						topicNodeUnlock(currentPtr);
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
			else {
				
				topicDataUnlock(currentPtr);
				topicNodeUnlock(currentPtr);
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
#endif

#ifdef MQTT_CLIENT

	#define CLIENTID	"CoAPBroker"
	#define QOS         1
	#define TIMEOUT     10000L
	
	volatile MQTTClient_deliveryToken deliveredtoken;
	
	void delivered(void *context, MQTTClient_deliveryToken dt)
	{
		debug("Message with token value %d delivery confirmed\n", dt);
		deliveredtoken = dt;
	}

	int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
	{
		int i;
		char* payloadptr;
		
		RESOURCES_ITER(((*global_ctx))->resources, r) {
			if(compareString(r->uri.s, topicName)){
				TopicDataPtr 	temp_data = cloneTopic(&topicDB,r->uri.s);
				char*			temp_payload = malloc(sizeof(char)*(message->payloadlen + 2));
				snprintf(temp_payload, (message->payloadlen)+1, "%s", (char*)message->payload);
				debug("Data from DB : %s\n", temp_data->data);
				debug("Data from MQTT : %s\n", temp_payload);
				if(!compareString(temp_data->data,temp_payload)){
					updateTopicData(&topicDB,topicName,0,message->payload,message->payloadlen);
					r->dirty = 1; 
					coap_check_notify((*global_ctx));
				}
				free(temp_payload);
				freeTopic(temp_data);
				
				MQTTClient_freeMessage(&message);
				MQTTClient_free(topicName);
				return 1;
			}
		}
		
		MQTTClient_freeMessage(&message);
		MQTTClient_free(topicName);
		return 0;
	}

	void connlost(void *context, char *cause)
	{
		debug("\nConnection lost cause: %s\n", cause);
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
             
int main(int argc, char* argv[]){
	
	coap_context_t*  	ctx;
	coap_address_t   	serv_addr;
	coap_resource_t* 	broker_resource;
	fd_set         		readfds;
	global_ctx			= &ctx;
	
	/* MQTT Client Init */
	
	char mqtt_address[]    				= "tcp://localhost:1883";
	MQTTClient 							client;
    global_client 						= &client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc 								= 0 ;

    MQTTClient_create			(&client, mqtt_address, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession 		= 1;
    MQTTClient_setCallbacks		(client, NULL, connlost, msgarrvd, delivered);

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);       
    }
	/* MQTT Client Init */
	
	/* turn on debug an printf() */
	coap_log_t log_level 	= LOG_DEBUG;
	coap_set_log_level		(log_level);
	/* turn on debug an printf() */
		
	/* Prepare the CoAP server socket */ 
	coap_address_init					(&serv_addr);
	serv_addr.addr.sin.sin_family		= AF_INET6;
	serv_addr.addr.sin.sin_addr.s_addr	= INADDR_ANY;
	serv_addr.addr.sin.sin_port			= htons(5683); //default port
	ctx									= coap_new_context(&serv_addr);
	if (!ctx) {
		exit(EXIT_FAILURE);
	}
	/* Prepare the CoAP server socket */ 	
	
	#ifdef COAP_RD
	init_rd_resources(ctx);
	#endif
	
	/* Initialize the observable resource */
	broker_resource 		= coap_resource_init(broker_path, strlen(broker_path), 0);
	coap_register_handler	(broker_resource, COAP_REQUEST_GET, hnd_get_broker);
	coap_register_handler	(broker_resource, COAP_REQUEST_POST, hnd_post_broker);
	coap_register_handler	(broker_resource, COAP_REQUEST_DELETE, hnd_delete_broker);
	coap_add_attr			(broker_resource, (unsigned char *)"ct", 2, (unsigned char *)"40", strlen("40"), 0); //40 :link
	coap_add_attr			(broker_resource, (unsigned char *)"rt", 2, (unsigned char *)"core.ps", strlen("core.ps"), 0);
	coap_add_resource		(ctx, broker_resource);
	/* Initialize the observable resource */	
	
	/* Initialize CTRL+C Handler */	
	signal(SIGINT, handleSIGINT);
	/* Initialize CTRL+C Handler */
	
	/*Listen for incoming connections*/	
	while (!quit) {
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
        //printDB(topicDB);
        topicDataMAMonitor(topicDB);
        topicMAMonitor(topicDB);
    }
    
    /* clean-up */
    debug("Exiting Main\n");
    RESOURCES_ITER((*global_ctx)->resources, r) {
		if(!compareString(r->uri.s, broker_path)){
			deleteTopic(&topicDB, r->uri.s);
			MQTTClient_unsubscribe(*global_client, r->uri.s);
			RESOURCES_DELETE((*global_ctx)->resources, r);
			coapFreeResource(r);
		}
	}	
    coap_free_context		(ctx);  
	MQTTClient_disconnect	(client, 10000);
    MQTTClient_destroy		(&client);
    /* clean-up */
    
    return 0;
}

static void
hnd_get_broker(coap_context_t *ctx, struct coap_resource_t *resource, 
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
		if((strlen(r->uri.s) > 3)){
			if(r->uri.s[0] == 'p' && r->uri.s[1] == 's' && r->uri.s[2] == '/'){
				
				int 						found_query = 0;	
				coap_opt_iterator_t 		value_opt_iter; 
				coap_option_iterator_init	(request, &value_opt_iter, COAP_OPT_ALL);
				while ((option = coap_option_next(&value_opt_iter))) {
				   if (value_opt_iter.type == COAP_OPTION_URI_QUERY) {
					   char	*query_name,*query_value;
					   int 	status = parseOptionURIQuery(coap_opt_value(option), coap_opt_length(option), &query_name, &query_value);
					   if (status == 1){
							coap_attr_t* temp_attr = coap_find_attr(r, query_name, strlen(query_name));
							if(temp_attr != NULL){
								if(compareString(temp_attr->value.s, query_value)){
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
					coap_print_link					(r, response_data, &response_size, &response_offset);
					debug							("Resource in Link Format : %s\n", response_data);
					debug							("Link Format size : %ld\n", strlen(response_data));
					debug							("Found Matching Resource with requested URI Query : %s\n", r->uri.s);
					requested_link_format_counter++;
					
					if(requested_link_format_counter == 1){
						dynamicConcatenate(&requested_link_format_data, response_data);
					}
					else {
						dynamicConcatenate(&requested_link_format_data,",");
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
				coap_add_block(response, strlen(requested_link_format_data), requested_link_format_data, block.num, block.szx);
			} 
			
			else {
				if (!coap_add_data(response, strlen(requested_link_format_data), requested_link_format_data)) { 
					block.szx = 6;
					coap_write_block_opt(&block, COAP_OPTION_BLOCK2, response,strlen(requested_link_format_data));				
					coap_add_block(response, strlen(requested_link_format_data), requested_link_format_data,block.num, block.szx);	
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

static void
hnd_post_broker(coap_context_t *ctx, struct coap_resource_t *resource, 
              const coap_endpoint_t *local_interface, coap_address_t *peer, 
              coap_pdu_t *request, str *token, coap_pdu_t *response) 
{

	coap_resource_t *new_resource = NULL;
		
	/* declare a safe variable for data */
	size_t size;
    unsigned char *data;
    unsigned char *data_safe;
	(void)coap_get_data(request, &size, &data);
	data_safe = coap_malloc(sizeof(char)*(size+2));
	snprintf(data_safe,size+1, "%s", data);
	/* declare a safe variable for data */
	
	/* parse payload */
	int status = parseLinkFormat(data_safe,resource, &new_resource);
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
				int ct_value = atoi(new_resource_attr->value.s);
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
			if(compareString(r->uri.s, new_resource->uri.s)){
				found_resource = 1; /* Jump to "Topic already exists" handler if both resource have the same uri */
				break;
			}
		}
		
		/* "Topic already exists" handler */
		if(found_resource){
			if (abs_topic_ma < earliest_topic_max_age && abs_topic_ma != 0){
					earliest_topic_max_age = abs_topic_ma;
			}
			
			updateTopicInfo(&topicDB, new_resource->uri.s, abs_topic_ma);
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
			
			MQTTClient_subscribe(*global_client, new_resource->uri.s, QOS);
			coap_register_handler(new_resource, COAP_REQUEST_GET, hnd_get_topic);
			coap_register_handler(new_resource, COAP_REQUEST_POST, hnd_post_topic);
			coap_register_handler(new_resource, COAP_REQUEST_PUT, hnd_put_topic);
			coap_register_handler(new_resource, COAP_REQUEST_DELETE, hnd_delete_topic);
			new_resource->observable = 1;
			coap_add_resource(ctx, new_resource);
			coap_add_option(response, COAP_OPTION_LOCATION_PATH, new_resource->uri.length, new_resource->uri.s);
			addTopicWEC(&topicDB, new_resource->uri.s, new_resource->uri.length, abs_topic_ma);
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

static void hnd_delete_broker(coap_context_t *ctx,
             struct coap_resource_t *resource,
             const coap_endpoint_t *local_interface,
             coap_address_t *peer,
             coap_pdu_t *request,
             str *token,
             coap_pdu_t *response){
				 
	quit = 1;
}
static void hnd_get_topic(coap_context_t *ctx, struct coap_resource_t *resource, 
              const coap_endpoint_t *local_interface, coap_address_t *peer, 
              coap_pdu_t *request, str *token, coap_pdu_t *response){
					
		unsigned char buf[3];
		int status = 0; 
		int is_observe_notification_response = 0;
		int is_observe_registration_request = 0;
		int ct_attr_value = atoi((coap_find_attr(resource,(const unsigned char*) "ct", 2))->value.s);
		
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
		
		TopicDataPtr temp = cloneTopic(&topicDB, resource->uri.s);
		/* It should never have this condition, ever. Just in Case. */
		/* Not Found */
		if (temp == NULL ) {
				response->hdr->code 	= COAP_RESPONSE_CODE(404);
				coap_add_data(response, strlen(coap_response_phrase(response->hdr->code)),(unsigned char *)coap_response_phrase(response->hdr->code));
				return;
		}
		/* Not Found */
		
		if(status || is_observe_notification_response){
			
			/* No Content */
			if (temp->data == NULL){
				if (coap_find_observer(resource, peer, token)) {
					coap_add_option(response, COAP_OPTION_OBSERVE, coap_encode_var_bytes(buf, ctx->observe), buf);
				}
				coap_add_option(response, COAP_OPTION_CONTENT_TYPE, coap_encode_var_bytes(buf, ct_attr_value), buf);
				response->hdr->code 	= COAP_RESPONSE_CODE(204);
				coap_add_data(response, strlen("No Content"),(unsigned char *)"No Content");
				freeTopic(temp);
				return;
			}
			/* No Content */
			
			/* No Content || Content */
			else {
				if (coap_find_observer(resource, peer, token)) {
					coap_add_option(response, COAP_OPTION_OBSERVE, coap_encode_var_bytes(buf, ctx->observe), buf);
				}		
				time_t remaining_maxage_time = temp->data_ma - time(NULL);
				if (remaining_maxage_time < 0 && !(temp->data_ma == 0)){
					coap_add_option(response, COAP_OPTION_CONTENT_TYPE, coap_encode_var_bytes(buf, ct_attr_value), buf);
					response->hdr->code 	= COAP_RESPONSE_CODE(204);
					coap_add_data(response, strlen("No Content"),(unsigned char *)"No Content");
					freeTopic(temp);
					return;
				}
				else{
					response->hdr->code 	= COAP_RESPONSE_CODE(205);
					coap_add_option(response, COAP_OPTION_CONTENT_TYPE, coap_encode_var_bytes(buf, ct_attr_value), buf);
					if ((!(temp->data_ma == 0))){
						printf("Current time   : %ld\n",time(NULL));
						printf("Expired time   : %ld\n",temp->data_ma);
						printf("Remaining time : %ld\n",temp->data_ma - time(NULL));
						coap_add_option(response, COAP_OPTION_MAXAGE,coap_encode_var_bytes(buf, remaining_maxage_time), buf);
					} 
					coap_add_data(response, strlen(temp->data), temp->data);
					freeTopic(temp);
					return;
				}
			}
			/* No Content || Content */
		} 		 
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
		int ct_attr_value = atoi(ct_attr->value.s);
		
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
	
	TopicDataPtr temp = cloneTopic(&topicDB, resource->uri.s);
	/* Not Found */ // It should never have this condition, ever. Just in Case. //
	if (temp == NULL ) {
		response->hdr->code 	= COAP_RESPONSE_CODE(404);
		coap_add_data(response, strlen(coap_response_phrase(response->hdr->code)),(unsigned char *)coap_response_phrase(response->hdr->code));
		return;
	}
	/* Not Found */
	
	if (updateTopicData(&topicDB, resource->uri.s, ma_opt_val_time_t, data, size)){
		if (ma_opt_val_time_t < earliest_data_max_age && ma_opt_val_time_t != 0){
			earliest_data_max_age = ma_opt_val_time_t;
		}		
		MQTTClient_message pubmsg = MQTTClient_message_initializer;
		MQTTClient_deliveryToken token;
		pubmsg.payload = data;
		pubmsg.payloadlen = size;
		pubmsg.qos = QOS;
		pubmsg.retained = 0;
		deliveredtoken = 0;
		MQTTClient_publishMessage(*global_client, resource->uri.s, &pubmsg, &token);
		debug("Waiting for publication of %s on topic %s for client with ClientID: %s\n", data, resource->uri.s, CLIENTID); // printing unsafe data //
		resource->dirty = 1;
		coap_check_notify(ctx);
		response->hdr->code = COAP_RESPONSE_CODE(204);
		coap_add_data(response, strlen(coap_response_phrase(response->hdr->code)),(unsigned char *)coap_response_phrase(response->hdr->code)); 
		freeTopic(temp);
		return;
	} 
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
		
	/* declare a safe variable for data */
	size_t size;
    unsigned char *data;
    unsigned char *data_safe;
	(void)coap_get_data(request, &size, &data);
	data_safe = coap_malloc(sizeof(char)*(size+2));
	snprintf(data_safe,size+1, "%s", data);
	/* declare a safe variable for data */
	
	/* parse payload */
	int status = parseLinkFormat(data_safe,resource, &new_resource);
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
				int ct_value = atoi(new_resource_attr->value.s);
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
			if(compareString(r->uri.s, new_resource->uri.s)){
				found_resource = 1; /* Jump to "Topic already exists" handler if both resource have the same uri */
				break;
			}
		}
		
		/* "Topic already exists" handler */
		if(found_resource){
			if (abs_topic_ma < earliest_topic_max_age && abs_topic_ma != 0){
					earliest_topic_max_age = abs_topic_ma;
			}
			
			updateTopicInfo(&topicDB, new_resource->uri.s, abs_topic_ma);
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
			
			MQTTClient_subscribe(*global_client, new_resource->uri.s, QOS);
			coap_register_handler(new_resource, COAP_REQUEST_GET, hnd_get_topic);
			coap_register_handler(new_resource, COAP_REQUEST_POST, hnd_post_topic);
			coap_register_handler(new_resource, COAP_REQUEST_PUT, hnd_put_topic);
			coap_register_handler(new_resource, COAP_REQUEST_DELETE, hnd_delete_topic);
			new_resource->observable = 1;
			coap_add_resource(ctx, new_resource);
			coap_add_option(response, COAP_OPTION_LOCATION_PATH, new_resource->uri.length, new_resource->uri.s);
			addTopicWEC(&topicDB, new_resource->uri.s, new_resource->uri.length, abs_topic_ma);
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
