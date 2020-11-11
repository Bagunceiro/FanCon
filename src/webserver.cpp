#include <Arduino.h>
#include "config.h"
#include "webserver.h"
#include "networks.h"

ESP8266WebServer server(80);

const String pageRoot       = "/";
const String pageMQTT       = "/config.mqtt";
const String pageMQTTUpdate = "/config.update";
const String pageWiFi       = "/config.net";
const String pageWiFiNet    = "/config.netedit";
const String pageWiFiNetAdd = "/config.addnet";
const String pageReset      = "/reset";

const String style = R"=====(
<script>
function gohome()     {location.assign(")=====" + pageRoot + R"=====(");}
function gomqttconf() {location.assign(")=====" + pageMQTT + R"=====(");}
function gowificonf() {location.assign(")=====" + pageWiFi + R"=====(");}
function goreset()    {location.assign(")=====" + pageReset + R"=====(");}
</script>
<style>
body {font-family:Arial, Sans-Serif; font-size: 4vw; background-color: lavender;}
table {font-family: arial, sans-serif; border-collapse: collapse; margin-left: 5%%; width: 100%%;}
th, td, input {font-size: 4vw; border: 1px solid #dddddd;white-space:nowrap; text-align: left; padding: 8px; }
tr:nth-child(even) { background-color: thistle;}
button {font-size: 3vw; background: indigo; color: white; }
input[type=checkbox] {display:none;}
input[type=checkbox] + label:before {content:"☐  ";}
input:checked + label:before {content:"☑  ";}
.sticky {
  position: fixed;
  top: 0;
  width: 100%;
}
.header {
  position: fixed;
  top: 0;
  width: 100%;}
</style>
)=====";

const String head1(R"=====(
<HEAD><meta http-equiv="content-type" charset="UTF-8"
content="text/html, width=device-width, initial-scale=1">
<TITLE>
)=====");

const String head2("</TITLE>");
const String headEnd(R"=====(
</HEAD>
)=====");

const String body1(R"=====(
<BODY>
<div class="header" id="myHeader">
<button onclick="gohome()">Home</button>
<button onclick=gomqttconf()>MQTT</button>
<button onclick=gowificonf()>WiFi</button>
-)=====");

const String tail("");

void sendPage(const String &s1,
              const String &s2,
              const String &s3,
              const String &s4,
              const String &s5,
              const String &s6,
              const String &s7,
              const String &s8)
{
  int contentLength = s1.length() + s2.length() + s3.length() + s4.length() + s5.length() + s6.length() + s7.length() + s8.length() + tail.length();

  server.setContentLength(contentLength);
  server.send(200, "text/html", s1);
  server.sendContent(s2);
  server.sendContent(s3);
  server.sendContent(s4);
  server.sendContent(s5);
  server.sendContent(s6);
  server.sendContent(s7);
  server.sendContent(s8);
  server.sendContent(tail);
}

void handleRoot()
{
  const unsigned int maxPageSize = 1024;
  char buffer[64];
  char body2[maxPageSize];

  const String title("Controller");
  //const String head3("<script>function gomqttconf() {location.assign('config.mqtt');} function gowificonf() {location.assign('config.net');}</script>");
  const String head3("");

  snprintf(body2, maxPageSize, R"=====(
<button onclick=goreset()>Reset</button>
</div>
<div class=content>
<BR><B>Controller: %s</B>
<TABLE>
<TR><TD>Version</TD><TD>%s (%s %s)</TD></TR>
<TR><TD>MAC Address</TD><TD>%s</TD></TR>
<TR><TD>Uptime</TD><TD>%s</TD></TR>
<TR><TD>WiFi SSID</TD><TD>%s</TD></TR>
</TABLE><BR>
</DIV>
</BODY>
)=====",
           persistant.controllername.c_str(),
           version, compTime, compDate,
           WiFi.macAddress().c_str(),
           upTime(buffer),
           WiFi.SSID().c_str());

  sendPage(head1, title, head2, style, head3, headEnd, body1, body2);
}

void resetMessage()
{
  const String title("Controller");
  const String head3 = "<meta http-equiv=\"refresh\" content=\"15;url=/\" />";
  const int maxBody = 256;
  char body2[maxBody];
  snprintf(body2, maxBody, R"=====(
  </div>
  <div class=content>
  <BODY>
  <br><br>Controller %s Resetting, please wait
  </div>
  </BODY>
  )=====",
           persistant.controllername.c_str());

  sendPage(head1, title, head2, style, head3, headEnd, body1, body2);
  delay(1000);
  ESP.reset();
}

