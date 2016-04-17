#include "sensorNode.h"

char *concat(char *s1, char *s2)
{
    char *result = malloc(strlen(s1)+strlen(s2)+1);
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

unsigned short getADC(unsigned char pin) {
	unsigned short x;
	char *command, *c;
	char pinStr[3];
	
	sprintf(pinStr, "%d", pin);
	c = concat("python node.py ADC ", pinStr);
	command = concat(c, " 0");
	FILE *ls = popen(command, "r");
	char buf[256];
	while (fgets(buf, sizeof(buf), ls) != 0) {
    		//wait...
	}
	pclose(ls);	
	free(command);
	x = atoi(buf);
	return x;
}

void turnGPIO(unsigned char pin, bool onOff) {
	char *command, *c;
	char pinStr[3];
	
	sprintf(pinStr, "%d", pin);
	c = concat(pinStr, (onOff ? " 1" : " 0"));
	command = concat("python node.py GPIO ", c);
	if (system(command) == -1)
		error("Error");	
	free(command);
}

//PWM: 3, 5, 6, 9, 10, and 11
void setPWM(unsigned char pin, unsigned char value) {
	char *command, *c;
	char pinStr[3], val[4];
	
	if (pin != 3 && pin != 5 && pin != 6 && 
		pin != 9 && pin != 10 && pin != 11) {
		printf("PWM is only for pin 3, 5, 6, 9, 10, and 11\n");
		return;
	}
	sprintf(pinStr, "%d", pin);
	sprintf(val, " %d", value);
	c = concat(pinStr, val);
	command = concat("python node.py PWM ", c);
	if (system(command) == -1)
		error("Error");	
	free(command);
}
