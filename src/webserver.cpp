#include <Arduino.h>
#include "config.h"
#include "webserver.h"

ESP8266WebServer server(80);

void handleRoot()
{
  const unsigned int maxPageSize = 1024;
  char buffer[64];
  char temp[maxPageSize];

  snprintf(temp, maxPageSize,
           "<HEAD><title>Controller</title><style>\
body {font-family:Arial, Sans-Serif; font-size: 4vw;}\
table {font-family: arial, sans-serif; border-collapse: collapse; width: 100%%;}\
th, td {font-size: 4vw; border: 1px solid #dddddd;white-space:nowrap; text-align: left; padding: 8px; }\
button {font-size: 4vw}\
tr:nth-child(even) { background-color: #dddddd;}\
</style>\
<script>function goconf() {location.assign('config');}</script>\
</HEAD><BODY>\
<H1>Controller %s</H1>\
<TABLE>\
<TR><TD>Version</TD><TD>%s (%s %s)</TD></TR>\
<TR><TD>MAC Address</TD><TD>%s</TD></TR>\
<TR><TD>Uptime</TD><TD>%s</TD></TR>\
<TR><TD>WiFi SSID</TD><TD>%s</TD></TR>\
</TABLE><BR>\
<button type=button onclick=goconf()>Configure</button>\
</BODY>",
           persistant.controllername,
           version, compTime, compDate,
           WiFi.macAddress().c_str(),
           upTime(buffer),
           WiFi.SSID().c_str()
           );
  server.send(200, "text/html", temp);
}

void handlePost()
{
  if (server.method() == HTTP_POST)
  {

    for (uint8_t i = 0; i < server.args(); i++)
    {
      const String argName = server.argName(i);
      if (argName == "ctlrname")
      {
        snprintf(persistant.controllername, sizeof(persistant.controllername), server.arg(i).c_str());
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
        snprintf(persistant.mqtthost, sizeof(persistant.mqtthost), server.arg(i).c_str());
      }
      else if (argName == "mqttport")
      {
        snprintf(persistant.mqttport, sizeof(persistant.mqttport), server.arg(i).c_str());
      }
      else if (argName == "mqttuser")
      {
        snprintf(persistant.mqttuser, sizeof(persistant.mqttuser), server.arg(i).c_str());
      }
      else if (argName == "mqttpwd")
      {
        snprintf(persistant.mqttpwd, sizeof(persistant.mqttpwd), server.arg(i).c_str());
      }
      else if (argName == "mqttroot")
      {
        snprintf(persistant.mqttroot, sizeof(persistant.mqttroot), server.arg(i).c_str());
      }
      else if (argName == "mqtttopic")
      {
        snprintf(persistant.mqtttopic, sizeof(persistant.mqtttopic), server.arg(i).c_str());
      }
    }
    persistant.dump();
    persistant.writeFile();
    handleConfig();
    Serial.println("Resetting");

    delay(3000);
    ESP.reset();
    Serial.println("Done reset");
  }
}

void handleConfig()
{
  const int maxPageSize = 2048;

  char temp[maxPageSize];

  snprintf(temp, maxPageSize,
           "<HEAD><title>Controller Configuration</title><style>\
body {font-family:Arial, Sans-Serif; font-size: 4vw;}\
input {font-family:Arial, Helvetica, Sans-Serif; font-size: 4vw; Color:#000088;}\
table {font-family: arial, sans-serif; border-collapse: collapse; width: 100%%;}\
th, td {font-size: 4vw; border: 1px solid #dddddd;white-space:nowrap; text-align: left; padding: 8px; }\
tr:nth-child(even) { background-color: #dddddd;}\
</style>\
</HEAD><BODY>\
<H1>Controller Configuration</H1>\
<FORM method=post action=/config.update>\
<center><table>\
<tr><td><label for=ctlrname>Controller Name:</label></td><td><input type=text id=ctlrname name=ctlrname value=%s></td></tr>\
<tr><td><label for=mqtthost>MQTT Broker:</label></td><td><input type=text id=mqtthost name=mqtthost value=%s></td></tr>\
<tr><td><label for=mqttport>MQTT Port:</label></td><td><input type=text id=mqttport name=mqttport value=%s></td></tr>\
<tr><td><label for=mqttuser>MQTT User:</label></td><td><input type=text id=mqttuser name=mqttuser value=%s></td></tr>\
<tr><td><label for=mqttuser>MQTT Password:</label></td><td><input type=text id=mqttpwd name=mqttpwd value=%s></td></tr>\
<tr><td><label for=mqttroot>MQTT Topic root:</label></td><td><input type=text id=mqttroot name=mqttroot value=%s></td></tr>\
<tr><td><label for=mqtttopic>MQTT Topic:</label></td><td><input type=text id=mqtttopic name=mqtttopic value=%s></td></tr>\
<tr><td colspan=2><input type=submit value=\"Save and Reset\"></center></td></tr>\
</table></center>\
</FORM>\
</BODY>",
           persistant.controllername,
           persistant.mqtthost,
           persistant.mqttport,
           persistant.mqttuser,
           persistant.mqttpwd,
           persistant.mqttroot,
           persistant.mqtttopic);
  server.send(200, "text/html", temp);
}

void initWebServer()
{
  server.on("/", handleRoot);
  server.on("/config", handleConfig);
  server.on("/config.update", handlePost);

  //  String mac = WiFi.macAddress();
  // char ssid[24];
  //  sprintf(ssid, "c_%c%c%c%c%c%c", mac[9], mac[10], mac[12], mac[13], mac[15], mac[16]);
  //  Serial.print("SSID = "); Serial.println(ssid);
  server.begin();
}
