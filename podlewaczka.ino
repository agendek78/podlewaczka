/*
  This sketch sends data via HTTP GET requests to data.sparkfun.com service.

  You need to get streamId and privateKey at data.sparkfun.com and paste them
  below. Or just customize this script to talk to other HTTP servers.

*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <EasyDDNS.h>

#include <Adafruit_AM2320.h>
#include <Adafruit_TSL2561_U.h>
#include <Adafruit_BMP280.h>
#include "libraries/i2c/esp8266_twi.h"

#include "CMeasData.h"
#include "CIrrigation.h"
#include "CBattMon.h"

#define CURR_TIME()   (timeClient.getEpochTime() % DAY)
#define MOTOR1_PORT   D11
#define MOTOR2_PORT   D12
#define VBATT_PIN     A0
#define TANK_COUNT    (3)

CIrrigation     irr(TANK_COUNT);
CBattMon        batt(VBATT_PIN, 10);
Adafruit_AM2320 am2320;
Adafruit_BMP280 bmp;
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT);

const char* ssid = "testssid";
const char* password = "12345678";

WiFiUDP ntpUDP;

// By default 'pool.ntp.org' is used with 60 seconds update interval and
// no offset
NTPClient timeClient(ntpUDP, "tempus1.gum.gov.pl", 1 * 60 * 60);

static bool wifiReconnect(void)
{
  if (WiFi.isConnected() == true)
  {
    return true;
  }
  else
  {
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
      would try to act as both a client and an access-point and could cause
      network-issues with your other WiFi-devices on your WiFi-network. */
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    int wait_time = 10;

    while (WiFi.status() != WL_CONNECTED && wait_time-- > 0)
    {
      delay(500);
      Serial.print(".");
    }

    if (WiFi.isConnected() == true)
    {
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());

      return true;
    }
  }

  return false;
}

ESP8266WebServer server (33033);

#define DAILY_MEAS_PERIOD (15 * MINUTE)
#define DAILY_MEAS_COUNT  (MAKE_TIME(24,0,0) / DAILY_MEAS_PERIOD)

static const char	  *dailyFileName = "/dayilyMeas.bin";
static const char	  *monthlyFileName = "/monthlyMeas.bin";
static CMeasData    dayilyMeas(DAILY_MEAS_COUNT, dailyFileName);
static CMeasData    monthlyMeas(31, monthlyFileName);
static MeasPoint_t  currMeas;

void formatSPIFFS(void)
{
  Serial.println("Please wait 30 secs for SPIFFS to be formatted");
  SPIFFS.format();
  Serial.println("Spiffs formatted");

  File f = SPIFFS.open("/formatComplete.txt", "w");
  if (!f)
  {
    Serial.println("file open failed");
  }
  else
  {
    f.println("Format Complete");
  }
}

void addCurrMeasJSON(float val, String& json)
{
  json = "{" + json + ",\"curr\":";
  json += val;
  json += ",\"date\":";
  json += timeClient.getEpochTime();
  json += "}";
}

void sendDayTemp(void)
{
  String json;

  dayilyMeas.GetMeasJSON(MeasType_TEMP, json);
  addCurrMeasJSON(currMeas.temp / 10.0f, json);
  server.send(200, "application/json", json);
}

void sendMonthTemp(void)
{
  String json;

  monthlyMeas.GetMeasJSON(MeasType_TEMP, json);
  addCurrMeasJSON(currMeas.temp / 10.0f, json);
  server.send(200, "application/json", json);
}

void sendDayPressure(void)
{
  String json;

  dayilyMeas.GetMeasJSON(MeasType_PRESS, json);
  addCurrMeasJSON(currMeas.press / 10.0f, json);
  server.send(200, "application/json", json);
}

void sendMonthPressure(void)
{
  String json;

  monthlyMeas.GetMeasJSON(MeasType_PRESS, json);
  addCurrMeasJSON(currMeas.press / 10.0f, json);
  server.send(200, "application/json", json);
}

void sendDayHum(void)
{
  String json;

  dayilyMeas.GetMeasJSON(MeasType_HUM, json);
  addCurrMeasJSON(currMeas.hum, json);
  server.send(200, "application/json", json);
}

void sendMonthHum(void)
{
  String json;

  monthlyMeas.GetMeasJSON(MeasType_HUM, json);
  addCurrMeasJSON(currMeas.hum, json);
  server.send(200, "application/json", json);
}

void sendDayLux(void)
{
  String json;

  dayilyMeas.GetMeasJSON(MeasType_LUX, json);
  addCurrMeasJSON(currMeas.lux, json);
  server.send(200, "application/json", json);
}