void handleMQTTUpdate()
{
  if (server.method() == HTTP_POST)
  {

    for (uint8_t i = 0; i < server.args(); i++)
    {
      const String argName = server.argName(i);
      if (argName == persistant.controllername_n)
      {
        persistant.controllername = server.arg(i);
      }
      else if (argName == persistant.mqtthost_n)
      {
        persistant.mqtthost = server.arg(i);
      }
      else if (argName == persistant.mqttport_n)
      {
        persistant.mqttport = server.arg(i);
      }
      else if (argName == persistant.mqttuser_n)
      {
        persistant.mqttuser = server.arg(i);
      }
      else if (argName == persistant.mqttpwd_n)
      {
        persistant.mqttpwd = server.arg(i);
      }
      else if (argName == persistant.mqttroot_n)
      {
        persistant.mqttroot = server.arg(i);
      }
      else if (argName == persistant.mqtttopic_n)
      {
        persistant.mqtttopic = server.arg(i);
      }
    }
    persistant.dump();
    persistant.writeFile();
    Serial.println("Resetting");

    delay(3000);
    resetMessage();
  }
}

void handleMQTTConfig()
{
  String title("<title>Controller Configuration</title>");
  String head3("");
  String body2((String)R"=====(
<button type=submit form=theform>Save and Reset</button>
</div>
<div class=content>
<BR><B>MQTT Configuration: )=====" + persistant.controllername + R"=====(</B>
<FORM id=theform method=post action=")=====" + pageMQTTUpdate + R"=====(")>
<table>
<tr><td><label for=ctlrname>Controller Name:</label></td>
<td><input type=text name=")=====" + persistant.controllername_n + R"!(" value=")!" +  persistant.controllername + R"=====("></td></tr>
<tr><td><label for=mqtthost>MQTT Broker:</label></td>
<td><input type=text name=")=====" + persistant.mqtthost_n + R"!(" value=")!" + persistant.mqtthost + R"=====("></td></tr>
<tr><td><label for=mqttport>MQTT Port:</label></td>
<td><input type=text name=")=====" + persistant.mqttport_n + R"!(" value=")!" + persistant.mqttport + R"=====("></td></tr>
<tr><td><label for=mqttuser>MQTT User:</label></td>
<td><input type=text name=")=====" + persistant.mqttuser_n + R"!(" value=")!" + persistant.mqttuser + R"=====("></td></tr>
<tr><td><label for=mqttuser>MQTT Password:</label></td>
<td><input type=text name=")=====" + persistant.mqttpwd_n + R"!(" value=")!" + persistant.mqttpwd + R"=====("></td></tr>
<tr><td><label for=mqttroot>MQTT Topic root:</label></td>
<td><input type=text name=")=====" + persistant.mqttroot_n + R"!(" value=")!" + persistant.mqttroot + R"=====("></td></tr>
<tr><td><label for=mqtttopic>MQTT Topic:</label></td>
<td><input type=text name=")=====" + persistant.mqtttopic_n + R"!(" value=")!" + persistant.mqtttopic + R"=====("></td></tr>
</table>
</FORM>
</div>
</BODY>
)=====");

  sendPage(head1, title, head2, style, head3, headEnd, body1, body2);
}

String &listNetworks(String &body, networkList &networks, bool selected)
{
  for (unsigned int i = 0; i < networks.size(); i++)
  {
    body +=  String(R"=====(
<tr>
<td>
<input type=checkbox)=====") + String(selected ? " checked" : "") + String(" id=") + String(selected ? "cf" : "ds") + String(i) + String(R"=====( name=%s></input>
<label for=)=====") + String(selected ? "cf" : "ds") + i + String(R"=====()>&nbsp;</label>
<input type=hidden name=ssid value="%s"/>
</td>
<td>)=====") + String(networks[i].openNet ? "🔓" : "🔒")
            + (selected ? (String(" <a href=\"") + pageWiFiNet + "?ssid=" + networks[i].ssid + "\">") : "")
            + networks[i].ssid
            + String(selected ? "</a>" : "")
            + String(R"=====(
</td>
</tr>
)=====");
    /*
    const int maxNetLine = 256;
    char buffer[maxNetLine];
    snprintf(buffer, maxNetLine, R"=====(
<tr>
<td>
<input type=checkbox %s id=%s%d name=%s></input>
<label for=%s%d>&nbsp;</label>
<input type=hidden name=ssid value="%s"/>
</td>
<td>%s %s%s%s</td>
</tr>
)=====",
             (selected ? "checked" : ""),
             (selected ? "cf" : "ds"), i,
             "conf",
             (selected ? "cf" : "ds"), i,
             networks[i].ssid.c_str(),
             (networks[i].openNet ? "🔓" : "🔒"),
             (selected ? (String("<a href=\"") + pageWiFiNet + "?ssid=" + networks[i].ssid + "\">").c_str() : ""),
             networks[i].ssid.c_str(),
             (selected ? "</a>" : ""));
    body += buffer;
    */
  }
  return body;
}

