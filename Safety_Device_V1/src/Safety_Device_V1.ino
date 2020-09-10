/*
 * Project Safety_Device_V1
 * Description: Final
 * Author: Lee Abeyta/Benjamin Hanson
 * Date: 9/4/20
 */

//---Adafruit IO Libraries---//
#include "secret.h"
#include <Adafruit_MQTT.h>
#include "Adafruit_MQTT/Adafruit_MQTT.h"
#include "Adafruit_MQTT/Adafruit_MQTT_SPARK.h"
#include "Adafruit_MQTT/Adafruit_MQTT.h"

//---sd card libaries---//
#include <SPI.h>
#include <SdFat.h>

//-----gps libaries----//
#include "Particle.h"
#include "TinyGPS++.h"

//#include "Adafruit_GFX.h"
//#include "Adafruit_SSD1306.h"

//---oled code---//
//#define OLED_RESET D4
//Adafruit_SSD1306 display(OLED_RESET);

/*
 * GPS Module Connections
 * GPS Unit Rx connects to Argon Pin D9  (UART1_TX)
 * GPS Unit Tx connects to Argon Pin D10 (UART1_RX)
*/

//---gps constants ---//
SYSTEM_THREAD(ENABLED);
const unsigned long PUBLISH_PERIOD = 120000;
const unsigned long SERIAL_PERIOD = 5000;
const unsigned long MAX_GPS_AGE_MS = 10000; // GPS location must be newer than this to be considered valid

/************ Global State (you don't need to change this!) ***   ***************/
TCPClient TheClient;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_SPARK mqtt(&TheClient,AIO_SERVER,AIO_SERVERPORT,AIO_USERNAME,AIO_KEY);

/****************************** Feeds ***************************************/
// Setup Feeds to publish or subscribe
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish latWrite = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Latitude");
Adafruit_MQTT_Publish longWrite = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Longitude");
Adafruit_MQTT_Publish altWrite = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Altitude");
Adafruit_MQTT_Publish distanceWrite = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Distance in CM");

// The TinyGPS++ object
TinyGPSPlus gps;
//---GPS variables ---//
const int UTC_offset = -6; 
unsigned long lastSerial = 0;
unsigned long lastPublish = 0;
unsigned long startFix = 0;
bool gettingFix = false;
float lat,lon,alt;
uint8_t hr,mn,se,sat, dy, mth, yr;

//--------ultraSonic variables------///
bool beam_status = false;
float cm = 0.0;
int trigPin = D4;
int echoPin = D5;

//-------SDcard varabiles and presetup code----//
const int chipSelect = D14;
//const int SAMPLE_INTERVAL_uS = 50;

#define FILE_BASE_NAME "Data"

// Create file system object.
SdFat sd;
SdFile file;

// Time in micros for next data record.
unsigned long logTime;
unsigned long startTime;
int i;
bool logStart; // ButtonState
const int startPin = D2;
const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;
char fileName[13] = FILE_BASE_NAME "00.csv";
//==============================================================================
// Error messages stored in flash.
#define error(msg) sd.errorHalt(msg)
//------------------------------------------------------------------------------

void setup()
{
	Serial.begin(9600);

	delay(500);
   pinMode(trigPin, OUTPUT);
   pinMode(echoPin, INPUT);

	Serial1.begin(9600);
    startFix = millis();
    gettingFix = true;
	
	// OLED Display initialization
	//display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
	//helloWorld();
  
  SDcardSetup();

	delay(500);
	Serial.println("attempt 20");
	delay(500);

}

void loop()
{
  packetGroper();
  MQTT_connect();
	UltraSonicFunction();
  
	while (Serial1.available() > 0) {
		if (gps.encode(Serial1.read())) {
			displayInfo();
      doSomethingWhenDistanceIs(100);
		}
	}

SDwriteFunction();
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return; 
  }

  Serial.print("Connecting to MQTT... ");

  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
  }
  Serial.println("MQTT Connected!");
}


void packetGroper(){
 static unsigned long last;
 static unsigned long currentMil;

  currentMil = millis();
  if ((millis()-last)>120000) {
      Serial.printf("Pinging MQTT \n");
      if(! mqtt.ping()) {
        Serial.printf("Disconnecting \n");
        mqtt.disconnect();
      }
      last = currentMil;
  }
}

