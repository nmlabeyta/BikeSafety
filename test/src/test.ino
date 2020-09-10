/*
 * Project test
 * Description:
 * Author:
 * Date:
 */
/*

//---sd card libaries---//
#include <SPI.h>
#include <SdFat.h>
#include "uCAMIII.h"

long prepareCam(uCamIII_SNAP_TYPE snap    = uCamIII_SNAP_RAW, 
                uCamIII_IMAGE_FORMAT fmt  = uCamIII_RAW_8BIT, 
                uCamIII_RES res           = uCamIII_80x60, 
                uCamIII_CBE contrast      = uCamIII_DEFAULT,
                uCamIII_CBE brightness    = uCamIII_DEFAULT,
                uCamIII_CBE exposure      = uCamIII_DEFAULT,
                uCamIII_callback callback = NULL);

uCamIII<USARTSerial> ucam(Serial1, A0, 500);                        // use HW Serial1 and A0 as reset pin for uCamIII
//-------SDcard varabiles and presetup code----//
const int chipSelect = D14;
#define FILE_BASE_NAME "PIC"

// Create file system object.
SdFat sd;
SdFile file;

// Time in micros for next data record.
unsigned long logTime;
unsigned long startTime;
int i;
int offset;
bool logStart; // ButtonState
const int startPin = D2;
const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;
char fileName[13] = FILE_BASE_NAME "00.jpg";
//==============================================================================
// Error messages stored in flash.
#define error(msg) sd.errorHalt(msg)
//------------------------------------------------------------------------------
uCamIII_callback snapTarget = callbackSerial;
uint8_t     imageBuffer[160*120*2 + 134];                           // max raw image 160x120x2byte + max BMP header size
int         imageSize     = 0;
int         bmpHeader(uint8_t *buf, int len, int width, int height, int bits, int encoding = 0, int padding = 0);
int         imageType     = uCamIII_TYPE_JPEG; 
int         imageSnapType = uCamIII_SNAP_RAW;
int         imageWidth    = 0;
int         imageHeight   = 0;
int         imagePxDepth  = 0;
int         imageTime     = 0;
char        lIP[16];

// setup() runs once, when the device is first turned on.
void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  SDcardSetup();
  pinMode(D7, OUTPUT);
  ucam.init(115200);
  Serial.printf("Picture being taken\n");
  setSnapshotTarget();
  ucam.setImageFormat(uCamIII_COMP_JPEG,uCamIII_640x480);
  ucam.takeSnapshot(uCamIII_SNAP_JPEG,0);
  takeSnapshot();
  Serial.print("Picture Taken\n");
  Serial.printf("Image Size: %i\n",imageSize);
  Serial.printf("Buffer: %i\n",imageBuffer);
  logStart = true;
  SDwriteFunction();
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  // The core of your code will likely live here.

}

void setSnapshotTarget()
{
  snapTarget = callbackSerial;                            // default to Serial
}

int callbackSerial(uint8_t *buf, int len, int id)
{
  
  return Serial.write(buf, len);
}

void takeSnapshot() 
{
  long prep = prepareCam(uCamIII_SNAP_JPEG, uCamIII_COMP_JPEG, uCamIII_640x480, 
                       uCamIII_DEFAULT, uCamIII_DEFAULT, uCamIII_DEFAULT, snapTarget);

 imageSize = ucam.getPicture(uCamIII_TYPE_JPEG);

    if (imageType == uCamIII_SNAP_JPEG)
      for (int received = 0, chunk = 0; (received < imageSize) && (chunk = ucam.getJpegData(&imageBuffer[constrain(received, 0, sizeof(imageBuffer)-512)], 512, snapTarget)); received += chunk);
    /*else if (imageSize <= sizeof(imageBuffer)) 
    {
     offset = bmpHeader(imageBuffer, sizeof(imageBuffer), imageWidth, imageHeight, imagePxDepth);

      if (imagePxDepth == 16) 
      {
        for (int i = offset; i < imageSize + offset; i += 2)    // raw image comes big-endian and upside down from cam,
        {                                                       // this block corrects endianness
          uint8_t dmy = imageBuffer[i];
          imageBuffer[i] = imageBuffer[i+1];
          imageBuffer[i+1] = dmy;
        }
      }
      snapTarget(imageBuffer, imageSize + offset, 0);
    }

long prepareCam(uCamIII_SNAP_TYPE snap, uCamIII_IMAGE_FORMAT fmt, uCamIII_RES res, 
                 uCamIII_CBE contrast, uCamIII_CBE brightness, uCamIII_CBE exposure,
                 uCamIII_callback callback)
{
  int retVal = 0;
  
  digitalWrite(D7, HIGH);

  ucam.hardReset();

  if (!(retVal = ucam.sync())) return -1;
  
  if (!(retVal = ucam.setImageFormat(fmt, res))) return -2;
  
  retVal = ucam.setCBE(contrast, brightness, exposure);

  if (imageType == uCamIII_TYPE_SNAPSHOT)
    if (!(retVal = ucam.takeSnapshot(snap))) return -3;

  imageTime = Time.now();

  if (fmt == uCamIII_COMP_JPEG) 
    if (!(retVal = ucam.setPackageSize(512))) return -4;

  // if we made it to here, we can set the global image variables accordingly 
  imageType = snap;             
  
  switch(res) 
  {
    case uCamIII_80x60:
      imageWidth  = 80;
      imageHeight = 60;
      break;
    case uCamIII_160x120:          
      imageWidth  = 160;
      imageHeight = 120;
      break;
    case uCamIII_320x240:
      imageWidth  = 320;
      imageHeight = 240;
      break;
    case uCamIII_640x480:
      imageWidth  = 640;
      imageHeight = 480;
      break;
    case uCamIII_128x96:
      imageWidth  = 128;
      imageHeight = 96;
      break;
    case uCamIII_128x128:
      imageWidth  = 128;
      imageHeight = 128;
      break;
    default:
      imageWidth  = 80;
      imageHeight = 60;
      break;
  }

  switch (fmt)
  {
    case uCamIII_COMP_JPEG:
      imagePxDepth = 24;
      break;
    case uCamIII_RAW_16BIT_RGB565:
    case uCamIII_RAW_16BIT_CRYCBY:
      imagePxDepth = 16;
      break;
    case uCamIII_RAW_8BIT:
    defaut:
      imagePxDepth = 8;
      break;
  }
  
  return retVal;
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
  int len = imageSize + offset;
  Serial.print("Writing data to SDcard \n");
  file.write(imageBuffer, imageSize);
}

*/


