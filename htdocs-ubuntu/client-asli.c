/*
** Nanti ini ditaro di CGI
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>

#define SOCK_PATH "echo_socket"
#define RESPONSE_SIZE 9

//CoAP - HTTP
char *responseTable[RESPONSE_SIZE][2] = {
	{"2.01 Created", "201 Created"},
	{"2.02 Deleted", "200 OK"},
	{"2.05 Content", "200 OK"}, 
	{"4.00 Bad Request", "400 Bad Request"},
	{"4.01 Unauthorized", "403 Forbidden"},
	{"4.04 Not Found", "404 Not Found"},
	{"5.02 Bad Gateway", "502 Bad Gateway"},
	{"5.03 Service Unavailable", "503 Service Unavailable"},
	{"5.04 Gateway Timeout", "504 Gateway Timeout"}
};

void responseCoAPtoHTTP(char *CoAPresponse, char *HTTPresponse) {
	int i = 0;
	
	for (i = 0; i < RESPONSE_SIZE; i++) {
		if (strstr(responseTable[i][0], CoAPresponse) != NULL) {
			strcpy(HTTPresponse, responseTable[i][1]);
			return;
		}
	}
	
	HTTPresponse[0] = '\0';
}

void json_encode_sensor(int val) {
	char str[32] = "{\"sensor\" : ";
	char temp[5];
	
	sprintf(temp, "%d", val);
	strcat(str, temp);
	strcat(str, "}");
	printf("%s", str);
}

void decode(char *hasil, char *asli) {
	int i, len, indexHasil = 0, indexAsli, myVal;
	char buf[3];
	
	len = strlen(asli);
	for (i = 0; i <= len; i++) {
		
		if (asli[i] == '%') {
			buf[0] = asli[++i];
			buf[1] = asli[++i];
			buf[2] = '\0';
			sscanf(buf, "%x", &myVal);
			sprintf(buf, "%c", myVal);
			hasil[indexHasil++] = buf[0];
		}
		else {
			hasil[indexHasil++] = asli[i];
		}
	}
}

void tambahInterface(char *hasil, char *asli) {
	int i, j, k, len, len2;
	char *temp;
	char interface[] = "%lowpan0";
	
	k = 0;
	len = strlen(asli);
	for (i = 0; i < len; i++) {
		if (asli[i] == ']') {
			len2 = len - i;
			temp = malloc(len2);
			for (j = i; j < len; j++) {
				temp[k++] = asli[j];
			}
			temp[k] = '\0';
			asli[i] = '\0';
			strcat(asli, interface);
			break;
		}
	}
	
	strcat(asli, temp);
	strcpy(hasil, asli);
	free(temp);
}

int main(int argc, char *argv[])
{
    int s, t, len;
    struct sockaddr_un remote;
    char str[300];
    
    char *data;
    char query[100], res[100];
	int hasil[3];
	char buf[10];
	
	char httpResponse[12];
	char complete[100];
	
	//printf("Content-Type: application/json;charset=us-ascii\n\n");
	printf("Content-Type: application/json\n");
	//printf("Status: 200 OK\n\n");
    data = getenv("QUERY_STRING"); //ternyata kalo ga ada query string langsung kelar
	if (sscanf(data, "coap_target_uri=%s", query) == EOF)
		error("gagal dapet query\n");
    
    decode(res, query);
    strcpy(complete, res);
    //tambahInterface(complete, res);
    
    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
	remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, SOCK_PATH);
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    //kalo gagal connect, dan errno = 13, berarti ga dapet permission
    //kasih chmod
    if (connect(s, (struct sockaddr *)&remote, len) == -1) {
        printf("Status: 403 Forbidden\n\n");
        exit(1);
    }
	
	if (send(s, complete, strlen(complete), 0) == -1) {
		perror("send");
		exit(1);
	}
	if (strstr(complete, "gpio") != NULL || strstr(complete, "pwm") != NULL) {
		printf("Status: 200 OK\n\n");
		close(s);
	
		return 0;
	}
	
	if (strstr(complete, "gpio") == NULL || strstr(complete, "pwm") == NULL) {
		if ((t=recv(s, str, sizeof(str), 0)) > 0) {
			str[t] = '\0';
				
			responseCoAPtoHTTP(str, httpResponse);
			if (httpResponse[0] == '\0') {
				printf("Status: 200 OK\n\n");
				if (strstr(complete, "well-known") != NULL) {
					printf("%s", str);
				}
				else
					json_encode_sensor(atoi(str));
				close(s);
				return 0;
			}
			else {
				printf("Status: %s\n\n", httpResponse);
				close(s);
				return 0;
			}
		} else {
			if (t < 0) perror("recv");
			else printf("Server closed connection\n");
				exit(1);
		}
	}
	printf("Status: 200 OK\n\n");
	close(s);
	
    return 0;
}
