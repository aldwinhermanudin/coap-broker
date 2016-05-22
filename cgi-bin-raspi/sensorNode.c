#include "sensorNode.h"

char *concat(char *s1, char *s2)
{
    char *result = malloc(strlen(s1)+strlen(s2)+1);
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

/*
 * port = /dev/ttyUSB0
 * baudRate = B9600
 */
void initSerial(const char *port, speed_t baudRate) {
	/* open serial port */
	fd = open(port, O_RDWR | O_NOCTTY);
	if (fd == -1)
		printf("errno: %d, string: %s\n", errno, strerror(errno));
  
	/* wait for the Arduino to reboot */
	sleep(2);
  
	/* get current serial port settings */
	tcgetattr(fd, &toptions);
	/* set 9600 baud both ways */
	cfsetispeed(&toptions, baudRate);
	cfsetospeed(&toptions, baudRate);
	/* 8 bits, no parity, no stop bits */
	toptions.c_cflag &= ~PARENB;
	toptions.c_cflag &= ~CSTOPB;
	toptions.c_cflag &= ~CSIZE;
	toptions.c_cflag |= CS8;
	/* Canonical mode */
	toptions.c_lflag |= ICANON;
	/* commit the serial port settings */
	tcsetattr(fd, TCSANOW, &toptions);
}

unsigned short getADC(unsigned char pin) {
	unsigned short x = 0;
	char buffer[4] = "0";
	char temp[2];
	char hasil[64];
	int n;
	
	n = (int)pin;
	sprintf(temp, "%d", n);
	strcat(buffer, temp);
	strcat(buffer, "\n");
	write(fd, buffer, strlen(buffer));
	n = read(fd, hasil, 64);
	hasil[n] = 0;
	x = atoi(hasil);
	return x;
}

void turnGPIO(unsigned short pin, bool onOff) {
	char command[4] = "1";
	int x;
	char temp[3];

	x = (int)pin;
	sprintf(temp, "%d", x);
	strcat(command, temp);
	temp[0] = '\0';
	x = strlen(command);
	command[x] = onOff ? '1' : '0';
	command[x+1] = '\n';
	command[x+2] = '\0';
	//printf("command: %s", command);
	write(fd, command, strlen(command));
}

//PWM: 3, 5, 6, 9
void setPWM(unsigned short pin, unsigned short value) {
	char command[4] = "2";
	int x;
	char temp[4];
	
	if (pin != 3 && pin != 5 && pin != 6 && 
		pin != 9) {
		printf("PWM is only for pin 3, 5, 6, 9\n");
		return;
	}
	x = (int)pin;
	sprintf(temp, "%d", x);
	strcat(command, temp);
	temp[0] = '\0';
	x = (int)value;
	sprintf(temp, "%d", x);
	strcat(command, temp);
	strcat(command, "\n");
	write(fd, command, strlen(command));
}
