/*
** echos.c -- the echo server for echoc.c; demonstrates unix sockets
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCK_PATH "echo_socket" //alamat domain

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

int main(void)
{
	int s, s2, t, len, res;
	int a[2];
	struct sockaddr_un local, remote;
	char str[100];

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

	if (listen(s, 5) == -1) {
		perror("listen");
		exit(1);
	}

	while (1) {
		int done, n;
		//printf("Waiting for a connection...\n");
		t = sizeof(remote);
		if ((s2 = accept(s, (struct sockaddr *)&remote, &t)) == -1) {
			perror("accept");
			exit(1);
		}

		printf("Connected.\n");

		done = 0;
		do {
			n = recv(s2, str, 100, 0);
			if (n <= 0) {
				if (n < 0) perror("recv");
				done = 1;
			}
			
			if (!done) {
				//printf("received string: %s\n", str);
				//disini nih ambil data ke sensor
				//bisa berupa return value ambil datanya (susah kayanya)
				//buat unix domain juga (jir muter2)
				//atau print dengan std output, capture
				
				//tes
				FILE *ls = popen("./tes-executor", "r");
				char buf[256];
				while (fgets(buf, sizeof(buf), ls) != 0) {
						//wait...
				}
				pclose(ls);	
				//printf("result: %s\n", buf);
				explodeToInt('-', buf, a);
				strcpy(str, buf);
				res = atoi(buf);
				res++;
				//printf("hasil res: %d\n", res);
				//end of tes
				
				//selesai, trus insert ke database
				
				//selesai, trus balikin
				if (send(s2, str, n, 0) < 0) {
					perror("send");
					done = 1;
				}
			}
		} while (!done);

		close(s2);
	}

	return 0;
}
