/* Fig. 12.3: fig12_03.c
   Operating and maintaining a list,
   which adds characters in alphabetical (ASCII) order
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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
void 		cleanDB( TopicDataPtr* sPtr );

int 		compareString(char* a, char* b){
	if (a == NULL || b == NULL) return 0;
	if (strcmp(a,b) == 0)	return 1;
	else return 0;
}


int main()
{ 
    TopicDataPtr startPtr = NULL; /* initially there are no nodes */
    int total_topic = 1000;
    for (int ctr = 0; ctr < total_topic; ctr++) {
		char topic_name[100];
		sprintf(topic_name,"test%d",ctr);
		addTopicWEC(&startPtr, topic_name, strlen(topic_name), time(NULL)); 
	}  
	printDB(startPtr);
	cleanDB(&startPtr);
	printDB(startPtr);
	/*
	for (int ctr = 0; ctr < total_topic; ctr++) {
		char topic_name[100];
		sprintf(topic_name,"test%d",ctr);
		deleteTopic(&startPtr, topic_name); 
	}  
	printDB(startPtr);
    
    int ftest[9];
    ftest[0] = getTopic(&startPtr, "LALALILILELO") != NULL ? 1 : 0;
    ftest[1] = setTopic(&startPtr, "LALALILILELO", "lala",4, 0, 0 );
    ftest[2] = topicExist(&startPtr,  "LALALILILELO");
    ftest[3] = deleteTopic(&startPtr, "LALALILILELO");
    ftest[4] = updateTopicInfo(&startPtr, "LALA", 65);
    ftest[5] = updateTopicData(&startPtr, "LLI", 60, "asd", 3);
    ftest[8] = DBEmpty(startPtr);
	ftest[6] = addTopic(&startPtr, "test/test2/test3", strlen("test/test2/test3"), time(NULL));
    ftest[7] = addTopicWEC(&startPtr, "test/test2/test3", strlen("test/test2/test3"), time(NULL));
    
    for (int ctr = 0; ctr < 9; ctr++) printf("Value %d : %d\n", ctr, ftest[ctr]);
    
    int stest[9];
    stest[0] = getTopic(&startPtr, "test/test2/test3") != NULL ? 1 : 0;
    stest[1] = setTopic(&startPtr, "test/test2/test3", "lala",4, 0, 0 );
    stest[2] = topicExist(&startPtr,  "test/test2/test3");
    stest[4] = updateTopicInfo(&startPtr, "test/test2/test3", 65);
    stest[5] = updateTopicData(&startPtr, "test/test2/test3", 60, "asd", 3);
    stest[3] = deleteTopic(&startPtr, "test/test2/test3");
    stest[8] = DBEmpty(startPtr);
	stest[6] = addTopic(&startPtr, "test/test2/test3", strlen("test/test2/test3"), time(NULL));
    stest[7] = addTopicWEC(&startPtr, "test/test2/test3", strlen("test/test2/test3"), time(NULL));    
    
     for (int ctr = 0; ctr < 9; ctr++) printf("Value %d : %d\n", ctr, stest[ctr]);
    
    addTopicWEC(&startPtr, "test", strlen("test"), time(NULL));
    addTopicWEC(&startPtr, "test/test2", strlen("test/test2"), time(NULL));
    addTopicWEC(&startPtr, "test/test2/test3", strlen("test/test2/test3"), time(NULL));
    addTopicWEC(&startPtr, "test/test2/test3", strlen("test/test2/test3"), time(NULL)) ? printf("add topic success") : printf("add topic failed");
    printDB(startPtr) ;
    addTopicWEC(&startPtr, "test/test2/test3", strlen("test/tes"), time(NULL));
    printDB(startPtr);
    addTopicWEC(&startPtr, "test/test2/test3", strlen("test/test2/test3"), time(NULL)+123 );
    printDB(startPtr);
    topicExist(&startPtr, "test/test2") ? printf("exist!\n") : printf("not exist!\n");
    topicExist(&startPtr, "test/test4") ? printf("exist!\n") : printf("not exist!\n");
    updateTopicData(&startPtr, "test/test2", time(NULL)+30, "aldwin", strlen("aldwin"));
    updateTopicInfo(&startPtr, "test/test2", time(NULL)+30 );
    printDB(startPtr);
    deleteTopic(&startPtr, "test/test2");
    printDB(startPtr);
    deleteTopic(&startPtr, "test/test2/test3");
    printDB(startPtr);
    deleteTopic(&startPtr, "test");
    printDB(startPtr);
    addTopicWEC(&startPtr, NULL, strlen("test"), time(NULL));
    addTopicWEC(&startPtr, "test", strlen("test"), time(NULL));
    addTopicWEC(&startPtr, "test123", strlen("test123"), time(NULL));
    updateTopicData(&startPtr, "test/tes2", time(NULL)+30, "aldwin", strlen("aldwin"));
    updateTopicData(&startPtr, "test/tes2", time(NULL)+30, "aldwin", 0);
    updateTopicData(&startPtr, "test/tes2", time(NULL)+30, NULL, strlen("aldwin"));
    updateTopicData(&startPtr, "test", time(NULL)+30, "aldwin", strlen("aldwin"));
    printDB(startPtr); 
    addTopicWEC(&startPtr, "test123", strlen("test123"), 0);
    printDB(startPtr); 
    
    
    deleteTopic(&startPtr, "test");
    deleteTopic(&startPtr, "test123");
    printDB(startPtr); */
    return 0; /* indicates successful termination */

} /* end main */


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
