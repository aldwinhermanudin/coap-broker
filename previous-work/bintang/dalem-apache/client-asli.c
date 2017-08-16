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

int main(int argc, char *argv[])
{
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
        printf("gagal konek: %d\n", errno);
        perror("connect");
        exit(1);
    }
	
	if (send(s, res, strlen(res), 0) == -1) {
		perror("send");
		exit(1);
	}
	if ((t=recv(s, str, 100, 0)) > 0) {
		str[t] = '\0';
		printf("echo> %s\n", str);
	} else {
		if (t < 0) perror("recv");
		else printf("Server closed connection\n");
			exit(1);
	}
	close(s);
	
    return 0;
}
