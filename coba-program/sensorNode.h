#ifndef SENSOR_NODE_H
#define SENSOR_NODE_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

char *concat(char *s1, char *s2);
unsigned short getADC(unsigned char pin);
void turnGPIO(unsigned char pin, bool onOff);
void setPWM(unsigned char pin, unsigned char value);

#endif
