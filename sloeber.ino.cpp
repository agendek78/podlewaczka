#ifdef __IN_ECLIPSE__
//This is a automatic generated file
//Please do not modify this file
//If you touch this file your change will be overwritten during the next build
//This file has been generated on 2018-08-23 13:29:30

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <EasyDDNS.h>
#include <Adafruit_AM2320.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_TSL2561_U.h>
#include "CMeasData.h"
#include "CIrrigation.h"

static bool wifiReconnect(void) ;
void formatSPIFFS(void) ;
void addCurrMeasJSON(float val, String& json) ;
void sendDayTemp(void) ;
void sendMonthTemp(void) ;
void sendDayPressure(void) ;
void sendMonthPressure(void) ;
void sendDayHum(void) ;
void sendMonthHum(void) ;
void sendDayLux(void) ;
void sendMonthLux(void) ;
void sendIrrStatus(void) ;
void parseSettings(void) ;
void displaySensorDetails(void) ;
void setup() ;
bool IsMeasTimeValid(uint32_t currTime) ;
void loop() ;

#include "podlewaczka.ino"


#endif