#include "uCamIII.h"
#include "WebServer.h"
#include "WebPage.h"
#include "TCPClientX.h"

// uncomment for debugging
//SerialLogHandler traceLog(LOG_LEVEL_TRACE);

long prepareCam(uCamIII_SNAP_TYPE snap    = uCamIII_SNAP_RAW, 
                uCamIII_IMAGE_FORMAT fmt  = uCamIII_RAW_8BIT, 
                uCamIII_RES res           = uCamIII_80x60, 
                uCamIII_CBE contrast      = uCamIII_DEFAULT,
                uCamIII_CBE brightness    = uCamIII_DEFAULT,
                uCamIII_CBE exposure      = uCamIII_DEFAULT,
                uCamIII_callback callback = NULL);

uCamIII<USARTSerial> ucam(Serial1, A0, 500);                        // use HW Serial1 and A0 as reset pin for uCamIII
// or
// uCamIII<ParticleSoftSerial> ucam(new ParticleSoftSerial(D0, D1));
// or
// ParticleSoftSerial pss(D0, D1);
// uCamIII<ParticleSoftSerial> ucam(pss);

uint8_t     imageBuffer[160*120*2 + 134];                           // max raw image 160x120x2byte + max BMP header size
int         imageSize     = 0;
int         imageType     = uCamIII_TYPE_SNAPSHOT;                  // default for the demo 
                                                                    // alternative: _TYPE_RAW & _TYPE_JPEG
