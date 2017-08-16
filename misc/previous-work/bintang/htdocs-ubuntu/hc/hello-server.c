#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

int main() {
	int s, t, len;
    struct sockaddr_un remote;
    char str[100];
    char *data;
	char query[100];
    
    printf("Content-Type: text/plain;charset=us-ascii\n\n");
	data = getenv("QUERY_STRING"); //ternyata kalo ga ada query string langsung kelar
	if (sscanf(data, "coap_target_uri=%s", query) == EOF)
		error("gagap dapet query\n");
    
    printf("The query: %s, length: %d\n", query, (int)strlen(query));
	data = getenv("REQUEST_URI"); //ternyata kalo ga ada query string langsung kelar
	if (sscanf(data, "%s", query) == EOF)
		error("gagal dapet query\n");
    
    printf("REQUEST_URI: %s\n", query);
	
	return 0;
}
