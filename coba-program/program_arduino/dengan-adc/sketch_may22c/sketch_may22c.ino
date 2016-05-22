#define _ADC 0
#define _GPIO 1
#define _PWM 2

char incoming[7];
int sensorVal = 0, i = 0, k, pin = 0, val = 0;

void setup()
{
  Serial.begin(9600);
  for (i = 2; i <= 12; i++) {
   pinMode(i, OUTPUT);
   digitalWrite(i, LOW);
  } 
}

char incomingByte;
String str, str1;
 
void loop() {
  if (Serial.available() > 0) {   // something came across serial
    
    i = 0;
    while(1) {            // force into a loop until 'n' is received
      incoming[i] = Serial.read();
      if (incoming[i] == '\n') break;   // exit the while(1), we're done receiving
      if (incoming[i] == -1) continue;  // if no characters are in the buffer read() returns -1
      incoming[i] -= '0';
     i++;
   }
   if (incoming[0] == _ADC) {
     sensorVal = analogRead((int)incoming[1]);
     delay(1);
     //str1 = "ADC ";
     //str = sensorVal;
     Serial.println(sensorVal); 
   }
   else if (incoming[0] == _GPIO) {
     if (i == 3) {
       digitalWrite((int)incoming[1], (int)incoming[2]);
       delay(1);
     }
     else if (i == 4) {
       pin = (incoming[1]) * 10 + (incoming[2]);
       
       digitalWrite((int)incoming[1], (int)incoming[3]);
     }
   }
   //pwm buat 3, 6, 9 aja
   else if (incoming[0] == _PWM) {
     val = 0;
     for (k = 2; k < i; k++) {
       val *= 10;
       val += (incoming[k]);
     }
     analogWrite((int)incoming[1], val); 
   }
   else if (incoming[0] == 9) {
    Serial.println("tai"); 
   }
    
  }
}