int         imageSnapType = uCamIII_SNAP_RAW;
int         imageWidth    = 0;
int         imageHeight   = 0;
int         imagePxDepth  = 0;
int         imageTime     = 0;
char        lIP[16];


IPAddress   serverAddr;
int         serverPort;
char        nonce[34];
#if Wiring_WiFi
  TCPClientX client(1024, 100);
#elif Wiring_Cellular
  TCPClientX client( 512, 100);
#endif
uCamIII_callback snapTarget = callbackSerial;

int         sendImageTCP(const uint8_t *buf, int len, int chunkSize = 512, uint32_t flushTime = 100);
int         bmpHeader(uint8_t *buf, int len, int width, int height, int bits, int encoding = 0, int padding = 0);

void setup() {
  Time.zone(+2.0);
    
  Particle.function("setServer", devicesHandler);
  Particle.function("setTarget", setSnapshotTarget);
  Particle.function("snap", takeSnapshot);

  pinMode(D7, OUTPUT);
  ucam.init(115200);

#if Wiring_WiFi
  strncpy(lIP, String(WiFi.localIP()), sizeof(lIP));
  Particle.variable("IP", lIP);

  webserver.setDefaultCommand(&defaultCmd);
  webserver.addCommand("index.html", &defaultCmd);
  webserver.addCommand("image.dat", &imageCmd);
  webserver.begin();
#endif
}





void loop() {
  static uint32_t msSend = 0;    

#if Wiring_WiFi
  char buff[64];
  int len = 64;
  webserver.processConnection(buff, &len);
#endif
}

int devicesHandler(String data) 
{
  Log.trace(__FUNCTION__); 

  int addr[4];
  Log.trace("devicesHandler data=%s", data.c_str());

                                                                                        // nonce optional
  if (sscanf(data, "%u.%u.%u.%u:%u,%32s", &addr[0], &addr[1], &addr[2], &addr[3], &serverPort, nonce) >= 5) {
	serverAddr = IPAddress(addr[0], addr[1], addr[2], addr[3]);
	Log.info("serverAddr=%s serverPort=%u nonce=%s", (const char*)serverAddr.toString(), serverPort, nonce);
	return serverPort;
  }
  return 0;
}

int setSnapshotTarget(String target)
{
  if (target.equalsIgnoreCase("tcp"))
  {
    snapTarget = callbackTCP;
    return 1;
  }
  else 
    snapTarget = callbackSerial;                            // default to Serial
    
  return 0;
}

int takeSnapshot(String format) 
{
  Log.trace(__FUNCTION__); 

  int retVal;
  
  if (format.equalsIgnoreCase("JPG") || format.equalsIgnoreCase("JPEG"))
    retVal = prepareCam(uCamIII_SNAP_JPEG, uCamIII_COMP_JPEG, uCamIII_640x480, 
                       uCamIII_DEFAULT, uCamIII_DEFAULT, uCamIII_DEFAULT, snapTarget);
  else if (format.equalsIgnoreCase("RGB16"))
    retVal = prepareCam(uCamIII_SNAP_RAW, uCamIII_RAW_16BIT_RGB565, uCamIII_160x120, 
                       uCamIII_DEFAULT, uCamIII_DEFAULT, uCamIII_DEFAULT, snapTarget);
  else if (format.equalsIgnoreCase("UYVY16") || format.equalsIgnoreCase("CrYCbY16"))
    retVal = prepareCam(uCamIII_SNAP_RAW, uCamIII_RAW_16BIT_CRYCBY, uCamIII_160x120, 
                       uCamIII_DEFAULT, uCamIII_DEFAULT, uCamIII_DEFAULT, snapTarget);
  else // default to "GRAY8"
    retVal = prepareCam(uCamIII_SNAP_RAW, uCamIII_RAW_8BIT, uCamIII_160x120, 
                       uCamIII_DEFAULT, uCamIII_DEFAULT, uCamIII_DEFAULT, snapTarget);

  if (retVal <= 0) return retVal;
  
  if (retVal = imageSize = ucam.getPicture((uCamIII_PIC_TYPE)imageType))
  {
    Log.info("\r\nImageSize: %d", imageSize);

    if (imageType == uCamIII_SNAP_JPEG)
      for (int received = 0, chunk = 0; (received < imageSize) && (chunk = ucam.getJpegData(&imageBuffer[constrain(received, 0, sizeof(imageBuffer)-512)], 512, snapTarget)); received += chunk);
    else if (imageSize <= sizeof(imageBuffer)) 
    {
      int offset = bmpHeader(imageBuffer, sizeof(imageBuffer), imageWidth, imageHeight, imagePxDepth);
      retVal     = ucam.getRawData(&imageBuffer[offset], imageSize, NULL);

      if (imagePxDepth == 16) 
      {
        for (int i = offset; i < imageSize + offset; i += 2)    // raw image comes big-endian and upside down from cam,
        {                                                       // this block corrects endianness
          uint8_t dmy = imageBuffer[i];
          imageBuffer[i] = imageBuffer[i+1];
          imageBuffer[i+1] = dmy;
        }
      }
      snapTarget(imageBuffer, imageSize + offset, 0);
    }

    if (client.connected())                                     // if the TCP client would still be connected
      client.stop();                                            // stop the connection

    digitalWrite(D7, LOW);

    return retVal;
  }

  return 0;
}

