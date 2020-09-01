/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "c:/Users/Benja/Documents/IOT/BikeSafety/SDcard/src/SDcard.ino"
/*
 * Project SDcard 
 * Description: Log Data to an SDCard
 * Author: Brian Rashap
 * Date: 26-APR-2020
 */

#include <SPI.h>
#include <SdFat.h>

void setup();
void loop();
void logData2(long timeLog, int data1);
#line 11 "c:/Users/Benja/Documents/IOT/BikeSafety/SDcard/src/SDcard.ino"
const int chipSelect = SS;
//const int SAMPLE_INTERVAL_uS = 50;

#define FILE_BASE_NAME "Data"

// Create file system object.
SdFat sd;
SdFile file;

// Time in micros for next data record.
unsigned long logTime;
unsigned long startTime;
int i;

// ButtonState
bool logStart;
const int startPin = D2;

const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;
char fileName[13] = FILE_BASE_NAME "00.csv";

//==============================================================================
// Error messages stored in flash.
#define error(msg) sd.errorHalt(msg)
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
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


void loop() {
logStart=true; // use a button nomrally
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

    for (i=0;i<100;i++) {

      logTime = micros() - startTime;
      Serial.print(".");
      logData2(logTime,random(0,100));

      // Force data to SD and update the directory entry to avoid data loss.
      if (!file.sync() || file.getWriteError()) {
      Serial.printf("write error");
      }
      delay(random(100,500));
    }
    logStart =  false;   // button release
    if (logStart==false) {
      file.close();
      Serial.printf("Done \n");
      delay(2000);
      Serial.printf("Ready for next data log \n");
    }
  }
}  

void logData2(long timeLog, int data1) {
  file.printf("%i , %i \n",timeLog,data1);
}
