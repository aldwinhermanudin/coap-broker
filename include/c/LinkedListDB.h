#ifndef LINKED_LIST_DB_H
#define LINKED_LIST_DB_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>    
#include <time.h>

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
int 			compareString(char* a, char* b);
TopicDataPtr 	getTopic( TopicDataPtr *sPtr, char* path);
TopicDataPtr	cloneTopic( TopicDataPtr *sPtr, char* path);
int				freeTopic(TopicDataPtr topic);
	
int		getTopicPath(TopicDataPtr topic, char ** path);
int		getTopicData(TopicDataPtr topic, char ** data);
time_t 	getTopicMA(TopicDataPtr topic);
time_t 	getTopicDataMA(TopicDataPtr topic);
int 	topicNodeRLock(TopicDataPtr topic);
int 	topicNodeWLock(TopicDataPtr topic);
int 	topicNodeUnlock(TopicDataPtr topic);
int 	topicDataRLock(TopicDataPtr topic);
int 	topicDataWLock(TopicDataPtr topic);
int 	topicDataUnlock(TopicDataPtr topic);
int		setTopic( TopicDataPtr *sPtr, char* path, char* data, size_t data_size, time_t topic_ma, time_t data_ma);
int 	topicExist(TopicDataPtr *sPtr, char* path );				
int		addTopic(TopicDataPtr *sPtr, char* path, size_t path_size, time_t topic_ma);
int		addTopicWEC(TopicDataPtr *sPtr,	char* path, size_t path_size, time_t topic_ma);
int 	deleteTopic( TopicDataPtr *sPtr, char* path );					
int 	deleteTopicData( TopicDataPtr *sPtr, char* path );					
int		updateTopicInfo(TopicDataPtr *sPtr,	char* path, time_t topic_ma);
int		updateTopicData(TopicDataPtr *sPtr,	char* path, time_t data_ma,	char* data, size_t data_size);
int 	DBEmpty( TopicDataPtr sPtr );
void 	printDB( TopicDataPtr currentPtr );
void 	cleanDB( TopicDataPtr *sPtr );

#endif
