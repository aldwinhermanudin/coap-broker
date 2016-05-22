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
#define max_execution_time 30

int globalNih = 0;
sem_t mutex;
pthread_mutex_t mutex_cache = PTHREAD_MUTEX_INITIALIZER;
struct node *cache = NULL;

void *handle_request(void *par);

void explodeToInt(char delimiter, char *string, int *arr) {
	int len = strlen(string);
	int i, buff, indexArray = 0;
	buff = string[0] - '0';
	
	printf("explode:\n");
	printf("string: %s\n", string);
	printf("buff awal: %d\n", buff);
	for (i = 1; i <= len; i++) {
		if (string[i] == delimiter || string[i] == '\0') {
			arr[indexArray] = buff;
			indexArray++;
			i++;
			buff = string[i] - '0';
		}
		else
			buff = buff * 10 + (string[i] - '0');
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
    int s, s2, t, len, i = 0, j = 0, ret;
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
	int n, data, hasilArr[3];
	int socket_desc = (int)par;
	char str[100], strInt[10], query[100], queryClient[100];
	int done, ret;
	char *c;
	char getResult[320];
	time_t curtime;
	struct tm *loctime;
	unsigned int sebelum, sekarang;
	struct node *temp = NULL;
	
	//receive from client IPC
	n = recv(socket_desc, str, 100, 0);
    if (n <= 0) {
		if (n < 0) 
			perror("recv");
	}
	str[n] = '\0'; //hasil receive
	
	strcpy(queryClient, str);
	
	//cari di cache
	pthread_mutex_lock(&mutex_cache);
	temp = findNode(&cache, str);
	pthread_mutex_unlock(&mutex_cache);
	
	if (strstr(str, "gpio") != NULL) {
		printf("2.5\n");
		if (temp != NULL) {
			printf("gpio ada di cache\n");
			return;
		}
		else {
			pthread_mutex_lock(&mutex_cache);
			deleteWordNode(&cache, "gpio");
			pthread_mutex_unlock(&mutex_cache);
		}
	}
	else if (strstr(str, "pwm") != NULL) {
		if (temp != NULL) {
			printf("pwm ada di cache\n");
			return;
		}
		else {
			pthread_mutex_lock(&mutex_cache);
			deleteWordNode(&cache, "pwm");
			pthread_mutex_unlock(&mutex_cache);
		}
	}
	else if (strstr(str, "sensor") != NULL) {
		if (temp != NULL) {
			sprintf(str, "%d", temp->data);
			strcat(str, " dari cache");
			printf("data cache: %d\n", temp->data);
			if (send(socket_desc, str, n, 0) < 0) {
				perror("send");
			}
			return;
		}
	}
	else if (strstr(str, "well-known") != NULL) {
		strcpy(query, "./client-tes -m get "); //buat command coap kayak coap://[ipv6lowpan]/sensor
		strcat(query, str);
		
		sem_wait(&mutex); //the question is how to make a timeout if the mutex is too long locked by another thread
		printf("query: %s\n", query);
		FILE *ls = popen(query, "r");
		while (fgets(getResult, sizeof(getResult), ls) != 0) {
			//wait...
			//Get the current time. 
			curtime = time (NULL);

			//Convert it to local time representation. 
			loctime = localtime (&curtime);
			sekarang = (unsigned int)curtime;
			if (sekarang == (sebelum + max_execution_time)) {
				if (send(socket_desc, "err: 5.04 gateway timeout", n, 0) < 0) {
					perror("send");
					done = 1;
				}
				sem_post(&mutex);
				return;
			}
		}
		pclose(ls);	
		printf("aaaaa: %s, \nlength: %zu\nn: %d\n", getResult, strlen(getResult), n);
		//end of critical section
		sem_post(&mutex);
		if (send(socket_desc, getResult, strlen(getResult), 0) < 0) {
			perror("send");
		}
		return;
	}
	else {
		if (send(socket_desc, "4.04 Not Found", n, 0) < 0) {
			perror("send");
		}
		return;
	}
	
	// Get the current time. 
	curtime = time (NULL);

	// Convert it to local time representation. 
	loctime = localtime (&curtime);
	sebelum = (unsigned int)curtime;
	
	strcpy(query, "./client-tes -m get "); //buat command coap kayak coap://[ipv6lowpan]/sensor
	strcat(query, str);
	
	sem_wait(&mutex); //the question is how to make a timeout if the mutex is too long locked by another thread
	printf("query: %s\n", query);
	FILE *ls = popen(query, "r");
	while (fgets(getResult, sizeof(getResult), ls) != 0) {
		//wait...
		//Get the current time. 
		curtime = time (NULL);

		//Convert it to local time representation. 
		loctime = localtime (&curtime);
		sekarang = (unsigned int)curtime;
		if (sekarang == (sebelum + max_execution_time)) {
			if (send(socket_desc, "err: 5.04 gateway timeout", n, 0) < 0) {
				perror("send");
				done = 1;
			}
			sem_post(&mutex);
			return;
		}
	}
	pclose(ls);	
	
	//end of critical section
	sem_post(&mutex);
	
	//Get the current time. 
	curtime = time (NULL);
	//Convert it to local time representation. 
	loctime = localtime (&curtime);
	
	explodeToInt('-', getResult, hasilArr);
	data = hasilArr[0];
	pthread_mutex_lock(&mutex_cache);
	push(&cache, str, data, (int)curtime, asctime(loctime));
	pthread_mutex_unlock(&mutex_cache);
	
	if (strstr(str, "sensor")) {
		sprintf(str, "%d", hasilArr[0]);
		if (send(socket_desc, str, n, 0) < 0) {
			perror("send");
		}
	}
	
	printf("sleep: %s\n", queryClient);
	sleep(hasilArr[1]);
	printf("selesai sleep: %s\n", queryClient);
	pthread_mutex_lock(&mutex_cache);
	deleteNode(&cache, queryClient);
	pthread_mutex_unlock(&mutex_cache);
}

/*
void *handle_request(void *par) {
	int n, data, hasilArr[3];
	int socket_desc = (int)par;
	char str[100], strInt[10], query[100], queryClient[100];
	int done, ret;
	char *c;
	char getResult[25];
	time_t curtime;
	struct tm *loctime;
	unsigned int sebelum, sekarang;
	struct node *temp = NULL;
	
	n = recv(socket_desc, str, 100, 0);
    if (n <= 0) {
		if (n < 0) 
			perror("recv");
	}
	str[n] = '\0';
	
	strcpy(query, str);
	strcpy(queryClient, str);
	//cek dulu di database cache
	
	if (strstr(str, "gpio") != NULL) {
		pthread_mutex_lock(&mutex_cache);
		temp = findNode(&cache, str);
		pthread_mutex_unlock(&mutex_cache);
		
		if (temp != NULL)
			return;
		else {
			deleteWordNode(&cache, "gpio");
			// Get the current time. 
			curtime = time (NULL);

			// Convert it to local time representation. 
			loctime = localtime (&curtime);
			sebelum = (unsigned int)curtime;
	
			strcpy(query, "./client-tes -m get "); //buat command coap kayak coap://[ipv6lowpan]/sensor
			strcat(query, str);
			
			sem_wait(&mutex); 
			system(query);
			
			//end of critical section
			sem_post(&mutex);
			
			//Get the current time. 
			curtime = time (NULL);
			//Convert it to local time representation. 
			loctime = localtime (&curtime);
			
		}
	}
	else if (strstr(str, "pwm") != NULL) {
		
	}
	pthread_mutex_lock(&mutex_cache);
	temp = findNode(&cache, str);
	pthread_mutex_unlock(&mutex_cache);
	
	
	//kirim data dari cache
	if (temp != NULL) {
		sprintf(str, "%d", temp->data);
		strcat(str, " dari cache");
		printf("data cache: %d\n", temp->data);
		if (send(socket_desc, str, n, 0) < 0) {
			perror("send");
			done = 1;
		}
		return;
	}
	//end of checking database cache
	
	// Get the current time. 
	curtime = time (NULL);

	// Convert it to local time representation. 
	loctime = localtime (&curtime);
	sebelum = (unsigned int)curtime;
	
	 
	strcpy(query, "./client-tes -m get "); //buat command coap kayak coap://[ipv6lowpan]/sensor
	strcat(query, str);
	
	sem_wait(&mutex); //the question is how to make a timeout if the mutex is too long locked by another thread
	printf("query: %s\n", query);
	FILE *ls = popen(query, "r");
	while (fgets(getResult, sizeof(getResult), ls) != 0) {
		//wait...
		//Get the current time. 
		curtime = time (NULL);

		//Convert it to local time representation. 
		loctime = localtime (&curtime);
		sekarang = (unsigned int)curtime;
		if (sekarang == (sebelum + max_execution_time)) {
			if (send(socket_desc, "err: 5.04 gateway timeout", n, 0) < 0) {
				perror("send");
				done = 1;
			}
			sem_post(&mutex);
			return;
		}
	}
	pclose(ls);	
	
	//end of critical section
	sem_post(&mutex);
	
	//Get the current time. 
	curtime = time (NULL);
	//Convert it to local time representation. 
	loctime = localtime (&curtime);
	
	explodeToInt('-', getResult, hasilArr);
	data = hasilArr[0];
	pthread_mutex_lock(&mutex_cache);
	push(&cache, str, data, (int)curtime, asctime(loctime));
	pthread_mutex_unlock(&mutex_cache);
	
	sprintf(str, "%d", hasilArr[0]);
	printf("yeah: %s\n", str);
	if (send(socket_desc, str, n, 0) < 0) {
		perror("send");
	}
	
	sleep(hasilArr[1]);
	pthread_mutex_lock(&mutex_cache);
	deleteNode(&cache, queryClient);
	pthread_mutex_unlock(&mutex_cache);
}
*/
//void *handle_request(void *par) {
	/*int socket_desc = (int)par;
	int n, data, hasilArr[3];
	char str[100], strInt[10], query[100];
	int done, ret;
	char *c;
	char getResult[25];
	time_t curtime;
	struct tm *loctime;
	unsigned int sebelum, sekarang;
	pthread_attr_t tattr;
	pthread_t tid;
    struct deletePar *paketDelete = malloc(sizeof(struct deletePar));
    struct node *temp = NULL;
	
	printf("s2 - thread = %d\n", socket_desc);
	
    
	n = recv(socket_desc, str, 100, 0);
    if (n <= 0) {
		if (n < 0) 
			perror("recv");
	}
	
	strcpy(query, str);
	
	//cek dulu di database cache
	pthread_mutex_lock(&mutex_cache);
	temp = findNode(&cache, str);
	pthread_mutex_unlock(&mutex_cache);
	
	//kirim data dari cache
	if (temp != NULL) {
		sprintf(str, "%d", temp->data);
		strcat(str, " dari cache");
		printf("data cache: %d\n", temp->data);
		if (send(socket_desc, str, n, 0) < 0) {
			perror("send");
			done = 1;
		}
		return;
	}
	//end of checking database cache
	
	// Get the current time. 
	curtime = time (NULL);

	// Convert it to local time representation. 
	loctime = localtime (&curtime);
	sebelum = (unsigned int)curtime;
	
	//start of the critical section
	
	//this will be something like...
	 
	strcpy(query, "./client-tes -m get "); //buat command coap kayak coap://[ipv6lowpan]/sensor
	strcat(query, str);
	
	sem_wait(&mutex); //the question is how to make a timeout if the mutex is too long locked by another thread
	
	FILE *ls = popen(query, "r");
	while (fgets(getResult, sizeof(getResult), ls) != 0) {
		//wait...
		//Get the current time. 
		curtime = time (NULL);

		//Convert it to local time representation. 
		loctime = localtime (&curtime);
		sekarang = (unsigned int)curtime;
		if (sekarang == (sebelum + max_execution_time)) {
			if (send(socket_desc, "5.04 gateway timeout", n, 0) < 0) {
				perror("send");
				done = 1;
			}
			sem_post(&mutex);
			return;
		}
	}
	pclose(ls);	
	
	//end of critical section
	sem_post(&mutex);
	
	//Get the current time. 
	curtime = time (NULL);
	//Convert it to local time representation. 
	loctime = localtime (&curtime);
	
	explodeToInt('-', getResult, hasilArr);
	data = hasilArr[0];
	pthread_mutex_lock(&mutex_cache);
	push(&cache, str, data, (int)curtime, asctime(loctime));
	pthread_mutex_unlock(&mutex_cache);
	
	printf("print semua node\n");
	printAllNodes(cache);
	
	//disini baru assign thread untuk ngapus cache
	explodeToInt('-', getResult, hasilArr);
	printf("hasilArr[0] = %d\nhasilArr[1] = %d\n", hasilArr[0], hasilArr[1]);
	
	paketDelete->maxage = hasilArr[1]; //ntar set beneran...
	paketDelete->listData = &cache;
	strcpy(paketDelete->query, query); 
	
	pthread_mutex_lock(&mutex_cache);
	ret = pthread_attr_init(&tattr);
	ret = pthread_attr_setdetachstate(&tattr,PTHREAD_CREATE_DETACHED);
	ret = pthread_create(&tid, &tattr, deleteCache, (void *)paketDelete);
	pthread_mutex_unlock(&mutex_cache);
	//kalo ga jalan untuk beberpa waktu, post aja semnya
	
	sprintf(str, "%d", hasilArr[0]);
	printf("yeah: %s\n", str);
	if (send(socket_desc, str, n, 0) < 0) {
		perror("send");
		done = 1;
	}*/
	//free(paketDelete);
		
//}
