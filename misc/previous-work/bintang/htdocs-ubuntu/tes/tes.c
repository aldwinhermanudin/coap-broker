#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
}

int main() {
	int i = 0;
	char res[16];
	
	for (i = 0; i < 9; i++) {
		printf("%s\n", responseTable[i][1]);
	}
	printf("----------------------------\n");
	
	responseCoAPtoHTTP("5.04", res);
	printf("%s\n", res);
	
	return 0;
}
