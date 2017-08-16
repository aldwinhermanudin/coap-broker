#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include "cache.h"

#define SOCK_PATH "echo_socket"
#define NUMB_OF_SOCKETS 10 //number of incoming requests permitted kali ye?

int globalNih = 0;
sem_t mutex;
pthread_mutex_t mutex_cache = PTHREAD_MUTEX_INITIALIZER;

void *handle_request(void *par);

void explodeToInt(char delimiter, char *string, int *arr) {
	int len = strlen(string);
	int i, buff, indexArray = 0;
	buff = string[0] - '0';
	
	for (i = 1; i <= len; i++) {
		if (string[i] == delimiter || string[i] == '\0') {
			arr[indexArray++] = buff;
			i++;
			buff = string[i] - '0';
		}
		else
			buff = buff * 10 + (string[i] - '0');
	}
	printf("YEAH\n");
	for (i = 0; i < indexArray; i++) {
		printf("arr[%d]: %d\n", i, arr[i]);
	}
}

char *concat(char *s1, char *s2)
{
    char *result = malloc(strlen(s1)+strlen(s2)+1);
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

int main(void)
{
    int s, s2, t, len, i = 0, j = 0;
    struct sockaddr_un local, remote;
    char str[100];
    pthread_t threadRequest[NUMB_OF_SOCKETS];

	sem_init(&mutex, 0, 1);
	
    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    local.sun_family = AF_UNIX;
    strcpy(local.sun_path, SOCK_PATH);
    unlink(local.sun_path);
    len = strlen(local.sun_path) + sizeof(local.sun_family);
    if (bind(s, (struct sockaddr *)&local, len) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(s, NUMB_OF_SOCKETS) == -1) {
        perror("listen");
        exit(1);
    }

    while (1) {
        int done, n;
        printf("Waiting for a connection...\n");
        t = sizeof(remote);
        if ((s2 = accept(s, (struct sockaddr *)&remote, &t)) == -1) {
            perror("accept");
            exit(1);
        }

        printf("s2 = %d\n", s2);
        
        if ( pthread_create(&threadRequest[i++], NULL, handle_request, (void *)s2) ) {
			printf("error creating thread.");
			abort();
		}
    }
	
	for (j = 0; j <= i; j++) {
		pthread_join(threadRequest[j], NULL);
	}
    return 0;
}

void *handle_request(void *par) {
	int socket_desc = (int)par;
	int n;
	char str[100], strInt[10];
	int done, ret;
	char *c;
	char getResult[25];
	
	printf("s2 - thread = %d\n", socket_desc);
	
	done = 0;
    
	n = recv(socket_desc, str, 100, 0);
    if (n <= 0) {
		if (n < 0) 
			perror("recv");
	}
	
	sem_wait(&mutex); //the question is how to make a timeout if the mutex is too long locked by another thread
	
	//start of the critical section
	globalNih++; //this is only for testing purpose
	sleep(2);    //this is only fot testing purpose
	
	//this will be something like...
	/*
	 
	strcpy(query, "./client-tes -m get "); //buat command coap kayak coap://[ipv6lowpan]/sensor
	strcat(query, res);
	printf("%s\n", query);
	FILE *ls = popen(query, "r");
	while (fgets(getResult, sizeof(getResult), ls) != 0) {
		//wait...
	}
	pclose(ls);	
	
	pthread_create(&functionForInsertingToCache, ....) //->dalam bentuk linked list disimpannya
	*/
	//kalo ga jalan untuk beberpa waktu, post aja semnya
	
	//end of the critical section
	sem_post(&mutex);
	
	printf("c: %s\n", c);
	if (!done) {
		if (send(socket_desc, str, n, 0) < 0) {
			perror("send");
			done = 1;
		}
	}	
}
