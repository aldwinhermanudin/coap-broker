/*
 * 
 */ 
#ifndef _CACHE_H
#define _CACHE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

struct node {
	char query[256];
	int data;
	unsigned int timeStamp;
	char timeStampStr[32];
	struct node *next;
};

int listLength(struct node *head);
void push(struct node **headReff, char *query, int data, int timeStamp, char *timeStr);
struct node *findNode(struct node **head, char *query);
void deleteNode(struct node **head, char *queryCache);
void deleteAllNode(struct node **head);
void printAllNodes(struct node *head);

#endif
