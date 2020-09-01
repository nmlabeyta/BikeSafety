/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "c:/Users/Benja/Documents/IOT/BikeSafety/Ultrasonic_HC-SR04/src/Ultrasonic_HC-SR04.ino"
/*
 * Project UltraSonicSensor
 * Description: writing code for an alarm to go off at 1 meter
 * Author: Benjamin Hansen
 * Date: 08/27/2020
 */

void setup();
void loop();
void UltraSonicFunction();
void waitForEcho(int pin, int value, long timeout);
void sendTriggerPulse(int pin);
void doSomethingWhenDistanceIs(int distanceIs);
void timerfunction(int timer);
#line 8 "c:/Users/Benja/Documents/IOT/BikeSafety/Ultrasonic_HC-SR04/src/Ultrasonic_HC-SR04.ino"
SYSTEM_MODE(SEMI_AUTOMATIC);

//--------ultraSonic variables------///
bool beam_status = false;
float cm = 0.0;
int trigPin = D4;
int echoPin = D5;

void setup() {
  Serial.begin(9600);
  delay(500);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void loop() {
  // UltraSonicFunction();
  doSomethingWhenDistanceIs(125);
}

void UltraSonicFunction(){
  //----- this calculates the disance every 3 to 4 seconds -----//
  sendTriggerPulse(trigPin);
  waitForEcho(echoPin, HIGH, 100);
  long startTime = micros();
  waitForEcho(echoPin, LOW, 100);
  long endTime = micros();
  float duration = endTime - startTime;
  cm = duration / 58.0; //the speed of sound?//
    Serial.printf("Duration = %0.2f, Distance in CM: %0.2f \n",duration,cm);
    delay(2000);
}

void waitForEcho(int pin, int value, long timeout){
  //-----this listens for the pulse to return---//
    long giveupTime = millis() + timeout;
    while (digitalRead(pin) != value && millis() < giveupTime) {}
}

void sendTriggerPulse(int pin){
  //------this sends the pulse----//
    digitalWrite(pin, HIGH);
    delayMicroseconds(10);
    digitalWrite(pin, LOW);
}

void doSomethingWhenDistanceIs(int distanceIs){
  //-----this is the alarm set to go off at 1 meter----//
  UltraSonicFunction();
    if (cm<distanceIs){
        if (beam_status==false){
            
            Serial.println("less than 125cm");
            beam_status = true;   
        }
    } else {
        if (beam_status==false){
            
        } else {
            Serial.print("clear");
            beam_status = false;
        }
    }
    delay(1000);
}
void timerfunction(int timer){
  //----this is a timer to be used instead of delays-----//
  int currentTime = millis();
  int lastSecond;
  while ((currentTime - lastSecond) < timer){
         //do nothing//
  }
  lastSecond = millis();
}
