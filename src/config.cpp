#include "config.h"
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

   const char* configBlock::controllername_n = "controllername";
   const char* configBlock::mqtthost_n       = "mqtthost";
   const char* configBlock::mqttport_n       = "mqttport";
   const char* configBlock::mqttuser_n       = "mqttuser";
   const char* configBlock::mqttpwd_n        = "mqttpwd";
   const char* configBlock::mqttroot_n       = "mqttroot";
   const char* configBlock::mqtttopic_n      = "mqtttopic";

// const char* SSID = "asgard_2g";
//const char* SSID = "myth";
// const char* PSK  = "enaLkraP";

const int DIR_RELAY1_PIN = 12;      // K4
const int DIR_RELAY2_PIN = 13;      // K5
const int SPD_RELAY1_PIN = 16;      // K2
const int SPD_RELAY2_PIN = 14;      // K3

const int LIGHT_RELAY_PIN = 2;      // K1
const int LIGHT_SWITCH_PIN = 4;

// const int SPARE_PIN = 15    //    Unused
const int IR_DETECTOR_PIN = 5;

const unsigned int MQTT_CONNECT_ATTEMPT_PAUSE = 30000; // Delay between attempts to reconnect MQTT (ms)
const unsigned int WIFI_CONNECT_ATTEMPT_PAUSE = 15000;


#define MAGIC 0xc0ffee

configBlock persistant;

String upTime()
{
  time_t now = millis();
  int days = now / (1000 * 60 * 60 * 24);
  int hours = (now % (1000 * 60 * 60 * 24)) / (1000 * 60 * 60);
  int mins = (now % (1000 * 60 * 60)) / (1000 * 60);
  int secs = (now % (1000 * 60)) / 1000;
  int millisecs = now % 1000;
  char ms[4];
  sprintf(ms, "%03d", millisecs);
  return String(days)+" days "+String(hours)+":"+String(mins)+":"+String(secs)+"." + String(ms); 
}

void report()
{
  Serial.print("Report for ");   Serial.println(persistant.controllername);
  Serial.print("MAC Address: "); Serial.println(WiFi.macAddress());
  Serial.print("IP address: ");  Serial.println(WiFi.localIP());
  Serial.print("Up time: ");     Serial.println(upTime());
}

void configBlock::dump()
{
  // Serial.print("magicnumber: "); Serial.println(magicnumber, HEX);
  Serial.print("controllername: "); Serial.println(controllername);
  Serial.print("mqtthost: "); Serial.println(mqtthost);
  Serial.print("mqttport: "); Serial.println(mqttport);
  Serial.print("mqttuser: "); Serial.println(mqttuser);
  Serial.print("mqttpwd: "); Serial.println(mqttpwd);
  Serial.print("mqttroot: "); Serial.println(mqttroot);
  Serial.print("mqtttopic: "); Serial.println(mqtttopic);
}

bool configBlock::writeFile()
{
  StaticJsonDocument<512> doc;

Serial.println("writeFile");

  LittleFS.begin();

  File configFile = LittleFS.open("/config.json", "w");
  if (!configFile) {
    perror("");
    Serial.println("Config file open for write failed");
  }
  else {
    Serial.println("Building doc");
 
  doc[controllername_n] = controllername;
  doc[mqtthost_n] = mqtthost;
  doc[mqttport_n] = mqttport;
  doc[mqttuser_n] = mqttuser;
  doc[mqttpwd_n] = mqttpwd;
  doc[mqttroot_n] = mqttroot;
  doc[mqtttopic_n] = mqtttopic;
  Serial.println("writing file");
  serializeJson(doc, configFile);
  }
  configFile.close();
  LittleFS.end();
  Serial.println("done");
  return true;
}

bool configBlock::readFile()
{
  StaticJsonDocument<512> doc;
  bool result = false;

  LittleFS.begin();
  File configFile = LittleFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Config file open for read failed");
  }
  DeserializationError error = deserializeJson(doc, configFile);
  if (error)
  {
      Serial.println(F("Failed to read file; using default configuration"));
  }
  else
  {
      result = true;
  }
  // Serial.println("Read Config -");

  controllername = doc[controllername_n] | "FanCon";
  mqtthost       = doc[mqtthost_n]       | "";
  mqttport       = doc[mqttport_n]       | "";
  mqttuser       = doc[mqttuser_n]       | "";
  mqttpwd        = doc[mqttpwd_n]        | "";
  mqttroot       = doc[mqttroot_n]       | "";
  mqtttopic      = doc[mqtttopic_n]      | "";

  // Close the file (Curiously, File's destructor doesn't close the file)
  
  configFile.close();

  LittleFS.end();
  return result;
}
