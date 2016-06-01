#define _ADC 0
#define _GPIO 1
#define _PWM 2
#define _FLOWMETER 3
#define FLOW_PIN 2

char incoming[7];
int sensorVal = 0, i = 0, k, pin = 0, val = 0;
byte sensorInterrupt = 0;  // 0 = digital pin 2

// The hall-effect flow sensor outputs approximately 4.5 pulses per second per
// litre/minute of flow.
float calibrationFactor = 4.5;

volatile byte pulseCount;

float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

unsigned long oldTime;

char incomingByte;
String str, str1;

void setup()
{
    Serial.begin(9600);
    pinMode(3, OUTPUT);
    digitalWrite(3, LOW);

    pinMode(FLOW_PIN, INPUT);
    digitalWrite(FLOW_PIN, HIGH);

    pulseCount        = 0;
    flowRate          = 0.0;
    flowMilliLitres   = 0;
    totalMilliLitres  = 0;
    oldTime           = 0;

    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
}

void loop() {
    if (Serial.available() > 0) {   

        i = 0;
        while(1) {            
            incoming[i] = Serial.read();
            if (incoming[i] == '\n') break;   
            if (incoming[i] == -1) continue;  
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
        else if (incoming[0] == _FLOWMETER) {
            Serial.println(flowMilliLitres);
        }

    }

    if((millis() - oldTime) > 1000)  
    {
        detachInterrupt(sensorInterrupt);

        flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;

        oldTime = millis();

        flowMilliLitres = (flowRate / 60) * 1000;
       
        pulseCount = 0;

        attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
    }
}

void pulseCounter()
{
    pulseCount++;
}