void sendMonthLux(void)
{
  String json;

  monthlyMeas.GetMeasJSON(MeasType_LUX, json);
  addCurrMeasJSON(currMeas.lux, json);
  server.send(200, "application/json", json);
}

void sendIrrStatus(void)
{
  String json;

  irr.GetStatusJSON(json);
  server.send(200, "application/json", json);
}

void sendBattLevel(void)
{
  String json;

  dayilyMeas.GetMeasJSON(MeasType_BATT, json);
  addCurrMeasJSON(currMeas.vbatt, json);
  server.send(200, "application/json", json);
}

void parseSettings(void)
{
  uint32_t cc = irr.GetChannelCount();
  uint32_t ec = irr.GetEventCount();

  for (uint32_t i = 0; i < ec; i++)
  {
    for (uint32_t j = 0; j < cc; j++)
    {
      String arg = String("r") + i + String("c") + j;
      if (server.hasArg(arg))
      {
        irr.SetEvChannel(i, j, server.arg(arg) == "0" ? false : true);
      }
    }
  }
  server.send(200, "plain/text", "OK");
}

void parseMotorCtrl(void)
{
  const static char *motorStateArg = "state";
  const static char *motorChannelArg = "ch";

  if (server.hasArg(motorStateArg) && server.hasArg(motorChannelArg))
  {
    String val = server.arg(motorStateArg);

    if (val[0] == '1')
    {
      irr.ControlChannel(true, server.arg(motorChannelArg).toInt());
    }
    else if (val[0] == '0')
    {
      irr.ControlChannel(false, server.arg(motorChannelArg).toInt());
    }
    server.send(200, "plain/text", "OK");
  }
  else
  {
    server.send(404, "plain/text", "Parameters error!");
  }
}

void sendDailyBinaryData(void)
{
  File f = SPIFFS.open(dailyFileName, "r");
  if (f)
  {
    server.streamFile(f, "application/octet-stream");
    f.close();
  }
}

void sendMonthlyBinaryData(void)
{
  File f = SPIFFS.open(monthlyFileName, "r");
  if (f)
  {
    server.streamFile(f, "application/octet-stream");
    f.close();
  }
}

void displaySensorDetails(void)
{
  sensor_t sensor;
  tsl.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" lux");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" lux");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" lux");
  Serial.println("------------------------------------");
  Serial.println("");
}

#define MEAS_PERIOD 15UL

unsigned long nextMeasTime = 0;
unsigned long nextDayilyStoreMeas = 0;

void setup()
{
  Serial.begin(115200);
  delay(10);

  twi_init(2, 14);

  memset(&currMeas, 0, sizeof(currMeas));

  wifiReconnect();

  timeClient.begin();
  if (timeClient.forceUpdate() == false)
  {
    Serial.println("Can't update time!");
  }

  pinMode(MOTOR1_PORT, OUTPUT);
  pinMode(MOTOR2_PORT, OUTPUT);

  am2320.begin();
  bmp.begin(0x76);

  irr.AddChannel(MOTOR1_PORT, 0);
  irr.AddChannel(MOTOR2_PORT, 1);

  irr.AddTime(MAKE_TIME(8, 30, 0));
  irr.AddTime(MAKE_TIME(15, 30, 0));
  irr.AddTime(MAKE_TIME(18, 30, 0));
  irr.AddTime(MAKE_TIME(22, 10, 0));

  if (!SPIFFS.begin())
  {
    Serial.println("Unable to mount SPIFFS! Formating...");
    formatSPIFFS();
  }

  dayilyMeas.Load();
  monthlyMeas.Load();

  server.on("/temp.json", sendDayTemp);
  server.on("/tempMonth.json", sendMonthTemp);
  server.on("/pressure.json", sendDayPressure);
  server.on("/pressMonth.json", sendMonthPressure);
  server.on("/humidity.json", sendDayHum);
  server.on("/humMonth.json", sendMonthHum);
  server.on("/podlewanie.json", sendIrrStatus);
  server.on("/lux.json", sendDayLux);
  server.on("/luxMonth.json", sendMonthLux);
  server.on("/batt.json", sendBattLevel);
  server.on("/sett", parseSettings);
  server.on("/motor", parseMotorCtrl);
  server.on(dailyFileName, sendDailyBinaryData);
  server.on(monthlyFileName, sendMonthlyBinaryData);
  server.serveStatic("/js", SPIFFS, "/js");
  server.serveStatic("/", SPIFFS, "/index.html");
  server.begin();

  EasyDDNS.service("noip");
  //EasyDDNS.client("host", "login", "pw");

  if (tsl.begin())
  {
    displaySensorDetails();
    tsl.enableAutoRange(true);
    tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);
  }
  else
  {
    Serial.println("TSL2561 not detected!");
  }

  WiFi.setSleepMode(WIFI_MODEM_SLEEP);
}

