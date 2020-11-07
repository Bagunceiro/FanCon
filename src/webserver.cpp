#include <Arduino.h>
#include "config.h"
#include "webserver.h"

ESP8266WebServer server(80);

const String style("<style>\
body {font-family:Arial, Sans-Serif; font-size: 4vw;}\
table {font-family: arial, sans-serif; border-collapse: collapse; width: 100%%;}\
th, td {font-size: 4vw; border: 1px solid #dddddd;white-space:nowrap; text-align: left; padding: 8px; }\
tr:nth-child(even) { background-color: #dddddd;}\
button {font-size: 4vw}\
</style>");

const String head1("<HEAD><TITLE>");
const String head2("</TITLE>");
const String headEnd("</HEAD>");

void handleRoot()
{
  const unsigned int maxPageSize = 1024;
  char buffer[64];
  char body[maxPageSize];

  const String title("Controller");
  const String head3("<script>function goconf() {location.assign('config');}</script>");

  snprintf(body, maxPageSize,
           "<BODY>\
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
           WiFi.SSID().c_str());
  int contentLength = head1.length() + title.length() + head2.length() + style.length() + head3.length() + headEnd.length() + strlen(body);

  server.setContentLength(contentLength);
  server.send(200, "text/html", head1);
  server.sendContent(title);
  server.sendContent(head2);
  server.sendContent(style);
  server.sendContent(head3);
  server.sendContent(headEnd);
  server.sendContent(body);
}

void resetMessage()
{
  const String title("Controller");
  const String head3 = "<meta http-equiv=\"refresh\" content=\"15;url=/\" />";
  const int maxBody = 96;
  char body[maxBody];
  snprintf(body, maxBody, "<BODY><H1>Controller %s</H1>Resetting, please wait</BODY>", persistant.controllername);

int contentLength = head1.length() + title.length() + head2.length() + style.length() + head3.length() + headEnd.length() + strlen(body);

server.setContentLength(contentLength);
server.send(200, "text/html", head1);
server.sendContent(title);
server.sendContent(head2);
server.sendContent(style);
server.sendContent(head3);
server.sendContent(headEnd);
server.sendContent(body);
}

void handlePost()
{
  resetMessage();
  if (server.method() == HTTP_POST)
  {

    for (uint8_t i = 0; i < server.args(); i++)
    {
      const String argName = server.argName(i);
      if (argName == persistant.controllername_n)
      {
        snprintf(persistant.controllername, sizeof(persistant.controllername), server.arg(i).c_str());
      }
      else if (argName == persistant.mqtthost_n)
      {
        snprintf(persistant.mqtthost, sizeof(persistant.mqtthost), server.arg(i).c_str());
      }
      else if (argName == persistant.mqttport_n)
      {
        snprintf(persistant.mqttport, sizeof(persistant.mqttport), server.arg(i).c_str());
      }
      else if (argName == persistant.mqttuser_n)
      {
        snprintf(persistant.mqttuser, sizeof(persistant.mqttuser), server.arg(i).c_str());
      }
      else if (argName == persistant.mqttpwd_n)
      {
        snprintf(persistant.mqttpwd, sizeof(persistant.mqttpwd), server.arg(i).c_str());
      }
      else if (argName == persistant.mqttroot_n)
      {
        snprintf(persistant.mqttroot, sizeof(persistant.mqttroot), server.arg(i).c_str());
      }
      else if (argName == persistant.mqtttopic_n)
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
<tr><td><label for=ctlrname>Controller Name:</label></td><td><input type=text name=%s value=%s></td></tr>\
<tr><td><label for=mqtthost>MQTT Broker:</label></td><td><input type=text name=%s value=%s></td></tr>\
<tr><td><label for=mqttport>MQTT Port:</label></td><td><input type=text name=%s value=%s></td></tr>\
<tr><td><label for=mqttuser>MQTT User:</label></td><td><input type=text name=%s value=%s></td></tr>\
<tr><td><label for=mqttuser>MQTT Password:</label></td><td><input type=text name=%s value=%s></td></tr>\
<tr><td><label for=mqttroot>MQTT Topic root:</label></td><td><input type=text name=%s value=%s></td></tr>\
<tr><td><label for=mqtttopic>MQTT Topic:</label></td><td><input type=text name=%s value=%s></td></tr>\
<tr><td colspan=2><input type=submit value=\"Save and Reset\"></center></td></tr>\
</table></center>\
</FORM>\
</BODY>",
           persistant.controllername_n, persistant.controllername,
           persistant.mqtthost_n, persistant.mqtthost,
           persistant.mqttport_n, persistant.mqttport,
           persistant.mqttuser_n, persistant.mqttuser,
           persistant.mqttpwd_n, persistant.mqttpwd,
           persistant.mqttroot_n, persistant.mqttroot,
           persistant.mqtttopic_n, persistant.mqtttopic);
  server.send(200, "text/html", temp);
}

void handleNetConfig()
{
  const int maxPageSize = 2048;

  char temp[maxPageSize];

  snprintf(temp, maxPageSize,
           "<HEAD><title>Controller Network Configuration</title><style>\
body {font-family:Arial, Sans-Serif; font-size: 4vw;}\
input {font-family:Arial, Helvetica, Sans-Serif; font-size: 4vw; Color:#000088;}\
table {font-family: arial, sans-serif; border-collapse: collapse; width: 100%%;}\
th, td {font-size: 4vw; border: 1px solid #dddddd;white-space:nowrap; text-align: left; padding: 8px; }\
tr:nth-child(even) { background-color: #dddddd;}\
</style>\
</HEAD><BODY>\
<H1>Controller %s Network Configuration</H1>\
<FORM method=post action=/config.update>\
<center><table>\
<tr><td><label for=ssid>SSID:</label></td><td><input type=text name=%s value=%s></td></tr>\
<tr><td><label for=psk>PSK:</label></td><td><input type=text name=%s value=%s></td></tr>\
<tr><td colspan=2><input type=submit value=\"Save and Reset\"></center></td></tr>\
</table></center>\
</FORM>\
</BODY>",
           persistant.controllername,
           persistant.wifissid_n, persistant.wifissid,
           persistant.wifipsk_n, persistant.wifipsk);
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

void restartWebServer()
{
  server.stop();
  initWebServer();
}
