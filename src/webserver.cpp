#include <Arduino.h>
#include "config.h"
#include "webserver.h"

ESP8266WebServer server(80);

const char* updateStatusMessage = "";

void handlePost() {

  struct configBlock params = persistant;

  for (uint8_t i = 0; i < server.args(); i++) {
    const String argName = server.argName(i);
    if (argName == "ctlrname")
    {
      snprintf(params.controllername, sizeof(params.controllername), server.arg(i).c_str());
    }
    else if (argName == "wifissid")
    {
      // HARD CODED
      // snprintf(params.wifissid, sizeof(params.wifissid), server.arg(i).c_str());
    }
    else if (argName == "wifipsk")
    {
      // HARD CODED
      // snprintf(params.wifipsk, sizeof(params.wifipsk), server.arg(i).c_str());
    }
    else if (argName == "mqtthost")
    {
      snprintf(params.mqtthost, sizeof(params.mqtthost), server.arg(i).c_str());
    }
    else if (argName == "mqttport")
    {
      snprintf(params.mqttport, sizeof(params.mqttport), server.arg(i).c_str());
    }
    else if (argName == "mqttuser")
    {
      snprintf(params.mqttuser, sizeof(params.mqttuser), server.arg(i).c_str());
    }
    else if (argName == "mqttpwd")
    {
      snprintf(params.mqttpwd, sizeof(params.mqttpwd), server.arg(i).c_str());
    }
    else if (argName == "mqttroot")
    {
      snprintf(params.mqttroot, sizeof(params.mqttroot), server.arg(i).c_str());
    }
    else if (argName == "mqtttopic")
    {
      snprintf(params.mqtttopic, sizeof(params.mqtttopic), server.arg(i).c_str());
    }
  }
  persistant = params;
  persistant.magicnumber = MAGIC;
  persistant.write();
}

void handleRoot() {
  const int maxPageSize = 2048;
  String updated;
  bool resetting = false;

  if (server.method() == HTTP_POST) {
    handlePost();
    updated = "<P>Update success - Resetting</P>";
    resetting = true;
  }

  char buffer[64];
  char temp[maxPageSize];

  snprintf(temp, maxPageSize,
           "<HEAD><title>Controller Configuration</title><style>\
.l {background-color:lightblue; font-family:Arial, Helvetica, Sans-Serif; font-size: 4vw; Color:#000088;}\
input {background-color:#cccccc; font-family:Arial, Helvetica, Sans-Serif; font-size: 4vw; Color:#000088;}\
table, th, td { border: 1px solid black; }\
</style>\
</HEAD><BODY class=l>\
<H1>Controller Configuration</H1>\
Version: %s (%s %s)<br>\
MAC Address: %s<br>\
Uptime: %s<br>\
WiFi SSID: %s<br>\
%s<br>\
<FORM  method=post>\
<center><table class=l>\
<tr><td><label for=ctlrname>Controller:</label></td><td><input type=text id=ctlrname name=ctlrname value=%s></td></tr>\
<tr><th colspan=2>WiFi</th></tr>\
<tr><td><label for=wifissid>SSID:</label></td><td><input type=text id=wifissid name=wifissid value=\"%s\"></td></tr>\
<tr><td><label for=wifipsk>PSK:</label></td><td><input type=text id=wifipsk name=wifipsk value=\"%s\"></td></tr>\
<tr><th colspan=2>MQTT</th></tr>\
<tr><td><label for=mqtthost>Broker:</label></td><td><input type=text id=mqtthost name=mqtthost value=%s></td></tr>\
<tr><td><label for=mqttport>Port:</label></td><td><input type=text id=mqttport name=mqttport value=%s></td></tr>\
<tr><td><label for=mqttuser>User:</label></td><td><input type=text id=mqttuser name=mqttuser value=%s></td></tr>\
<tr><td><label for=mqttuser>Password:</label></td><td><input type=text id=mqttpwd name=mqttpwd value=%s></td></tr>\
<tr><td><label for=mqttroot>Topic root:</label></td><td><input type=text id=mqttroot name=mqttroot value=%s></td></tr>\
<tr><td><label for=mqtttopic>Topic:</label></td><td><input type=text id=mqtttopic name=mqtttopic value=%s></td></tr>\
<tr><td colspan=2><input type=submit value=\"Save and Reset\"></center></td></tr>\
</table></center>\
</FORM>\
</BODY>",
           version,compTime,compDate,
           WiFi.macAddress().c_str(),
           upTime(buffer),
           WiFi.SSID().c_str(),
           updated.c_str(),
           persistant.controllername,
           //persistant.wifissid,
           //persistant.wifipsk,
           "Hard Coded",
           "Hard Coded",
           persistant.mqtthost,
           persistant.mqttport,
           persistant.mqttuser,
           persistant.mqttpwd,
           persistant.mqttroot,
           persistant.mqtttopic);
  server.send(200, "text/html", temp);
  if (resetting)
  {
    delay(1000);
    ESP.reset();
  }
}

void initWebServer()
{
  server.on("/", handleRoot);
//  String mac = WiFi.macAddress();
// char ssid[24];
//  sprintf(ssid, "c_%c%c%c%c%c%c", mac[9], mac[10], mac[12], mac[13], mac[15], mac[16]);
//  Serial.print("SSID = "); Serial.println(ssid);
  server.begin();
}