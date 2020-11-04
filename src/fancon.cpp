#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>

const char* version = "FanCon 1.0.1";
const char* compDate = __DATE__;
const char* compTime = __TIME__;

#include "config.h"
#include "spdt.h"
#include "mqtt.h"
#include "infrared.h"
#include "webserver.h"
#include "lamp.h"
#include "fan.h"

WiFiClient wifiClient;
ESP8266WiFiMulti wifimulti;
PubSubClient mqttClient(wifiClient);

Lamp lamp("light");
Fan fan("fan");

bool OTAinit = false;

void initOTA()
{
  Serial.println("initOTA");
  Serial.printf("Hostname to %s\n", persistant.controllername);
  ArduinoOTA.setHostname(persistant.controllername);
  ArduinoOTA.onStart([]() { // Enter safe state for the upgrade
    lamp.sw(0);
    fan.setSpeed(0);
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
  });
  ArduinoOTA.onEnd([]() {   // Sgnal completion of upgrade? How about flash the lights?
  });
  ArduinoOTA.onError([](ota_error_t error) {
    (void)error;
    ESP.restart();
  });
  /* setup the OTA server */
  ArduinoOTA.begin();
  OTAinit = true;
}

void initWiFi()
{
  static unsigned long lastAttempt = 0;
  unsigned long now = millis();

  if ((lastAttempt == 0) || ((now - lastAttempt) > WIFI_CONNECT_ATTEMPT_PAUSE))
  {
    Serial.print("Connecting to WiFi: ");
    Serial.print(persistant.wifissid);
    Serial.print("/");
    Serial.println(persistant.wifipsk);

    // WiFi.begin(persistant.wifissid, persistant.wifipsk);
    wifimulti.addAP(persistant.wifissid, persistant.wifipsk);
    wifimulti.addAP("asgard", "enaLkraP");
    wifimulti.addAP("asgard2", "enaLkraP");

    wifimulti.run();

    Serial.println("WiFi begun");

    lastAttempt = now;
    MDNS.begin(persistant.controllername);
    MDNS.addService("http", "tcp", 80);
  }
}

os_timer_t myTimer;

void timedLoop(void *pArg) {
  lamp.pollSwitch();
  pollIR();
}

void setup() {
  Serial.begin(9600);
  Serial.println("");
  Serial.println("Fancon Starting");

  // persistant.read();
  // persistant.dump();
  // persistant.writeFile();
  if (persistant.readFile() == false)
  {
    persistant.writeFile();

  }
  persistant.dump();

  Serial.println(version);

  String ctlr = persistant.controllername;

  lamp.init(LIGHT_SWITCH_PIN, LIGHT_RELAY_PIN);  lamp.pollSwitch();
  fan.init(DIR_RELAY1_PIN, DIR_RELAY2_PIN, SPD_RELAY1_PIN, SPD_RELAY2_PIN);

  irrecv.enableIRIn();

  os_timer_setfn(&myTimer, timedLoop, NULL);
  os_timer_arm(&myTimer, 10, true);

  initWebServer();
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    if (mqttClient.connected())
    {
      mqttClient.loop();
    }
    else
    {
      initMQTT();
    }

    if (OTAinit == false)
    {
      initOTA();
    }
    MDNS.update();
  }
  else
  {
    OTAinit = false;
    initWiFi();
  }


  ArduinoOTA.handle();
  server.handleClient();
}
