#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "sensorNode.h"


int main(int argc, char *argv[])
{
  int fd, n, i;
  char buf[64] = "temp text";
  struct termios toptions;

  fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY);
  printf("fd opened as %i\n", fd);
  
  usleep(3500000);
  
  tcgetattr(fd, &toptions);
  cfsetispeed(&toptions, B9600);
  cfsetospeed(&toptions, B9600);
  toptions.c_cflag &= ~PARENB;
  toptions.c_cflag &= ~CSTOPB;
  toptions.c_cflag &= ~CSIZE;
  toptions.c_cflag |= CS8;
  toptions.c_lflag |= ICANON;
  tcsetattr(fd, TCSANOW, &toptions);

for (i = 0; i < 8; i++) {
  write(fd, "01\n", 3);
  n = read(fd, buf, 64);
  buf[n] = 0;

  printf("%i bytes read, buffer contains: %s\n", n, buf);
  memset(buf,0,sizeof(buf));
  fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY);
  printf("fd opened as %i\n", fd);
  sleep(2);
}
 
  /*if(DEBUG)
    {
      printf("Printing individual characters in buf as integers...\n\n");
      for(i=0; i<n; i++)
	{
	  printf("Byte %i:%i, ",i+1, (int)buf[i]);
	}
      printf("\n");
    }
    **/

  return 0;
}


/*

int main() {
	int i, x;
	initSerial("/dev/ttyUSB0", B9600);
	printf("1. tes ADC...\n");
	for (i = 0; i < 6; i++) {
		x = getADC(0);
		printf("%d - ADC: %d\n", i, x);
	}
	printf("tes GPIO...\n");
	printf("matiin...\n");
	turnGPIO(3, 0);
	sleep(4);
	printf("nyalain...\n");
	turnGPIO(3, 1);
	sleep(2);
	return 0;
}

*/
