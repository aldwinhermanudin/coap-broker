#include <unistd.h>    
#include <sys/types.h>  
#include <errno.h>     
#include <stdio.h>      
#include <stdlib.h>    
#include <pthread.h>    
#include <string.h>    
#include <time.h>     
#include <semaphore.h> 
#include "MQTTClient.h"

#define ADDRESS     "tcp://localhost:1883"
#define CLIENTID    "ExampleClientPub"
#define TOPIC       "test"
#define PAYLOAD     "Hello World!"
#define QOS         1
#define TIMEOUT     10000L

volatile MQTTClient_deliveryToken deliveredtoken;
MQTTClient* global_client;
void delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    int i;
    char* payloadptr;

    printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("   message: ");

    payloadptr = message->payload;
    for(i=0; i<message->payloadlen; i++)
    {
        putchar(*payloadptr++);
    }
    putchar('\n');
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

void connlost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}

void publisher_handler(void *ptr){
	
	int waitCritical = rand()%1000000;		// random waktu pengobatan
	usleep(waitCritical);
	MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    pubmsg.payload = PAYLOAD;
    pubmsg.payloadlen = strlen(PAYLOAD);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;
    deliveredtoken = 0;
    MQTTClient_publishMessage(*global_client, TOPIC, &pubmsg, &token);
    printf("Waiting for publication of %s\n"
            "on topic %s for client with ClientID: %s\n",
            PAYLOAD, TOPIC, CLIENTID);
	
	pthread_exit(0); 
	
} 

int main(int argc, char* argv[])
{
    MQTTClient client;
    global_client = &client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;

    MQTTClient_create(&client, ADDRESS, CLIENTID,
        MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);       
    }
    
    pthread_t publisher_thread[10];
    for(int x = 0 ; x < 10; x++){ 
		pthread_create (&publisher_thread[x], NULL, (void *) &publisher_handler, NULL); 
	}
    
    MQTTClient_subscribe(client, TOPIC, QOS);

    int ch;
    do 
    {
        ch = getchar();
    } while(ch!='Q' && ch != 'q');
    //while(deliveredtoken != token);
    for(int x = 0 ; x < 10; x++){ pthread_join(publisher_thread[x], NULL);} 
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}
