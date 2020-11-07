#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
// #include <EEPROM.h>

extern const char* SSID;
extern const char* PSK;

#define MQTT_TPC_STAT  "status"
#define MQTT_TPC_SPEED "speed"
#define MQTT_TPC_SWITCH "switch"

extern const char* version;
extern const char* compDate;
extern const char* compTime;

extern const int DIR_RELAY1_PIN;
extern const int DIR_RELAY2_PIN;
extern const int SPD_RELAY1_PIN;
extern const int SPD_RELAY2_PIN;

extern const int LIGHT_RELAY_PIN;
extern const int LIGHT_SWITCH_PIN;

extern const int IR_DETECTOR_PIN;

extern const unsigned int MQTT_CONNECT_ATTEMPT_PAUSE; // Delay between attempts to reconnect MQTT (ms)
extern const unsigned int WIFI_CONNECT_ATTEMPT_PAUSE;



// #define MAGIC 0xc0ffee

struct configBlock
{
  // uint32_t magicnumber;
  char wifissid[16];
  char wifipsk[16];
  char controllername[16];
  char mqtthost[32];
  char mqttport[8];
  char mqttuser[16];
  char mqttpwd[16];
  char mqttroot[16];
  char mqtttopic[16];

  static const char* wifissid_n;
  static const char* wifipsk_n;
  static const char* controllername_n;
  static const char* mqtthost_n;
  static const char* mqttport_n;
  static const char* mqttuser_n;
  static const char* mqttpwd_n;
  static const char* mqttroot_n;
  static const char* mqtttopic_n;

  void dump();
  bool writeFile();
  bool readFile();
};

extern configBlock persistant;

char* upTime(char* buffer);

void report();

#endif
