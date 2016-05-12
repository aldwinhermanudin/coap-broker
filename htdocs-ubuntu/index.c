#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define DATAINDEX 0
#define MAXAGEINDEX 1

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
}

void pisahinAngka(char *res, char *asli) {
	int i, j = 0, len;
	len = strlen(asli);
	for (i = 0; i < len; i++) {
		if (asli[i] >= '0' && asli[i] <= '9') {
			for (; i < len; i++) {
				res[j++] = asli[i];
			}
			res[j] = '\0';
			return;
		}
	}
}

int main() {
	int s, t, len;
    struct sockaddr_un remote;
    char str[100];
    char *data;
	char query[100], res[100];
	int hasil[3];
	char buf[10];
	char getResult[25];
    
    printf("Content-Type: text/plain;charset=us-ascii\n\n");
    
    data = getenv("QUERY_STRING"); //ternyata kalo ga ada query string langsung kelar
	if (sscanf(data, "coap_target_uri=%s", query) == EOF)
		error("gagal dapet query\n");
    
    decode(res, query);
	strcpy(query, "./client-tes -m get ");
	strcat(query, res);
	printf("%s\n", query);
	FILE *ls = popen(query, "r");
	while (fgets(getResult, sizeof(getResult), ls) != 0) {
		//wait...
	}
	pclose(ls);	
	if (strncmp("data", getResult, 4) == 0) {
		printf("sama\n");
		pisahinAngka(buf, getResult);
		printf("buf: %s\n", buf);
		explodeToInt('-', buf, hasil);
		printf("data: %d\nmaxage: %d\n", hasil[DATAINDEX], hasil[MAXAGEINDEX]);
	}
	
	return 0;
}