bool sortByRSSI(WiFiNetworkDef i, WiFiNetworkDef j)
{
  return (i.rssi > j.rssi);
}

void handleNetConfig()
{
  if (server.method() == HTTP_POST)
  {
    Serial.println("Network Update");
    networkList newlist;

    bool usenext = false;
    for (uint8_t i = 0; i < server.args(); i++)
    {
      const String argName = server.argName(i);
      const String value = server.arg(i);

      if (usenext && (argName == "ssid"))
      {
        Serial.println(value);
        addNetwork(newlist, value);
      }

      if (argName == "conf")
        usenext = true;
      else
        usenext = false;
    }
    networkConfWrite(newlist);
  }
  networkList &cnetworks = networkConfRead();
  String title("WiFi Networks");
  String head3("");
  String body2(String(R"=====(
<button type=submit form=theform>Update</button>
</div>
<div class=content>
<BR><B>WiFi Configuration: )=====") +
               String(persistant.controllername) + String(R"=====(</B>
<FORM id=theform method=post action=/config.net>
<TABLE>
<TR><TH colspan=2>Configured Networks</TH></TR>
)====="));

  listNetworks(body2, cnetworks, true);
  body2 += R"=====(
</TABLE>
<TABLE>
<TR><TH colspan=2>Discovered Networks</TH></TR>
)=====";
  networkList &snetworks = scanNetworks();
  std::sort(snetworks.begin(), snetworks.end(), sortByRSSI);
  listNetworks(body2, snetworks, false);
  body2 += R"=====(
  </TABLE></FORM></DIV>
  </BODY>
  )=====";

  sendPage(head1, title, head2, style, head3, headEnd, body1, body2);
}

void handleNewNet()
{
  WiFiNetworkDef net("");
  net.openNet = true;

  if (server.method() == HTTP_POST)
  {

    for (uint8_t i = 0; i < server.args(); i++)
    {
      const String argName = server.argName(i);
      Serial.printf("Arg %s, val %s\n", argName.c_str(), server.arg(i).c_str());

      if (argName == "ssid")
      {
        Serial.printf("SSID is %s\n", server.arg(i).c_str());
        net.ssid = server.arg(i);
      }
      else if (argName == "psk")
      {
        net.psk = server.arg(i);
        net.openNet = false;
      }
    }
  }
  Serial.printf("Editted network %s\n", net.ssid.c_str());
  updateWiFiDef(net);
  String title("WiFi Network");
  String head3("");
  char body2[512];
  snprintf(body2, 512, R"====(
</div>
<div class=content>
<br><B>WiFi Network Edit</B>
<BR><br>
%s Updated
</div>
</BODY>
)====",
           net.ssid.c_str());

  sendPage(head1, title, head2, style, head3, headEnd, body1, body2);
}

void handleNetEdit()
{
  String ssid;
  for (int i = 0; i < server.args(); i++)
  {
    if (server.argName(i) == "ssid")
    {
      ssid = server.arg(i);
      break;
    }
  }
  String title("WiFi Network");
  String head3("");
  String body2;
  body2 = (String)R"=====(
<button type=submit form=theform>Update</button>
</div>
<div class=content>
<BR><B>WiFi Network Edit: ")=====" + persistant.controllername + R"=====("</B>
<FORM id=theform method=post action=)=====" + pageWiFiNetAdd + R"=====(>
<table>
<tr>
<td>SSID:</td>
<td><INPUT name=ssid value=")=====" + ssid + R"====("/></td>
</tr>
<tr>
<td>PSK:</td>
<td><INPUT type=password name=psk value=""/></td>
</tr>
</table>
</FORM>
</div>
</BODY>
)====";


  sendPage(head1, title, head2, style, head3, headEnd, body1, body2);
}

void initWebServer()
{
  server.on(pageRoot,       handleRoot);
  server.on(pageMQTT,       handleMQTTConfig);
  server.on(pageMQTTUpdate, handleMQTTUpdate);
  server.on(pageWiFi,       handleNetConfig);
  server.on(pageWiFiNet,    handleNetEdit);
  server.on(pageWiFiNetAdd, handleNewNet);
  server.on(pageReset,      resetMessage);

  Serial.println("Web Server");
  server.begin();
}

void restartWebServer()
{
  server.stop();
  initWebServer();
}
