#include <SPI.h>              // SPI connection (SD)
#include <Wire.h>
#include <SD.h>               // SD card
#include <DHT.h>
#include <RTClib.h>

#define SDFILENAME "/DONT_CLICK_THIS.txt"

#define         DHT_PIN     14

DHT dht(DHT_PIN, DHT22);
RTC_DS1307 rtc;

const unsigned long TIME_TO_SLEEP = 5 * 60 - 1;

SDFile myFile;
String dataString = "";
bool useRealTimeClock;
RTC_DATA_ATTR int recordCounter = 0;

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */

// =========== LOGIC ===============

void setup()   {                
  Serial.begin(115200);
  delay(1000); // giving RTC memory some time to recover

  dht.begin();

  useRealTimeClock = rtc.begin();
  if (!useRealTimeClock){
    Serial.println("Couldn't find RTC");
  }

  if (recordCounter == 0 && !rtc.isrunning()) {
    Serial.println("RTC not running, adjusting time.");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  if(!SD.begin())
  {
    Serial.println("Card Mount Failed");
    Sleep();
  }
  else
  {
    if (recordCounter == 0) Serial.println("Card Mounted");
  }

  if (recordCounter == 0){ // first start only
    Serial.println("Initializing Sensor");
    Serial.println("Initializing SD Card");
    
    uint8_t cardType = SD.cardType();
    if(cardType == CARD_NONE){
      Serial.println("No SD card attached");
    }

    Serial.print("SD Card Type: ");
    if(cardType == CARD_MMC){
      Serial.println("MMC");
    } else if(cardType == CARD_SD){
      Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
      Serial.println("SDHC");
    } else {
      Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize); Serial.println();

    myFile = SD.open(SDFILENAME, FILE_APPEND);
    Serial.print("Log file size: " );Serial.print(myFile.size()/1000); Serial.println(" kB");
  }
  else
  {
    myFile = SD.open(SDFILENAME, FILE_APPEND);
  }

  dataString = "";
  // gather data
  if (useRealTimeClock)
  {
    //dataString += rtc.now().timestamp();
    DateTime now = rtc.now();
    if (now.month() < 10) dataString += "0";
    dataString += now.month();
    dataString += "/";
    if (now.day() < 10) dataString += "0";
    dataString += now.day();
    dataString += "/";
    dataString += now.year();
    dataString += " ";
    if (now.hour() < 10) dataString += "0";
    dataString += now.hour();
    dataString += ":";
    if (now.minute() < 10) dataString += "0";
    dataString += now.minute();
    dataString += ":";
    if (now.second() < 10) dataString += "0";
    dataString += now.second();
  }
  else dataString += recordCounter;
  
  dataString += ",";
  dataString += dht.readHumidity();
  dataString += ",";
  dataString += dht.readTemperature();

  if(myFile.println(dataString)){
      recordCounter += TIME_TO_SLEEP;
      Serial.println("New record added:");
      Serial.println(dataString);
  }

  myFile.close();

  Sleep();
}

void loop() {
}

void Sleep(){
  Serial.print("Going to sleep for ");
  if ((int)((TIME_TO_SLEEP + 1)/60) > 0)
  {
    Serial.print((int)((TIME_TO_SLEEP + 1)/60));Serial.println(" minute(s).");
  }
  else
  {
    Serial.print((int)((TIME_TO_SLEEP + 1)));Serial.println(" second(s).");
  }
  Serial.flush();

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}