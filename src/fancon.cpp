#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>
#include <ESP8266httpUpdate.h>
#include <WiFiSerial.h>
#include <time.h>

const char *version = "FanCon 3.0.2";
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

WiFiClient mqttWifiClient;
WiFiClient updWifiClient;

PubSubClient mqttClient(mqttWifiClient);

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
  WSerial.println("HTTP update process started");
  fan.setSpeed(0);
  lamp.sw(0);
  for (int i = 0; i < 3; i++) delay(lamp.blip(500));
}

void update_completed()
{
  time_t now = timeClient.getEpochTime();
  WSerial.printf("HTTP update process complete at %s\n", ctime(&now));

  persistant.updateTime = String(now);
  persistant.writeFile();
  for (int i = 0; i < 6; i++) delay(lamp.blip(500));
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

  startup();

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
  ESPhttpUpdate.onEnd(update_completed);
  initWebServer();
}

bool wifiConnected = false;
bool ntpstarted = false;

void checkForUpdates()
{
  static time_t lastUpdCheck = 0;
  time_t now = timeClient.getEpochTime();

  if (persistant.updateServer.length() > 0)
  {
    unsigned int sinceLastCheck = (now - lastUpdCheck);
    unsigned int updateInterval = persistant.updateInterval.toInt() * 60;

    if ((lastUpdCheck == 0) || (sinceLastCheck > updateInterval))
    {
      WSerial.println("Checking for updates");
      lastUpdCheck = now;

      t_httpUpdate_return ret = ESPhttpUpdate.update(updWifiClient, persistant.updateServer);

      switch (ret)
      {
      case HTTP_UPDATE_FAILED:
        WSerial.printf("Update failure, Error(%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        break;
      case HTTP_UPDATE_NO_UPDATES:
        WSerial.println("No update available");
        break;
      case HTTP_UPDATE_OK:
        WSerial.println("Update OK");
        break;
      }
    }
  }
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED)
  {

    if (!wifiConnected)
    {

      wifiConnected = true;
      WSerial.begin("FanCon");
      WSerial.println("WiFi connected");
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
      timeClient.setUpdateInterval(3600000);
      ntpstarted = true;
      timeClient.update();
      timeClient.setTimeOffset(TZ * 60 * 60);
    }
    else
    {
      if (!timeClient.update())
      {
        WSerial.println("NTP failure");
      }
    }

    MDNS.update();
    checkForUpdates();
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
