#ifndef SENSOR_NODE_H
#define SENSOR_NODE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/time.h>
#include <errno.h>
#include <time.h>
#include <stdbool.h>

int fd, n, i;
struct termios toptions;

char *concat(char *s1, char *s2);
void initSerial(const char *port, speed_t baudRate);
unsigned short getADC(unsigned char pin);
void turnGPIO(unsigned short pin, bool onOff);
void setPWM(unsigned short pin, unsigned short value);

#endif
