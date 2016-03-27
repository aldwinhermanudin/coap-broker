/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <net/if.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>

#include <asm/types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/types.h>
#include "nl802154.h"
#include "af_ieee802154.h"

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno;
     socklen_t clilen;
     char buffer[256];
     struct sockaddr_ieee802154 serv_addr, cli_addr;
     int n;
     
     char *some_addr;
     //char str[IEEE802154_ADDR_LEN];
     
     sockfd = socket(AF_IEEE802154, SOCK_DGRAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     else 
		printf("1: Opening socket succeeds\n");
		
	serv_addr.family = AF_IEEE802154;
     serv_addr.addr.short_addr = INADDR_ANY;
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
              error("TAIII ERROR on binding");
     else 
		printf("2: Binding succeeds\n");
	
	
     /*bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = PF_IEEE802154;
     serv_addr.addr.short_addr = INADDR_ANY;
     //serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     else 
		printf("2: Binding succeeds\n");
		

	// now get it back and print it
	inet_ntop(PF_IEEE802154, &(serv_addr.sin_addr), str, INET6_ADDRSTRLEN);

	printf("%s\n", str); // prints "2001:db8:8714:3a90::12"
     /*
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     newsockfd = accept(sockfd, 
                 (struct sockaddr *) &cli_addr, 
                 &clilen);
     if (newsockfd < 0) 
          error("ERROR on accept");
     bzero(buffer,256);
     n = read(newsockfd,buffer,255);
     if (n < 0) error("ERROR reading from socket");
     printf("Here is the message: %s\n",buffer);
     n = write(newsockfd,"I got your message",18);
     if (n < 0) error("ERROR writing to socket");
     close(newsockfd);
     */
     //close(sockfd);
	printf("berhasil meen\n");
     return 0; 
}
