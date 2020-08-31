//------Bike Safety Project---//
// ----Benjamin Hansen & Lee Abeyata
//-----08/2020

//---gps libaries---//
#include "Particle.h"
#include "TinyGPS++.h"
/*
 * GPS Module Connections
 * GPS Unit Rx connects to Argon Pin D9  (UART1_TX)
 * GPS Unit Tx connects to Argon Pin D10 (UART1_RX)
*/
//----- The TinyGPS++ object  ----//
TinyGPSPlus gps;
SYSTEM_THREAD(ENABLED);

//--------ultraSonic variables------///
bool beam_status = false;
float cm = 0.0;
int trigPin = D4;
int echoPin = D5;

//------GPS variables-----//
const unsigned long PUBLISH_PERIOD = 120000;
const unsigned long SERIAL_PERIOD = 5000;
const unsigned long MAX_GPS_AGE_MS = 10000; // GPS location must be newer than this to be considered valid
const int UTC_offset = -6; 
unsigned long lastSerial = 0;
unsigned long lastPublish = 0;
unsigned long startFix = 0;
bool gettingFix = false;
float lat,lon,alt;


void setup() {
  Serial.begin(9600);
  delay(500);
  UltraSonicSetup();
  GPSsetup();

}

void loop() {
  ifGPSAvailbleDisplay();
  doSomethingWhenDistanceIs(125);
}
//----ultrasonic code----//
void UltraSonicSetup(){
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
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
  //-----this is the alarm set to go off at int distance----//
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
//----this section is for the gps code ----//
	// The GPS module initialization
void GPSsetup(){
  Serial1.begin(9600);
  startFix = millis();
  gettingFix = true;
  }
 void ifGPSAvailbleDisplay(){
   	while (Serial1.available() > 0) {
		if (gps.encode(Serial1.read())) {
			printGPSInfo();
		}
	}
 } 
 void printGPSInfo() {
	float lat,lon,alt;
	uint8_t hr,mn,se;
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

			if(hr > 7) {
				hr = hr + UTC_offset;
			}
			else {
				hr = hr + 24 + UTC_offset;
			}
			Serial.printf("Time: %02i:%02i:%02i --- ",hr,mn,se);
			Serial.printf("lat: %f, long: %f, alt: %f \n", lat,lon,alt);
			if (gettingFix) {
				gettingFix = false;
				unsigned long elapsed = millis() - startFix;
				Serial.printlnf("%lu milliseconds to get GPS fix", elapsed);
			}

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
