
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <WiFiUdp.h>

#include "WiFiSerial.h"
#include "config.h"

WiFiUDP udp;

NTPClient timeClient(udp, TZ * 60 * 60);

const char *configBlock::controllername_n = "controllername";
const char *configBlock::mqtthost_n       = "mqtthost";
const char *configBlock::mqttport_n       = "mqttport";
const char *configBlock::mqttuser_n       = "mqttuser";
const char *configBlock::mqttpwd_n        = "mqttpwd";
const char *configBlock::mqttroot_n       = "mqttroot";
const char *configBlock::mqtttopic_n      = "mqtttopic";
const char *configBlock::updateInterval_n = "updateInterval";
const char *configBlock::updateServer_n   = "updateServer";
const char *configBlock::updateTime_n     = "updateTime";

// const char* SSID = "asgard_2g";
//const char* SSID = "myth";
// const char* PSK  = "enaLkraP";

const int DIR_RELAY1_PIN = 12; // K4
const int DIR_RELAY2_PIN = 13; // K5
const int SPD_RELAY1_PIN = 16; // K2
const int SPD_RELAY2_PIN = 14; // K3

const int LIGHT_RELAY_PIN = 2; // K1
const int LIGHT_SWITCH_PIN = 4;

// const int SPARE_PIN = 15    //    Unused
const int IR_DETECTOR_PIN = 5;

const unsigned int MQTT_CONNECT_ATTEMPT_PAUSE = 30000; // Delay between attempts to reconnect MQTT (ms)
const unsigned int WIFI_CONNECT_ATTEMPT_PAUSE = 15000;

configBlock persistant;
time_t startTime = 0;

void startup()
{
  startTime = time(0);
}

String upTime()
{
  time_t now = time(0) - startTime;
  int days = now / (60 * 60 * 24);
  int hours = (now % (60 * 60 * 24)) / (60 * 60);
  int mins = (now % (60 * 60)) / 60;
  int secs = now % 60;
  char buffer[20];
  sprintf(buffer, "%d days, %02d:%02d:%02d", days, hours, mins, secs);
  return buffer;
}

void report()
{
  WSerial.print("Report for ");
  WSerial.println(persistant.controllername);
  WSerial.print("MAC Address: ");
  WSerial.println(WiFi.macAddress());
  WSerial.print("IP address: ");
  WSerial.println(WiFi.localIP());
  WSerial.print("Up time: ");
  WSerial.println(upTime());
}

void configBlock::dump()
{
  const char* format = "%s: %s\n";
  WSerial.print("controllername: ");
  WSerial.println(controllername);
  WSerial.print("mqtthost: ");
  WSerial.println(mqtthost);
  WSerial.print("mqttuser: ");
  WSerial.println(mqttuser);
  WSerial.print("mqttpwd: ");
  WSerial.println(mqttpwd);
  WSerial.print("mqttroot: ");
  WSerial.println(mqttroot);
  WSerial.print("mqtttopic: ");
  WSerial.println(mqtttopic);
  WSerial.printf(format, updateInterval_n, updateInterval.c_str());
  WSerial.printf(format, updateServer_n, updateServer.c_str());
  WSerial.printf(format, updateTime_n, updateTime.c_str());
}

bool configBlock::writeFile()
{
  StaticJsonDocument<512> doc;

  // Serial.println("writeFile");

  LittleFS.begin();

  File configFile = LittleFS.open("/config.json", "w");
  if (!configFile)
  {
    perror("");
    WSerial.println("Config file open for write failed");
  }
  else
  {
    doc[controllername_n] = controllername;
    doc[mqtthost_n]       = mqtthost;
    doc[mqttport_n]       = mqttport;
    doc[mqttuser_n]       = mqttuser;
    doc[mqttpwd_n]        = mqttpwd;
    doc[mqttroot_n]       = mqttroot;
    doc[mqtttopic_n]      = mqtttopic;
    doc[updateInterval_n] = updateInterval;
    doc[updateServer_n]   = updateServer;
    doc[updateTime_n]     = updateTime;
    serializeJson(doc, configFile);
  }
  configFile.close();
  LittleFS.end();
  return true;
}

bool configBlock::readFile()
{
  StaticJsonDocument<512> doc;
  bool result = false;

  LittleFS.begin();
  File configFile = LittleFS.open("/config.json", "r");
  if (!configFile)
  {
    WSerial.println("Config file open for read failed");
  }
  DeserializationError error = deserializeJson(doc, configFile);
  if (error)
  {
    WSerial.println(F("Failed to read file; using default configuration"));
  }
  else
  {
    result = true;
  }

  controllername = doc[controllername_n] | "FanCon";
  mqtthost       = doc[mqtthost_n] | "";
  mqttport       = doc[mqttport_n] | "";
  mqttuser       = doc[mqttuser_n] | "";
  mqttpwd        = doc[mqttpwd_n] | "";
  mqttroot       = doc[mqttroot_n] | "";
  mqtttopic      = doc[mqtttopic_n] | "";
  updateInterval = doc[updateInterval_n] | "15";
  updateServer   = doc[updateServer_n] | "";
  updateTime     = doc[updateTime_n] | "0";

  // Close the file (Curiously, File's destructor doesn't close the file)

  configFile.close();

  LittleFS.end();
  return result;
}