long prepareCam(uCamIII_SNAP_TYPE snap, uCamIII_IMAGE_FORMAT fmt, uCamIII_RES res, 
                 uCamIII_CBE contrast, uCamIII_CBE brightness, uCamIII_CBE exposure,
                 uCamIII_callback callback)
{
  int retVal = 0;
  
  digitalWrite(D7, HIGH);

  ucam.hardReset();

  if (!(retVal = ucam.sync())) return -1;
  
  if (!(retVal = ucam.setImageFormat(fmt, res))) return -2;
  
  retVal = ucam.setCBE(contrast, brightness, exposure);

  if (imageType == uCamIII_TYPE_SNAPSHOT)
    if (!(retVal = ucam.takeSnapshot(snap))) return -3;

  imageTime = Time.now();

  if (fmt == uCamIII_COMP_JPEG) 
    if (!(retVal = ucam.setPackageSize(512))) return -4;

  // if we made it to here, we can set the global image variables accordingly 
  imageType = snap;             
  
  switch(res) 
  {
    case uCamIII_80x60:
      imageWidth  = 80;
      imageHeight = 60;
      break;
    case uCamIII_160x120:          
      imageWidth  = 160;
      imageHeight = 120;
      break;
    case uCamIII_320x240:
      imageWidth  = 320;
      imageHeight = 240;
      break;
    case uCamIII_640x480:
      imageWidth  = 640;
      imageHeight = 480;
      break;
    case uCamIII_128x96:
      imageWidth  = 128;
      imageHeight = 96;
      break;
    case uCamIII_128x128:
      imageWidth  = 128;
      imageHeight = 128;
      break;
    default:
      imageWidth  = 80;
      imageHeight = 60;
      break;
  }

  switch (fmt)
  {
    case uCamIII_COMP_JPEG:
      imagePxDepth = 24;
      break;
    case uCamIII_RAW_16BIT_RGB565:
    case uCamIII_RAW_16BIT_CRYCBY:
      imagePxDepth = 16;
      break;
    case uCamIII_RAW_8BIT:
    defaut:
      imagePxDepth = 8;
      break;
  }
  
  return retVal;
}

int callbackSerial(uint8_t *buf, int len, int id)
{
  Log.info("Package %d (%d Byte) -> Serial", id, len);
  
  return Serial.write(buf, len);
}

int callbackTCP(uint8_t *buf, int len, int id)
{
  Log.info("Package %d (%d Byte) -> TCP %s:%d", id, len, (const char*)serverAddr.toString(), serverPort);

  if (serverPort)
  {
    if (!client.connected()) 
      client.connect(serverAddr, serverPort);
    return client.write(buf, len);
  }
  
  return 0;
}