#define CHECK_TIME  (10UL * MEAS_PERIOD)
#define CHECK_NEW_DAY(currTime, nextMeas) (((currTime) > (DAY / 2)) && ((nextMeas) < (DAY / 2)))

bool IsMeasTimeValid(uint32_t currTime)
{
  int32_t val = nextMeasTime - currTime;

  if (val < 0)
  {
    return ((DAY - currTime + nextMeasTime) > (int32_t)CHECK_TIME ? false : true);
  }
  else
  {
    return ((val > (int32_t)CHECK_TIME) ? false : true);
  }
}

void loop()
{
  wifiReconnect();
  timeClient.update();
  server.handleClient();
  unsigned long currTime = CURR_TIME();

  if (nextMeasTime <= currTime &&
      !CHECK_NEW_DAY(currTime, nextMeasTime))
  {
    nextMeasTime = (currTime + MEAS_PERIOD) % DAY;

    Serial.print("Temp: "); Serial.println(am2320.readTemperature());

    if (currMeas.hum != 0)
    {
      currMeas.hum = (currMeas.hum + (uint8_t)(am2320.readHumidity())) / 2;
    }
    else
    {
      currMeas.hum = (uint8_t)(am2320.readHumidity());
    }
    Serial.print("Hum: "); Serial.println(currMeas.hum);

    if (currMeas.press != 0)
    {
      currMeas.press = (currMeas.press + (uint16_t)(bmp.readPressure() / 10.0f)) / 2;
      currMeas.temp = (currMeas.temp + (int16_t)(bmp.readTemperature() * 10.0f)) / 2;
    }
    else
    {
      currMeas.press = (uint16_t)(bmp.readPressure() / 10.0f);
      currMeas.temp = (int16_t)(bmp.readTemperature() * 10.0f);
    }

    Serial.print("Pressure: "); Serial.println(currMeas.press / 10.0f);
    Serial.print("Temp2: "); Serial.println(currMeas.temp / 10.0f);

    sensors_event_t event;
    tsl.getEvent(&event);

    if (currMeas.lux != 0)
    {
      currMeas.lux = (currMeas.lux + (uint32_t)event.light) / 2;
    }
    else
    {
      currMeas.lux = (uint32_t)event.light;
    }

    Serial.print("Lux: "); Serial.println(event.light);
    Serial.print("Time: ");
    Serial.println(timeClient.getFormattedTime());

    batt.DoMeas();
    currMeas.vbatt = batt.GetMeanValue();
    Serial.print("Batt: "); Serial.print(currMeas.vbatt); Serial.print(", per: ");
    Serial.println(batt.GetBattLevel(currMeas.vbatt));

    currMeas.ts = currTime;

    if (nextDayilyStoreMeas <= nextMeasTime &&
        !(CHECK_NEW_DAY(nextMeasTime, nextDayilyStoreMeas)))
    {
      dayilyMeas.AddPoint(currMeas);
      memset(&currMeas, 0, sizeof(currMeas));
      nextDayilyStoreMeas = (nextMeasTime + DAILY_MEAS_PERIOD) % DAY;
      dayilyMeas.Save();
      Serial.print("Saving daily meas data! Next point at ");
      Serial.print((int)(nextDayilyStoreMeas / HOUR));
      Serial.print(":");
      Serial.println((int)((nextDayilyStoreMeas % HOUR) / MINUTE));

      if (CHECK_NEW_DAY(currTime, nextDayilyStoreMeas))
      {
        MeasPoint_t mean;

        dayilyMeas.CalcMeanMeas(mean);
        mean.ts = timeClient.getEpochTime();
        monthlyMeas.AddPoint(mean);
        monthlyMeas.Save();
        Serial.println("Saving monthly meas data!");
      }
    }
  }

  irr.DoWork(currTime);
  EasyDDNS.update(10000);

  if (IsMeasTimeValid(currTime) == false)
  {
    nextMeasTime = currTime;
    nextDayilyStoreMeas = currTime;
    Serial.println("Invalid meas time! Updating to current!");
  }

  //Serial.println("entering sleep");
  delay(500);
}
