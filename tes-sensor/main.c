#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "sensorNode.h"

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
