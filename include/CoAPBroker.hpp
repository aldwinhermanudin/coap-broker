#ifndef COAP_BROKER_HPP
#define COAP_BROKER_HPP

#include <limits.h> 
#include <string>
#include "LinkedListDB.hpp"
#include <MQTTClient.h>
#include "coap.h"
#include "LibcoapMod.hpp"
#include "LinkFormatParser.hpp"
#include "EString.hpp"
#include "CoAPResource.hpp"
#include "CoAPServer.hpp"

/* MQTT bridge variable */
#define CLIENTID	"CoAPBroker"
#define QOS         1
#define TIMEOUT     10000L

class CoAPBroker : public CoAPResource{
    
    private:
        /* Global variable */
        static coap_context_t**  	global_ctx;
        static TopicDB      topic_db;
        static MQTTClient*  global_client;
        static bool          mqtt_bridge;
        static time_t       earliest_topic_max_age;
        static time_t		earliest_data_max_age;
        static volatile     MQTTClient_deliveryToken deliveredtoken;

        static int	compareString(char* a, char* b);
            
        static void hnd_get_broker(coap_context_t *ctx, struct coap_resource_t *resource, 
                    const coap_endpoint_t *local_interface, coap_address_t *peer, 
                    coap_pdu_t *request, str *token, coap_pdu_t *response);
    
    
        static void hnd_post_broker(coap_context_t *ctx, struct coap_resource_t *resource, 
                    const coap_endpoint_t *local_interface, coap_address_t *peer, 
                    coap_pdu_t *request, str *token, coap_pdu_t *response);
    
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
    
                                    
        static void hnd_post_topic(coap_context_t *ctx ,
                        struct coap_resource_t *resource ,
                        const coap_endpoint_t *local_interface ,
                        coap_address_t *peer ,
                        coap_pdu_t *request ,
                        str *token ,
                        coap_pdu_t *response );
    
        static void hnd_delete_topic(coap_context_t *ctx ,
                        struct coap_resource_t *resource ,
                        const coap_endpoint_t *local_interface ,
                        coap_address_t *peer ,
                        coap_pdu_t *request ,
                        str *token ,
                        coap_pdu_t *response );

        void topicDataMAMonitor();
        void topicMAMonitor();

        /* MQTT bridge call-back */	
        static void 	delivered (void *context, MQTTClient_deliveryToken dt);
        static int 	    msgarrvd  (void *context, char *topicName, int topicLen, MQTTClient_message *message);
        static void 	connlost  (void *context, char *cause);

    public:

        CoAPBroker(EString name, CoAPServer &server);
        bool initialize_mqtt_bridge();
    };

#endif