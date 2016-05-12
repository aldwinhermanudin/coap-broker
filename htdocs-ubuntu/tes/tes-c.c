#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


char *str = "[{\"href\":\"/sensors\",\"ct\":\"40\",\"title\":\"Sensor Index\"},"
    "{\"href\":\"/sensors/temp\",\"rt\":\"temperature-c\",\"if\":\"sensor\",\"obs\":true},"
    "{\"href\":\"/sensors/light\",\"rt\":\"light-lux\",\"if\":\"sensor\"},"
    "{\"href\":\"http://www.example.com/sensors/t123\","
     "\"anchor\":\"/sensors/temp\",\"rel\":\"describedby\","
     "\"foo\":[\"bar\",\"3\"],\"ct\":\"4711\"},"
    "{\"href\":\"/t\",\"anchor\":\"/sensors/temp\",\"rel\":\"alternate\"}]";

char *response = "</sensors>;ct=40;title=\"Sensor Index\","
   "</sensors/temp>;rt=\"temperature-c\";if=\"sensor\";obs,"
   "</sensors/light>;rt=\"light-lux\";if=\"sensor\","
   "<http://www.example.com/sensors/t123>;anchor=\"/sensors/temp\""
   ";rel=\"describedby\";foo=\"bar\";foo=3;ct=4711,"
   "</t>;anchor=\"/sensors/temp\";rel=\"alternate\"";

char *tai = "adika "
	"bintang";

char *crot = "</sensors>;ct=40;title=\"Sensor Index\","
   "</sensors/temp>;rt=\"temperature-c\";if=\"sensor\","
   "</sensors/light>;rt=\"light-lux\";if=\"sensor\","
   "<http://www.example.com/sensors/t123>;anchor=\"/sensors/temp\""
   ";rel=\"describedby\","
   "</t>;anchor=\"/sensors/temp\";rel=\"alternate\"";

void json_encode_sensor(int val) {
	char str[32] = "{\"sensor\" : ";
	char temp[5];
	
	sprintf(temp, "%d", val);
	strcat(str, temp);
	strcat(str, "}");
	printf("%s", str);
}



void json_encode_observe(char *string) {
	int len = strlen(string);
	char hasil[1000] = "[";
	char temp[255];
	int h = 0, i = 0, j = 0, k = 0;
	bool obs = false;
		
	k = 0;
	for (i = 0; i < len; i++) {
		if (string[i] == '<') {
			strcat(hasil, "{\"href\":\"");
			i++;
			k = 0;
			while (string[i] != '>') {
				temp[k++] = string[i++];
			}
			temp[k] = '\0';
			strcat(hasil, temp);
			strcat(hasil, "\",");
			temp[0] = '\0';
			
		}
		
		if (string[i] == ';') {
			i++;
			k = 0;
			while (1) {
				if (string[i] == '=') {
					temp[k] = '\0';
					strcat(hasil, "\"");
					strcat(hasil, temp);
					strcat(hasil, "\":");
					if (string[i+1] != '"') { //cek ada petik duanya ga
						strcat(hasil, "\"");
					}
					
					temp[0] = '\0';
					k = 0;
					i++;
					obs = false;
					break;
				}
				if (string[i] == ';') {
					temp[k] = '\0';
					strcat(hasil, "\"");
					strcat(hasil, temp);
					strcat(hasil, "\"");
					if (temp[0] != '!') {			
						strcat(hasil, ":true");
					}
					else 
						strcat(hasil, ":false");
					
					temp[0] = '\0';
					k = 0;
					//i++;
					break;
				}
				if (string[i] == ',') {
					temp[k] = '\0';
					strcat(hasil, "\"");
					strcat(hasil, temp);
					strcat(hasil, "\"");
					if (temp[0] != '!') {			
						strcat(hasil, ":true");
					}
					else 
						strcat(hasil, ":false");
					
					strcat(hasil, "},");
					temp[0] = '\0';
					k = 0;
					//i++;
					obs = true;
					break;
				}
				temp[k++] = string[i++];
			}
			
			if (obs)
				continue;
			
			while (1) {
				if (string[i] == ';') {
					temp[k] = '\0';
					strcat(hasil, temp);
					if (string[i-1] != '"') { //cek ada petik duanya ga
						strcat(hasil, "\"");
					}
					strcat(hasil, ",");
					temp[0] = '\0';
					
					i--;
					break;
				}
				
				if (string[i] == ',') {
					temp[k] = '\0';
					strcat(hasil, temp);
					if (string[i-1] != '"') { //cek ada petik duanya ga
						strcat(hasil, "\"");
					}
					temp[0] = '\0';
					strcat(hasil, "},");
					
					i--;
					break;
				}
				
				temp[k++] = string[i++];
			}
			
		}
		
		
	}
	hasil[(int)strlen(hasil) - 2] = '\0';
	strcat(hasil, "}]");
	printf("%s\n", hasil);
}

int main() {
	printf("Content-Type: application/json\n");
	//printf("Status: 400 Bad Request\n\n");
	printf("Status: 200 OK\n\n");	
	//printf("{\"a\":1,\"b\":2,\"c\":3,\"d\":4,\"e\":5}");
	//printf("{\"temp\" : 813}");
	//json_encode_sensor(1023);	
	printf("%s\n", response);
	printf("\n-------------\n");
	json_encode_observe(response);
	printf("\n-------------\n");
	json_encode_observe(crot);
	return 0;
}