// ------------------------------------------------------------------------------------------------------------------------
#if Wiring_WiFi // ---------------------------  only available for WiFi devices -------------------------------------------

int callbackWebServer(uint8_t *buf, int len, int id) 
{
  return webserver.write(buf, len);
}

void defaultCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  Log.trace(__FUNCTION__); 

  int snap   = -1;  // uCamIII_SNAP_TYPE
  int fmt    = -1;  // uCamIII_IMAGE_FORMAT 
  int res    = -1;  // uCamIII_RES          
  int rawRes = -1;
  int jpgRes = -1;
  int contr  = 0;
  int bright = 0;
  int expose = 0;

  char strScript[strlen((const char*)script)+16] = "";
  int  tzOffset  = Time.local() - Time.now();
  int  tzHours   = tzOffset / 3600; 
  int  tzMinutes = abs(tzOffset / 60) % 60;

  server.httpSuccess();

  if (type == WebServer::HEAD)
    return;

  if (strlen(url_tail)) 
  {
    while (strlen(url_tail))
    {
    URLPARAM_RESULT rc;
    char name[NAMELEN];
    char value[VALUELEN];
    
    rc = server.nextURLparam(&url_tail, name, NAMELEN, value, VALUELEN);
    if (rc != URLPARAM_EOS)
    {
      if (!strcmp(name, COLOR_TYPE))
      {
        fmt = atoi(value);
        snap = (fmt == uCamIII_COMP_JPEG) ? uCamIII_SNAP_JPEG : uCamIII_SNAP_RAW;
      }
      else if (!strcmp(name, RAW_RES))
        rawRes = atoi(value);           
      else if (!strcmp(name, JPG_RES))
        jpgRes = atoi(value);           
      else if (!strcmp(name, CONTRAST))
        contr = atoi(value);           
      else if (!strcmp(name, BRIGHTNESS))
        bright = atoi(value);           
      else if (!strcmp(name, EXPOSURE))
        expose = atoi(value);           
    }
    }
    
    res = (fmt == uCamIII_COMP_JPEG) ? jpgRes : rawRes;
    if (snap >= 0 && fmt > 0 && res > 0)
      prepareCam((uCamIII_SNAP_TYPE)snap, (uCamIII_IMAGE_FORMAT)fmt, (uCamIII_RES)res, 
                 (uCamIII_CBE)contr, (uCamIII_CBE)bright, (uCamIII_CBE)expose);

    snprintf(strScript, sizeof(strScript), (const char*)script, 
           fmt, rawRes, jpgRes, contr, bright, expose, 
           (const char*)Time.format(imageTime, "%FT%T"), tzHours, tzMinutes);
  }
  server.printP(Page_start);
  server.printP(camBody);
  server.print(strScript);
  server.printP(Page_end);
}

void imageCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  Log.trace(__FUNCTION__); 

  if (imageSize = ucam.getPicture((uCamIII_PIC_TYPE)imageType))
  {
    Log.info("\r\nImageSize: %d", imageSize);

    if (imageType == uCamIII_SNAP_JPEG)
    {
      Log.info("get JPEG chunks");
      for (int received = 0, chunk = 0; (received < imageSize) && (chunk = ucam.getJpegData(&imageBuffer[constrain(received, 0, sizeof(imageBuffer)-512)], 512, callbackWebServer)); received += chunk);
    }
    else if (imageSize <= sizeof(imageBuffer)) 
    {
      Log.info("get RAW image");

      int offset = bmpHeader(imageBuffer, sizeof(imageBuffer), imageWidth, imageHeight, imagePxDepth);
Log.trace("after bmpHeader (%d)", offset);
      int size   = ucam.getRawData(&imageBuffer[offset], imageSize, NULL);
Log.trace("after getRawData (%d)", size);

      if (imagePxDepth == 16) 
      {
        for (int i = offset; i < imageSize + offset; i += 2)    // raw image comes big-endian and upside down from cam,
        {                                                       // this block corrects endianness
          uint8_t dmy = imageBuffer[i];
          imageBuffer[i] = imageBuffer[i+1];
          imageBuffer[i+1] = dmy;
        }
      }
        
Log.trace("before server write (%d + %d = %d / %d == %d)", imageSize, offset, imageSize + offset, imageSize, size);
      server.write(imageBuffer, imageSize + offset);
Log.trace("after server write (%d + %d = %d / %d == %d)", imageSize, offset, imageSize + offset, imageSize, size);
    }
else
{
Log.trace("raw too big for buffer %d <= %d", imageSize, sizeof(imageBuffer));
}
    digitalWrite(D7, LOW);
  }
  
  //server.write(imageBuffer, imageSize);
}
#endif
// ------------------------------------------------------------------------------------------------------------------------

