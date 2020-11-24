#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>
#include <ESP8266httpUpdate.h>
#include <WiFiSerial.h>

const char *version = "FanCon 3.0.1";
const char *compDate = __DATE__;
const char *compTime = __TIME__;

#include "config.h"
#include "spdt.h"
#include "mqtt.h"
#include "infrared.h"
#include "webserver.h"
#include "configurator.h"
#include "networks.h"
#include "lamp.h"
#include "fan.h"

WiFiClient wifiClient;

PubSubClient mqttClient(wifiClient);

Lamp lamp("light");
Fan fan("fan");
extern Configurator configurator;

int wifiattemptcount = 0;

void initWiFi()
{
  static unsigned long lastAttempt = 0;
  unsigned long now = millis();
  unsigned long pause = WIFI_CONNECT_ATTEMPT_PAUSE;

  if (wifiattemptcount < 20)
    pause = 500;

  if ((lastAttempt == 0) || ((now - lastAttempt) > pause))
  {
    wifiattemptcount++;

    connectToWiFi();

    lastAttempt = now;
    MDNS.begin(persistant.controllername);
    MDNS.addService("http", "tcp", 80);
  }
}

void update_started()
{
  Serial.println("HTTP update process started");
  fan.setSpeed(0);
  lamp.sw(0);
}

os_timer_t myTimer;

void timedLoop(void *pArg)
{
  lamp.pollSwitch();
  pollIR();
}

void setup()
{
  WiFi.mode(WIFI_STA);
  Serial.begin(9600);
  Serial.println("");
  Serial.println("Fancon Starting");

  if (persistant.readFile() == false)
  {
    persistant.writeFile();
  }
  persistant.dump();

  Serial.println(version);

  String ctlr = persistant.controllername;

  lamp.init(LIGHT_SWITCH_PIN, LIGHT_RELAY_PIN);
  lamp.pollSwitch();
  fan.init(DIR_RELAY1_PIN, DIR_RELAY2_PIN, SPD_RELAY1_PIN, SPD_RELAY2_PIN);

  irrecv.enableIRIn();

  os_timer_setfn(&myTimer, timedLoop, NULL);
  os_timer_arm(&myTimer, 10, true);

  ESPhttpUpdate.onStart(update_started);
  initWebServer();
}

bool wifiConnected = false;
bool ntpstarted = false;

void checkForUpdates()
{
  static unsigned long lastChecked = 0;
  unsigned long now = millis();

  if ((persistant.updateServer.length() > 0) && ((lastChecked == 0) || ((now - lastChecked) > (unsigned long)persistant.updateInterval.toInt())))
  {
    Serial.println("Checking for updates");
    lastChecked = now;
    t_httpUpdate_return ret = ESPhttpUpdate.update(wifiClient, persistant.updateServer);
    switch (ret)
    {
    case HTTP_UPDATE_FAILED:
      Serial.printf("Update failure, Error(%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("No update available");
      break;
    case HTTP_UPDATE_OK:
      Serial.println("Update OK");
      break;
    }
  }
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    checkForUpdates();
    if (!wifiConnected)
    {
      Serial.println("WiFi connected");
      wifiConnected = true;
      WSerial.begin("FanCon");
    }
    wifiattemptcount = 0;
    if (mqttClient.connected())
    {
      mqttClient.loop();
    }
    else
    {
      initMQTT();
    }

    if (!ntpstarted)
    {
      timeClient.begin();
      ntpstarted = true;
      timeClient.update();
      timeClient.setTimeOffset(TZ * 60 * 60);
    }
    MDNS.update();
  }
  else
  {
    if (wifiConnected)
    {
      Serial.println("WiFi connection lost");
      wifiConnected = false;
    }
    initWiFi();
  }

  server.handleClient();
  configurator.poll();
  WSerial.loop();
}
