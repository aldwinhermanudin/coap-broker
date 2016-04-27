#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define DATAINDEX 0
#define MAXAGEINDEX 1

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
	int hasil[3];
	char buf[10];
	char getResult[25];
	
	FILE *ls = popen("./client-tes -m get coap://[::1]/sensor", "r");
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
