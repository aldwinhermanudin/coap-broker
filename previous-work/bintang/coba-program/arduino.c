int led = 13;
int n = 100;
int sensorADC1 = A0;
int sensorVal = 0;
char c;

void setup() {                
  pinMode(led, OUTPUT);    
  Serial.begin(9600); 
  digitalWrite(led, HIGH);
} 
    
void loop() {
  if (Serial.available()) {
    c = Serial.read();
    n = c - '0';
    Serial.println(c);
    switch (c) {
      case 'A':
      case 'a':
        sensorVal = analogRead(A0);
        Serial.println(sensorVal);
        delay(500);
        break;
      case 'G':
      case 'g':
        digitalWrite(led, HIGH);
        break;  
       case 'o':
       case 'O':
        digitalWrite(led, LOW);
        break;
       case 'P':
       case 'p':
         //pwm
         break;
    }
    delay(20);             
  }
  delay(20);
}
