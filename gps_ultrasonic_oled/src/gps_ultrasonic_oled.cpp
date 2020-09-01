/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#line 1 "c:/Users/Benja/Documents/IOT/BikeSafety/gps_ultrasonic_oled/src/gps_ultrasonic_oled.ino"
/*
 * Project GPSTrial
 * Description: Trial Program to Use TinyGPS++ on the Particle Argon with Display to OLED
 * Author: Brian Rashap
 * Date: 28-AUG-2020
 */

#include "Particle.h"
#include "TinyGPS++.h"

/*
 * GPS Module Connections
 * GPS Unit Rx connects to Argon Pin D9  (UART1_TX)
 * GPS Unit Tx connects to Argon Pin D10 (UART1_RX)
*/

#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"

void setup();
void loop();
void displayInfo();
void helloWorld();
void UltraSonicFunction();
void waitForEcho(int pin, int value, long timeout);
void sendTriggerPulse(int pin);
void doSomethingWhenDistanceIs(int distanceIs);
#line 20 "c:/Users/Benja/Documents/IOT/BikeSafety/gps_ultrasonic_oled/src/gps_ultrasonic_oled.ino"
#define OLED_RESET D7
Adafruit_SSD1306 display(OLED_RESET);

SYSTEM_THREAD(ENABLED);

const unsigned long PUBLISH_PERIOD = 120000;
const unsigned long SERIAL_PERIOD = 5000;
const unsigned long MAX_GPS_AGE_MS = 10000; // GPS location must be newer than this to be considered valid

// The TinyGPS++ object
TinyGPSPlus gps;
const int UTC_offset = -6; 
unsigned long lastSerial = 0;
unsigned long lastPublish = 0;
unsigned long startFix = 0;
bool gettingFix = false;

float lat,lon,alt;

bool beam_status = false;
float duration;
float cm = 0.0;
int trigPin = D4;
int echoPin = D5;

void setup()
{
	Serial.begin(9600);
  	pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);

	// The GPS module initialization
	Serial1.begin(9600);
    startFix = millis();
    gettingFix = true;


	// OLED Display initialization
	display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
	helloWorld();
}

void loop(){
	// UltraSonicFunction();
	while (Serial1.available() > 0) {
		if (gps.encode(Serial1.read())) {
			displayInfo();
		}
	}	
}

void displayInfo() {
	float lat,lon,alt;
	uint8_t hr,mn,se,sat;
	if (millis() - lastSerial >= SERIAL_PERIOD) {
		lastSerial = millis();

		char buf[128];
		if (gps.location.isValid() && gps.location.age() < MAX_GPS_AGE_MS) {
			lat = gps.location.lat();
			lon = gps.location.lng(); 
			alt = gps.altitude.meters();
			hr = gps.time.hour();
			mn = gps.time.minute();
			se = gps.time.second();
			sat = gps.satellites.value();

			if(hr > 7) {
				hr = hr + UTC_offset;
			}
			else {
				hr = hr + 24 + UTC_offset;
			}
			Serial.printf("%i satellites in view --- ",sat);
			Serial.printf("Time: %02i:%02i:%02i --- ",hr,mn,se);
			Serial.printf("lat: %f, long: %f, alt: %f \n", lat,lon,alt);
			if (gettingFix) {
				gettingFix = false;
				unsigned long elapsed = millis() - startFix;
				Serial.printlnf("%lu milliseconds to get GPS fix", elapsed);
			}
			display.clearDisplay();
			display.setCursor(0,0);
			display.printf("Time: %02i:%02i:%02i \n",hr,mn,se);
			display.printf("lat  %f \nlong %f \nalt %f\n", lat,lon,alt);
			display.printf("Satellites in view: %i \n",sat);
			Serial.printf("Duration = %0.2f, Distance in CM: %0.2f \n",duration,cm);
  			display.println(" ");
  		    display.printf("Duration = %0.2f, Distance in CM: %0.2f \n",duration,cm);
  			display.display();
			display.display();
		}
		else {
			strcpy(buf, "no location");
			if (!gettingFix) {
				gettingFix = true;
				startFix = millis();
			}
		}
	}
}

void helloWorld() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(20,5);
  display.println("UltraSonic initializing");
  display.display();
}

void UltraSonicFunction(){
  //----- this calculates the disance every 3 to 4 seconds -----//
  sendTriggerPulse(trigPin);
  waitForEcho(echoPin, HIGH, 100);
  long startTime = micros();
  waitForEcho(echoPin, LOW, 100);
  long endTime = micros();
  duration = endTime - startTime;
  cm = duration / 58.0; //the speed of sound?//
//     Serial.printf("Duration = %0.2f, Distance in CM: %0.2f \n",duration,cm);
//    display.clearDisplay();
//  	display.setCursor(0,0);
//     display.printf("Duration = %0.2f, Distance in CM: %0.2f \n",duration,cm);
//     display.display();
	delay(200);
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