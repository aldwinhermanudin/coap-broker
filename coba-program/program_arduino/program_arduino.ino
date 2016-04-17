#define _GPIO 0
#define _ADC 1
#define _PWM 2

int led = 13;
short i;
int sensorVal = 0;
String temp = "";
uint16_t inputSerial;
typedef struct serialInterface {
  unsigned char interface;
  unsigned char pin;
  unsigned char value;
} serialMasuk;

serialMasuk ser;
unsigned char incoming[3];

void setup() {                
  pinMode(led, OUTPUT);    
  Serial.begin(9600);
  for (i = 2; i <= 12; i++) {
   pinMode(i, OUTPUT);
   digitalWrite(i, LOW);
  } 
} 
    
void loop() {
  while(Serial.available() >= 3){
    for (int i = 0; i < 3; i++){
      incoming[i] = Serial.read();
    }
    ser.interface = incoming[0];
    ser.pin = incoming[1];
    ser.value = incoming[2];
    
    if (ser.interface == _GPIO) {
      digitalWrite((int)ser.pin, (ser.value != 0 ? 1 : 0));
      delay(5);
    }
    else if (ser.interface == _ADC) {
      sensorVal = analogRead((int)ser.pin);
      Serial.println(sensorVal);
      delay(250);
    }
    else if (ser.interface == _PWM) {
      analogWrite((int)ser.pin, (int)(ser.value * 255 / 100));
      delay(5);
    }
  }
  delay(5);
}