// -------------------------------------   helper function for bitmap headers   -------------------------------------------
struct BITMAPFILEHEADER 
{
  uint16_t   bfType;            // file type BM (0x424d)                        [  0]
  uint32_t   bfSize;            // file size all together                       [  2]
  uint16_t   bfReserved1;       //                                              [  6]
  uint16_t   bfReserved2;       //                                              [  8]
  uint32_t   bfOffBits;         // offset of actual image data                  [ 10]
} __attribute__((__packed__));  //                                              ( 14)

// https://msdn.microsoft.com/en-us/library/windows/desktop/dd183381(v=vs.85).aspx
struct BITMAPV5HEADER
{
  // --------------------------------------------------------------------------
  // legacy BITMAPINFOHEADER: 
  uint32_t Size;              // Size of this header in bytes               [  0][ 14]
  int32_t  Width;             // Image width in pixels                      [  4][ 18]
  int32_t  Height;            // Image height in pixels                     [  8][ 22]
  uint16_t Planes;            // Number of color planes                     [ 12][ 26]
  uint16_t BitsPerPixel;      // Number of bits per pixel                   [ 14][ 28]
  uint32_t Compression;       // Compression methods used                   [ 16][ 30]
  uint32_t SizeOfBitmap;      // Size of bitmap in bytes                    [ 20][ 34]
  int32_t  HorzResolution;    // Horizontal resolution in pixels per meter  [ 24][ 38] 
  int32_t  VertResolution;    // Vertical resolution in pixels per meter    [ 28][ 42]
  uint32_t ColorsUsed;        // Number of colors in the image              [ 32][ 46]
  uint32_t ColorsImportant;   // Minimum number of important colors         [ 36][ 50]
  // --------------------------------------------------------------------   ( 40)( 54)
  // Fields added for V2:
  uint32_t RedMask;           // Mask identifying bits of red component     [ 40][ 54]
  uint32_t GreenMask;         // Mask identifying bits of green component   [ 44][ 58]
  uint32_t BlueMask;          // Mask identifying bits of blue component    [ 48][ 62]
  // --------------------------------------------------------------------   ( 52)( 66)
  // Fields added for V3:
  uint32_t AlphaMask;         // Mask identifying bits of alpha component   [ 52][ 66]
/*// --------------------------------------------------------------------   ( 56)( 70)
  // Fields added for V4:
  uint32_t CSType;            // Color space type                           [ 56][ 70]
  int32_t  RedX;              // X coordinate of red endpoint               [ 60][ 74]
  int32_t  RedY;              // Y coordinate of red endpoint               [ 64][ 78]
  int32_t  RedZ;              // Z coordinate of red endpoint               [ 68][ 82]
  int32_t  GreenX;            // X coordinate of green endpoint             [ 72][ 86]
  int32_t  GreenY;            // Y coordinate of green endpoint             [ 76][ 90]
  int32_t  GreenZ;            // Z coordinate of green endpoint             [ 80][ 94]
  int32_t  BlueX;             // X coordinate of blue endpoint              [ 84][ 98]
  int32_t  BlueY;             // Y coordinate of blue endpoint              [ 88][102]
  int32_t  BlueZ;             // Z coordinate of blue endpoint              [ 92][106]
  uint32_t GammaRed;          // Gamma red coordinate scale value           [ 96][110]
  uint32_t GammaGreen;        // Gamma green coordinate scale value         [100][114]
  uint32_t GammaBlue;         // Gamma blue coordinate scale value          [104][118]
  // --------------------------------------------------------------------   (108)(122)
  // Fields added for V5:
  uint32_t Intent;            //                                            [108][122]
  uint32_t ProfileData;       //                                            [112][126]
  uint32_t ProfileSize;       //                                            [116][130]
  uint32_t Reserved;          //                                            [120][134]
*/
} __attribute__((__packed__));

