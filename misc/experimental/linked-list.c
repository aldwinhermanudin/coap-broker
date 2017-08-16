/* Fig. 12.3: fig12_03.c
   Operating and maintaining a list,
   which adds characters in alphabetical (ASCII) order
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define NOT_USED 0
/* self-referential structure */
struct listNode {            
	char* path;
	char * data;		/* each listNode contains a string */
    time_t topic_ma;	// max-age for topic in time after epoch
    time_t data_ma;		// max-age for data in time after epoch
    int ct_value;
    char* option_data;
    struct listNode *nextPtr; /* pointer to next node*/ 
}; /* end structure listNode */

typedef struct listNode ListNode; /* synonym for struct listNode */
typedef ListNode *ListNodePtr; /* synonym for ListNode* */

/* prototypes */
ListNodePtr getTopic( ListNodePtr *sPtr, char* path);
int			setTopic( ListNodePtr *sPtr, 
					char* path, char* data, size_t data_size, 
					time_t topic_ma, time_t data_ma, int ct_value,
					char* option_data, size_t option_size);
					
int 		topicExist(ListNodePtr *sPtr, char* path );				
int			addTopic(ListNodePtr *sPtr,
					char* path, size_t path_size,
					time_t topic_ma, int ct_value,
					char* option_data, size_t option_size);
int			addTopicWEC(ListNodePtr *sPtr,
					char* path, size_t path_size,
					time_t topic_ma, int ct_value,
					char* option_data, size_t option_size);
int 		deleteTopic( ListNodePtr *sPtr, char* path );					
int			updateTopicInfo(ListNodePtr *sPtr,
					char* path, time_t topic_ma, int ct_value,
					char* option_data, size_t option_size);
					
int			updateTopicData(ListNodePtr *sPtr,
					char* path, time_t data_ma,
					char* data, size_t data_size);


int 		DBEmpty( ListNodePtr sPtr );
void 		printDB( ListNodePtr currentPtr );

int 		compareString(char* a, char* b){
	if (a == NULL || b == NULL) return 0;
	if (strcmp(a,b) == 0)	return 1;
	else return 0;
}

