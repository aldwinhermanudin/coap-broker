#include "cache.h"

int listLength(struct node *head) {
	struct node *current = head;
	int count = 0;
	
	while (current != NULL) {
		count++;
		current = current->next;
	}
	
	return count;
}

//critical section
void push(struct node **headReff, char *query, int data, int timeStamp, char *timeStr) {
	struct node *newNode = NULL;
	
	newNode = malloc(sizeof(struct node));
	strcpy(newNode->query, query); 
	newNode->data = data;
	newNode->timeStamp = timeStamp;
	strcpy(newNode->timeStampStr, timeStr);
	
	if (*headReff == NULL) {
		newNode->next = NULL;
		*headReff = newNode;
	}
	else {
		newNode->next = *headReff;
		*headReff = newNode;
	}
}

struct node *findNode(struct node **head, char *query) {
	int i = 0, len;
	struct node *theNode = *head;
	
	len = listLength(*head);
	for (i = 0; i < len; i++) {
		if (strcmp(query, theNode->query) == 0)
			return theNode;
		else
			theNode = theNode->next;
	}
	
	return NULL; //not found
}

//critical section
void deleteNode(struct node **head, char *queryCache) {
	int len = listLength(*head);
	int i;
	struct node *current, *before;
	
	current = *head;
	before = current;
	if (strcmp(current->query, queryCache) == 0) {
		printf("YAAAAA\n");
		printf("head->query: %s\n", (*head)->query);
		*head = current->next;
		free(current);
		return;
	}
	for (i = 0; i < len; i++) {
		if (strcmp(current->query, queryCache) == 0) {
			before->next = current->next;
			free(current);
			break;
		}
		before = current;
		current = current->next;
	}
}

//critical section
void deleteAllNode(struct node **head) {
	int i = 0, len = listLength(*head);
	struct node *current = *head;
	struct node *before = current;
	
	for (i = 0; i < len; i++) {
		before = current;
		current = current->next;
		free(before);
	}
	
	*head = NULL;
}

void printAllNodes(struct node *head) {
	int i = 0, len = listLength(head);
	struct node *current = head;
	
	for (i = 0; i < len; i++) {
		printf("node: %d\n", (i+1));
		printf("list->query: %s\n", current->query);
		printf("list->data: %d\n", current->data); 
		printf("list->timeStap: %d\n", current->timeStamp);
		printf("list->timeStr: %s\n", current->timeStampStr);
		current = current->next;
		printf("\n");
	}
}
