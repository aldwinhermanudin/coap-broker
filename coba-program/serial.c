#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sensorNode.h"

int main(int argc, char *argv[]) {
	int pil, pin, val;
	unsigned short adcVal;
	
	adcVal = getADC(1);
	printf("adc val: %hu\n", adcVal);
	//setPWM(9, 0);
	return 0;
}