int main()
{ 
    ListNodePtr startPtr = NULL; /* initially there are no nodes */
     
    int ftest[9];
    ftest[0] = getTopic(&startPtr, "LALALILILELO") != NULL ? 1 : 0;
    ftest[1] = setTopic(&startPtr, "LALALILILELO", "lala",4, 0, 0,0,"lili",4);
    ftest[2] = topicExist(&startPtr,  "LALALILILELO");
    ftest[3] = deleteTopic(&startPtr, "LALALILILELO");
    ftest[4] = updateTopicInfo(&startPtr, "LALA", 65, 60, "as", 2);
    ftest[5] = updateTopicData(&startPtr, "LLI", 60, "asd", 3);
    ftest[8] = DBEmpty(startPtr);
	ftest[6] = addTopic(&startPtr, "test/test2/test3", strlen("test/test2/test3"), time(NULL), 30, "<option3>", strlen("<option3>"));
    ftest[7] = addTopicWEC(&startPtr, "test/test2/test3", strlen("test/test2/test3"), time(NULL), 30, "<option3>", strlen("<option3>"));
    
    for (int ctr = 0; ctr < 9; ctr++) printf("Value %d : %d\n", ctr, ftest[ctr]);
    
    int stest[9];
    stest[0] = getTopic(&startPtr, "test/test2/test3") != NULL ? 1 : 0;
    stest[1] = setTopic(&startPtr, "test/test2/test3", "lala",4, 0, 0,0,"lili",4);
    stest[2] = topicExist(&startPtr,  "test/test2/test3");
    stest[4] = updateTopicInfo(&startPtr, "test/test2/test3", 65, 60, "as", 2);
    stest[5] = updateTopicData(&startPtr, "test/test2/test3", 60, "asd", 3);
    stest[3] = deleteTopic(&startPtr, "test/test2/test3");
    stest[8] = DBEmpty(startPtr);
	stest[6] = addTopic(&startPtr, "test/test2/test3", strlen("test/test2/test3"), time(NULL), 30, "<option3>", strlen("<option3>"));
    stest[7] = addTopicWEC(&startPtr, "test/test2/test3", strlen("test/test2/test3"), time(NULL), 30, "<option3>", strlen("<option3>"));    
    
     for (int ctr = 0; ctr < 9; ctr++) printf("Value %d : %d\n", ctr, stest[ctr]);
    
    addTopicWEC(&startPtr, "test", strlen("test"), time(NULL), 50, "<option>", strlen("<option>"));
    addTopicWEC(&startPtr, "test/test2", strlen("test/test2"), time(NULL), 40, "<option2>", strlen("<option2>"));
    addTopicWEC(&startPtr, "test/test2/test3", strlen("test/test2/test3"), time(NULL), 30, "<option3>", strlen("<option3>"));
    addTopicWEC(&startPtr, "test/test2/test3", strlen("test/test2/test3"), time(NULL), 30, "<option3>", strlen("<option3>")) ? printf("add topic success") : printf("add topic failed");
    printDB(startPtr) ;
    addTopicWEC(&startPtr, "test/test2/test3", strlen("test/test2/test3"), time(NULL), 50, "lololasdasd", 3);
    printDB(startPtr);
    addTopicWEC(&startPtr, "test/test2/test3", strlen("test/test2/test3"), time(NULL)+123, 60, "<option345>", strlen("<option345>"));
    printDB(startPtr);
    topicExist(&startPtr, "test/test2") ? printf("exist!\n") : printf("not exist!\n");
    topicExist(&startPtr, "test/test4") ? printf("exist!\n") : printf("not exist!\n");
    updateTopicData(&startPtr, "test/test2", time(NULL)+30, "aldwin", strlen("aldwin"));
    updateTopicInfo(&startPtr, "test/test2", time(NULL)+30,20, "<option23>", strlen("<option23>"));
    printDB(startPtr);
    deleteTopic(&startPtr, "test/test2");
    printDB(startPtr);
    deleteTopic(&startPtr, "test/test2/test3");
    printDB(startPtr);
    deleteTopic(&startPtr, "test");
    printDB(startPtr);
    addTopicWEC(&startPtr, "test", strlen("test"), time(NULL), 50, "<option>", strlen("<option>"));
    updateTopicData(&startPtr, "test/tes2", time(NULL)+30, "aldwin", strlen("aldwin"));
    updateTopicData(&startPtr, "test", time(NULL)+30, "aldwin", strlen("aldwin"));
    printDB(startPtr); 
    return 0; /* indicates successful termination */

} /* end main */


/* start function get */
ListNodePtr getTopic( ListNodePtr *sPtr, char* path)
{ 
    ListNodePtr previousPtr = NULL; /* pointer to previous node in list */
    ListNodePtr currentPtr = NULL;  /* pointer to current node in list */
    ListNodePtr tempPtr = NULL;     /* temporary node pointer */
	
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

int setTopic( ListNodePtr *sPtr, 
					char* path, char* data, size_t data_size, 
					time_t topic_ma, time_t data_ma, int ct_value,
					char* option_data, size_t option_size)
{ 
	
	ListNodePtr topic = getTopic(sPtr, path);
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
			
		if( option_size > 0 && option_data != NULL){
			char* temp_option_data = malloc(sizeof(char) * (option_size + 2));
			snprintf(temp_option_data,sizeof(char) * (option_size + 1) ,"%s", option_data);
			topic->option_data 	= temp_option_data;
		}
		else {
			topic->option_data 	= NULL;
		}
		
		topic->topic_ma		= topic_ma;
		topic->data_ma 		= data_ma;
		topic->ct_value 	= ct_value;
		return 1;
	}
	
    return 0;

} 

int topicExist(ListNodePtr *sPtr, char* path )
{ 
    ListNodePtr topic = getTopic(sPtr, path);
	if (topic == NULL) return 0;
	else return 1;

} 