int bmpHeader(uint8_t *buf, int len, int width, int height, int bits, int encoding, int padding)  
{  
  Log.trace(__FUNCTION__); 

  uint32_t          headerSize      = sizeof(BITMAPFILEHEADER) 
                                    + sizeof(BITMAPV5HEADER);
  uint32_t          paletteSize     = (bits <= 8) ? (1 << (bits+2)) : 0;    // optional palette
  uint32_t          pixelDataSize   = height * ((width * (bits / 8)) + padding);  

  BITMAPFILEHEADER *bmpFileHeader   = (BITMAPFILEHEADER*)buf;
  BITMAPV5HEADER   *bmpInfoHeader   = (BITMAPV5HEADER*)(buf + sizeof(BITMAPFILEHEADER));
  uint8_t          *bmpPalette      = buf + headerSize; 

  if (len < headerSize + paletteSize) return 0;

  /// ToDo: where do we set encoding R5G6B5 vs. CrYCbY?

  memset(bmpFileHeader, 0, sizeof(BITMAPFILEHEADER));
  bmpFileHeader->bfType              = 0x4D42;                               // magic number "BM"
  bmpFileHeader->bfSize              = headerSize 
                                     + paletteSize
                                     + pixelDataSize;
  //bmpFileHeader->bfReserved1         =
  //bmpFileHeader->bfReserved2         = 0;
  bmpFileHeader->bfOffBits           = headerSize
                                     + paletteSize;

  memset(bmpInfoHeader, 0, sizeof(BITMAPV5HEADER));
  bmpInfoHeader->Size                = sizeof(BITMAPV5HEADER);  
  bmpInfoHeader->Width               = width;                               
  bmpInfoHeader->Height              = -height;                              // negative height flips vertically
                                                                            // so image isn't upside down 
                                                                            // but Google Chorme doesn't support that!!! 
  bmpInfoHeader->Planes              = 1;  
  bmpInfoHeader->BitsPerPixel        = bits;  
  //bmpInfoHeader->Compression         = 0; 
  bmpInfoHeader->SizeOfBitmap        = 0x03;                                 // BI_RGB (0), BI_BITFIELD (3)
  bmpInfoHeader->HorzResolution      = 0x1000; 
  bmpInfoHeader->VertResolution      = 0x1000; 
  bmpInfoHeader->ColorsUsed          =     
  bmpInfoHeader->ColorsImportant     = (bits <= 8) ? (1 << bits) : 0;        // <= 8bit use color palette
  if (bits == 16)
  {  
    bmpInfoHeader->RedMask           = 0x0000f800;
	bmpInfoHeader->GreenMask         = 0x000007e0;
	bmpInfoHeader->BlueMask          = 0x0000001f;
  }

  for (int c = 0; c < bmpInfoHeader->ColorsUsed; c++)                         // grayscale color table
  {
    uint8_t color = 255 * c / (bmpInfoHeader->ColorsUsed - 1);
    *(bmpPalette++) = color;
    *(bmpPalette++) = color;
    *(bmpPalette++) = color;
    *(bmpPalette++) = 0xFF;
  }
  Log.trace("BMP data offset: %d + %d + %d = %d, BMP file size: %d",  
    sizeof(BITMAPFILEHEADER), sizeof(BITMAPV5HEADER), paletteSize, bmpFileHeader->bfOffBits, bmpFileHeader->bfSize);
  return bmpFileHeader->bfOffBits;
}  