void displayInfo() {
	// float lat,lon,alt;
	// uint8_t hr,mn,se,sat;
	if (millis() - lastSerial >= SERIAL_PERIOD) {
		lastSerial = millis();

		char buf[128];
		Serial.println(gps.location.isValid());
	    Serial.println(gps.location.age());
		if (gps.location.isValid() && gps.location.age() < MAX_GPS_AGE_MS) {
			lat = gps.location.lat();
			lon = gps.location.lng(); 
			alt = gps.altitude.meters();
			hr = gps.time.hour();
			mn = gps.time.minute();
			se = gps.time.second();
			sat = gps.satellites.value();
      dy - gps.date.day();
      mth = gps.date.month();
      yr = gps.date.year();
			if(hr > 7) {
				hr = hr + UTC_offset;
			}
			else {
				hr = hr + 24 + UTC_offset;
			}
			Serial.printf("Distance in CM: %0.2f \n",cm);
			Serial.printf("%i satellites in view --- ",sat);
			Serial.printf("Time: %02i:%02i:%02i --- ",hr,mn,se);
			Serial.printf("lat: %f, long: %f, alt: %f \n", lat,lon,alt);
			if (gettingFix) {
				gettingFix = false;
				unsigned long elapsed = millis() - startFix;
				Serial.printlnf("%lu milliseconds to get GPS fix", elapsed);
			}
      
			/*display.clearDisplay();
			display.setCursor(0,0);
			display.printf("Time: %02i:%02i:%02i \n",hr,mn,se);
			display.printf("lat  %f \nlong %f \nalt %f\n", lat,lon,alt);
			display.printf("Satellites in view: %i \n",sat);
			display.println("");
			display.printf("Distance in CM: %0.2f \n",cm);
			display.display(); 
      */
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

/*void helloWorld() {
  display.clearDisplay();
	display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(20,5);
  display.println("GPS Initializing");
	display.display();
}
*/
void UltraSonicFunction(){
  //----- this calculates the disance every 3 to 4 seconds -----//
  sendTriggerPulse(trigPin);
  waitForEcho(echoPin, HIGH, 100);
  long startTime = micros();
  waitForEcho(echoPin, LOW, 100);
  long endTime = micros();
  float duration = endTime - startTime;
  cm = duration / 58.0; //the speed of sound?//
    // Serial.printf("Duration = %0.2f, Distance in CM: %0.2f \n",duration,cm);
    delay(100);
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
  // UltraSonicFunction();
    if (cm<distanceIs){
        if (beam_status==false){
            
            Serial.println("less than 125cm");
            logStart = !logStart;
            beam_status = true;   
            subscribePublish();
        }
    } else {
        if (beam_status==false){
            
        } else {
            Serial.print("clear \n");
            beam_status = false;
        }
    }
    delay(200);
}
// void timerfunction(int timer){
//   //----this is a timer to be used instead of delays-----//
//   int currentTime = millis();
//   int lastSecond;
//   while ((currentTime - lastSecond) < timer){
//          //do nothing//
//   }
//   lastSecond = millis();
// }
void SDcardSetup(){
	logStart = false;
  // Initialize at the highest speed supported by the board that is
  // not over 50 MHz. Try a lower speed if SPI errors occur.cc
  if (!sd.begin(chipSelect, SD_SCK_MHZ(50))) {
    sd.initErrorHalt();  
  }

  if (BASE_NAME_SIZE > 6) {
    Serial.println("FILE_BASE_NAME too long");
    while(1);
  }
  // Write data header.
  file.printf("TimeStamp, Random Data %i \n");
  // Start on a multiple of the sample interval.
}
void SDwriteFunction(){
  // logStart=true; // use a button nomrally
if (logStart==true) {
  Serial.printf("Starting Data Logging \n");
  while (sd.exists(fileName)) {
    if (fileName[BASE_NAME_SIZE + 1] != '9') {
      fileName[BASE_NAME_SIZE + 1]++;
    } else if (fileName[BASE_NAME_SIZE] != '9') {
      fileName[BASE_NAME_SIZE + 1] = '0';
      fileName[BASE_NAME_SIZE]++;
    } else {
      Serial.println("Can't create file name");
      while(1);
    }
  }
  if (!file.open(fileName, O_WRONLY | O_CREAT | O_EXCL)) {
    Serial.println("file.open");
  }
  Serial.printf("Logging to: %s \n",fileName);
  startTime = micros();
}
  while(logStart==true) {
    // for (i=0;i<100;i++) {

      // logTime = micros() - startTime;
      Serial.print(".");
      
      logData2();

      // Force data to SD and update the directory entry to avoid data loss.
      if (!file.sync() || file.getWriteError()) {
      Serial.printf("write error \n");
      }
      delay(random(100,500));
    // }
    logStart = false;   // button release
    if (logStart==false) {
      file.close();
      Serial.printf("Done \n");
      delay(2000);
      Serial.printf("Ready for next data log \n");
    }
  }
}
void logData2() {
 
  Serial.print("Writing data to SDcard \n");
  file.printf("Day: %i, Month %i, Year %i \n", dy, mth, yr);
  file.printf("Time: %02i:%02i:%02i \n",hr,mn,se);
	file.printf("lat  %f \nlong %f \nalt %f\n", lat,lon,alt);
	file.printf("Satellites in view: %i \n",sat);
	file.println("");
	file.printf("Distance in CM: %0.2f \n",cm);
  subscribePublish();
}

void subscribePublish(){


  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here
    
    if(mqtt.Update()) {
     latWrite.publish(lat);
     longWrite.publish(lon);
    //  altWrite.publish(alt);
     distanceWrite.publish(cm);
      Serial.printf("Publishing Data \n");
      }
  }