int	addTopic(ListNodePtr *sPtr,
					char* path, size_t path_size,
					time_t topic_ma, int ct_value,
					char* option_data, size_t option_size){
					
    ListNodePtr newPtr = NULL;      /* pointer to new node */
    ListNodePtr previousPtr = NULL; /* pointer to previous node in list */
    ListNodePtr currentPtr = NULL;  /* pointer to current node in list */

    newPtr = malloc( sizeof( ListNode ) ); /* create node on heap */
	
    if ( newPtr != NULL ) { /* is space available */
		
		
		/* add data to new struct here */
		if (path_size > 0){
			char* temp_path = malloc(sizeof(char) * (path_size + 2));		
			snprintf(temp_path,sizeof(char) * (path_size+1), "%s", path);
			newPtr->path = temp_path; /* place value in node */
		}
		else {
			free(newPtr);
			return 0;
		}
		
		if (option_size > 0 || option_data != NULL){
			char* temp_option_data = malloc(sizeof(char) * (option_size + 2));
			snprintf(temp_option_data,sizeof(char) * (option_size+1), "%s", option_data);
			newPtr->option_data = temp_option_data; /* place value in node */
		}
		else {
			newPtr->option_data = NULL;
		}
        
        newPtr->data = NULL; /* place value in node */
        newPtr->data_ma = 0; /* place value in node */
        newPtr->topic_ma = topic_ma; /* place value in node */
        newPtr->ct_value = 0; /* place value in node */
        newPtr->nextPtr = NULL; /* node does not link to another node */
		/* add data to new struct here */
		
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

int	addTopicWEC(ListNodePtr *sPtr,
					char* path, size_t path_size,
					time_t topic_ma, int ct_value,
					char* option_data, size_t option_size){
	
	ListNodePtr temp = getTopic(sPtr, path);
	if(temp != NULL){
		
		return updateTopicInfo(sPtr,path, topic_ma, ct_value, option_data, option_size);
	}
	else {
		return addTopic( sPtr, path, path_size, topic_ma, ct_value, option_data, option_size);
	}
					
}
int deleteTopic( ListNodePtr *sPtr, char* path)
{ 
    ListNodePtr previousPtr = NULL; /* pointer to previous node in list */
    ListNodePtr currentPtr = NULL;  /* pointer to current node in list */
    ListNodePtr tempPtr = NULL;     /* temporary node pointer */

	if (path == NULL || DBEmpty(*sPtr)) { return 0;}
	
    /* delete first node */
    if ( compareString(( *sPtr )->path,path)) { 
        tempPtr = *sPtr; /* hold onto node being removed */
        *sPtr = ( *sPtr )->nextPtr; /* de-thread the node */
        free( tempPtr->data );
        free( tempPtr->path );
        free( tempPtr->option_data );
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
			free( tempPtr->option_data );
            free( tempPtr );
            return 1;
        } /* end if */
     
    } /* end else */

    return 0;

} /* end function delete */

int updateTopicInfo(ListNodePtr *sPtr,
					char* path, time_t topic_ma, int ct_value,
					char* option_data, size_t option_size){
						
	ListNodePtr temp = getTopic(sPtr, path);
	if(temp != NULL){
		size_t data_size = 0;
		if (temp->data == NULL) {
			data_size = 0;
		}
		else {
			data_size = strlen(temp->data);
		}
		return setTopic(sPtr,path, temp->data, data_size, topic_ma, temp->data_ma, ct_value, option_data, option_size);
	}
	else {
		return 0;
	}
	
} /* end function delete */


/* start function set */
int updateTopicData(ListNodePtr *sPtr,
					char* path, time_t data_ma,
					char* data, size_t data_size){
   
	ListNodePtr temp = getTopic(sPtr, path);
	if(temp != NULL){
		size_t option_size = 0;
		if (temp->option_data == NULL) {
			option_size = 0;
		}
		else {
			option_size = strlen(temp->option_data);
		}
		return setTopic(sPtr,path, data, data_size, temp->topic_ma, data_ma, temp->ct_value, temp->option_data, option_size);
	}
	else {
		return 0;
	}

} /* end function delete */


/* Return 1 if the list is empty, 0 otherwise */
int DBEmpty( ListNodePtr sPtr )
{ 
    return sPtr == NULL;

} /* end function isEmpty */

/* Print the list */
void printDB( ListNodePtr currentPtr )
{ 

    /* if list is empty */
    if ( currentPtr == NULL ) {
        printf( "List is empty.\n\n" );
    } /* end if */
    else { 
        printf( "The list is:\n" );

        /* while not the end of the list */
        while ( currentPtr != NULL ) { 
            printf( "%s %s %d %d %d %s--> ",currentPtr->path, currentPtr->data,
            (int)currentPtr->topic_ma, (int)currentPtr->data_ma,
            currentPtr->ct_value, currentPtr->option_data);
            currentPtr = currentPtr->nextPtr;   
        } /* end while */

        printf( "NULL\n\n" );
    } /* end else */

} /* end function printList */
