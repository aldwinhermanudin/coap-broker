#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "sensorNode.h"


int main(int argc, char *argv[])
{
  int fd, n, i;
  char buf[64] = "temp text";
  struct termios toptions;

  /* open serial port */
  fd = open("/dev/ttyACM0", O_RDWR | O_NOCTTY);
  printf("fd opened as %i\n", fd);
  
  /* wait for the Arduino to reboot */
  usleep(3500000);
  
  /* get current serial port settings */
  tcgetattr(fd, &toptions);
  /* set 9600 baud both ways */
  cfsetispeed(&toptions, B9600);
  cfsetospeed(&toptions, B9600);
  /* 8 bits, no parity, no stop bits */
  toptions.c_cflag &= ~PARENB;
  toptions.c_cflag &= ~CSTOPB;
  toptions.c_cflag &= ~CSIZE;
  toptions.c_cflag |= CS8;
  /* Canonical mode */
  toptions.c_lflag |= ICANON;
  /* commit the serial port settings */
  tcsetattr(fd, TCSANOW, &toptions);

  /* Send byte to trigger Arduino to send string back */
  write(fd, "0", 1);
  /* Receive string from Arduino */
  n = read(fd, buf, 64);
  /* insert terminating zero in the string */
  buf[n] = 0;

  printf("%i bytes read, buffer contains: %s\n", n, buf);
 
  if(DEBUG)
    {
      printf("Printing individual characters in buf as integers...\n\n");
      for(i=0; i<n; i++)
	{
	  printf("Byte %i:%i, ",i+1, (int)buf[i]);
	}
      printf("\n");
    }

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
