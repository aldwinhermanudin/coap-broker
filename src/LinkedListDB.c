#include "LinkedListDB.h"

int	compareString(char* a, char* b){
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
