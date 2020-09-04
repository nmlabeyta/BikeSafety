/*
 * Project test
 * Description:
 * Author:
 * Date:
 */


//---sd card libaries---//
#include <SPI.h>
#include <SdFat.h>
#include "uCAMIII.h"

uCamIII<USARTSerial> ucam(Serial1, A0, 500);                        // use HW Serial1 and A0 as reset pin for uCamIII
//-------SDcard varabiles and presetup code----//
const int chipSelect = A5;
#define FILE_BASE_NAME "JPEG"

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
char fileName[13] = FILE_BASE_NAME "00.jpg";
//==============================================================================
// Error messages stored in flash.
#define error(msg) sd.errorHalt(msg)
//------------------------------------------------------------------------------

// setup() runs once, when the device is first turned on.
void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  SDcardSetup();
  pinMode(D7, OUTPUT);
  ucam.init(115200);
  ucam.setImageFormat(uCamIII_COMP_JPEG,uCamIII_640x480);
  ucam.takeSnapshot(uCamIII_SNAP_JPEG,1);
  SDwriteFunction();
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  // The core of your code will likely live here.

}
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
  file.print(ucam.getPicture(uCamIII_TYPE_JPEG));
}
