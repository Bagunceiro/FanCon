#include <Arduino.h>
#include "config.h"
#include "webserver.h"
#include "networks.h"
#include "WiFiSerial.h"

// ESP8266WebServer webServer(80);
FanConWebServer webServer(80);


const String pageRoot = "/";
const String pageGen = "/config.gen";
const String pageGenUpdate = "/config.update";
const String pageWiFi = "/config.net";
const String pageWiFiNet = "/config.netedit";
const String pageWiFiNetAdd = "/config.addnet";
const String pageReset = "/reset";

const String style = R"=====(
<script>
function gohome()     {location.assign(")=====" +
                     pageRoot + R"=====(");}
function gogenconf() {location.assign(")=====" +
                     pageGen + R"=====(");}
function gowificonf() {location.assign(")=====" +
                     pageWiFi + R"=====(");}
function goreset()    {location.assign(")=====" +
                     pageReset + R"=====(");}
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
<button onclick=gogenconf()>General</button>
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

  webServer.setContentLength(contentLength);
  webServer.send(200, "text/html", s1);
  webServer.sendContent(s2);
  webServer.sendContent(s3);
  webServer.sendContent(s4);
  webServer.sendContent(s5);
  webServer.sendContent(s6);
  webServer.sendContent(s7);
  webServer.sendContent(s8);
  webServer.sendContent(tail);
}

void handleRoot()
{
  const String title("Controller");
  const String head3("");
  time_t now = timeClient.getEpochTime();
  time_t lastUpdate = persistant.updateTime.toInt();

  String body2(R"=====(
<button onclick=goreset()>Reset</button>
</div>
<div class=content>
<BR><B>Controller: )=====" +
               persistant.controllername + R"=====(</B>
<TABLE>
<TR><TD>Time now</TD><TD>)=====" +
               ctime(&now) + R"=====(</TD></TR>
<TR><TD>Version</TD><TD>)=====" +
               version + " (" + compTime + " " + compDate + R"=====()</TD></TR>
<TR><TD>MAC Address</TD><TD>)=====" +
               WiFi.macAddress() + R"=====(</TD></TR>
<TR><TD>Last update</TD><TD>)=====" +
               (lastUpdate != 0 ? ctime(&lastUpdate) : "N/A") + R"=====(</TD></TR>
<TR><TD>Uptime</TD><TD>)=====" +
               upTime() + R"=====(</TD></TR>
<TR><TD>WiFi SSID</TD><TD>)=====" +
               WiFi.SSID() + R"=====(</TD></TR>
</TABLE><BR>
</DIV>
</BODY>
)=====");

  sendPage(head1, title, head2, style, head3, headEnd, body1, body2);
}

void resetMessage()
{
  const String title("Controller");
  const String head3 = "<meta http-equiv=\"refresh\" content=\"15;url=/\" />";
  String body2(R"=====(
  <div class=content>
  <BODY>
  <br><B>Controller )=====" +
               persistant.controllername + R"=====(</B><br><br>
  Resetting, please wait
  </div>
  </BODY>
  )=====");

  sendPage(head1, title, head2, style, head3, headEnd, "", body2);
  delay(1000);
  ESP.reset();
}

void handleGenUpdate()
{
  if (webServer.method() == HTTP_POST)
  {

    for (uint8_t i = 0; i < webServer.args(); i++)
    {
      const String argName = webServer.argName(i);
      if (argName == persistant.controllername_n)
      {
        persistant.controllername = webServer.arg(i);
      }
      else if (argName == persistant.mqtthost_n)
      {
        persistant.mqtthost = webServer.arg(i);
      }
      else if (argName == persistant.mqttport_n)
      {
        persistant.mqttport = webServer.arg(i);
      }
      else if (argName == persistant.mqttuser_n)
      {
        persistant.mqttuser = webServer.arg(i);
      }
      else if (argName == persistant.mqttpwd_n)
      {
        persistant.mqttpwd = webServer.arg(i);
      }
      else if (argName == persistant.mqttroot_n)
      {
        persistant.mqttroot = webServer.arg(i);
      }
      else if (argName == persistant.mqtttopic_n)
      {
        persistant.mqtttopic = webServer.arg(i);
      }
      else if (argName == persistant.updateServer_n)
      {
        persistant.updateServer = webServer.arg(i);
      }
      else if (argName == persistant.updateInterval_n)
      {
        persistant.updateInterval = webServer.arg(i);
      }
    }
    persistant.dump();
    persistant.writeFile();
    Serial.println("Resetting");

    delay(3000);
    resetMessage();
  }
}

void handleGenConfig()
{
  String title("<title>Controller Configuration</title>");
  String head3("");
  String body2((String) R"=====(
<button type=submit form=theform>Save and Reset</button>
</div>
<div class=content>
<BR><B>General Configuration: )=====" +
               persistant.controllername + R"=====(</B>
<FORM id=theform method=post action=")=====" +
               pageGenUpdate + R"=====(")>
<table>
<tr><td><label for=ctlrname>Controller Name:</label></td>
<td><input type=text name=")=====" +
               persistant.controllername_n + R"!(" value=")!" + persistant.controllername + R"=====("></td></tr>
<tr><td><label for=mqtthost>MQTT Broker:</label></td>
<td><input type=text name=")=====" +
               persistant.mqtthost_n + R"!(" value=")!" + persistant.mqtthost + R"=====("></td></tr>
<tr><td><label for=mqttport>MQTT Port:</label></td>
<td><input type=text name=")=====" +
               persistant.mqttport_n + R"!(" value=")!" + persistant.mqttport + R"=====("></td></tr>
<tr><td><label for=mqttuser>MQTT User:</label></td>
<td><input type=text name=")=====" +
               persistant.mqttuser_n + R"!(" value=")!" + persistant.mqttuser + R"=====("></td></tr>
<tr><td><label for=mqttuser>MQTT Password:</label></td>
<td><input type=text name=")=====" +
               persistant.mqttpwd_n + R"!(" value=")!" + persistant.mqttpwd + R"=====("></td></tr>
<tr><td><label for=mqttroot>MQTT Topic root:</label></td>
<td><input type=text name=")=====" +
               persistant.mqttroot_n + R"!(" value=")!" + persistant.mqttroot + R"=====("></td></tr>
<tr><td><label for=mqtttopic>MQTT Topic:</label></td>
<td><input type=text name=")=====" +
               persistant.mqtttopic_n + R"!(" value=")!" + persistant.mqtttopic + R"=====("></td></tr>
<tr><td><label for=mqtttopic>Update URL:</label></td>
<td><input type=text name=")=====" +
               persistant.updateServer_n + R"!(" value=")!" + persistant.updateServer + R"=====("></td></tr>
<tr><td><label for=mqtttopic>Update Check Interval:</label></td>
<td><input type=text name=")=====" +
               persistant.updateInterval_n + R"!(" value=")!" + persistant.updateInterval + R"=====("></td></tr>
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
    body += String(R"=====(
<tr>
<td>
<input type=checkbox)=====") +
            String(selected ? " checked" : "") + String(" id=") + String(selected ? "cf" : "ds") + String(i) + String(" name=conf") + String(R"=====(></input>
<label for=)=====") +
            String(selected ? "cf" : "ds") + i + String(R"=====(>&nbsp;</label>
<input type=hidden name=ssid value=")=====") +
            networks[i].ssid + String(R"=====("/>
</td>
<td>)=====") +
            String(networks[i].openNet ? "🔓" : "🔒") + (selected ? (String(" <a href=\"") + pageWiFiNet + "?ssid=" + networks[i].ssid + "\">") : "") + networks[i].ssid + String(selected ? "</a>" : "") + String(R"=====(
</td>
</tr>
)=====");
  }
  return body;
}

bool sortByRSSI(WiFiNetworkDef i, WiFiNetworkDef j)
{
  return (i.rssi > j.rssi);
}

void handleNetConfig()
{
  if (webServer.method() == HTTP_POST)
  {
    Serial.println("Network Update");
    networkList newlist;

    bool usenext = false;
    for (uint8_t i = 0; i < webServer.args(); i++)
    {
      const String argName = webServer.argName(i);
      const String value = webServer.arg(i);

      if (usenext && (argName == "ssid"))
      {
        Serial.println(value);
        addNetwork(newlist, value);
      }
Serial.println(argName.c_str());
      if (argName == "conf")
        usenext = true;
      else
      {
        usenext = false;
        if (argName == "newnet")
        {
          Serial.printf("newnet, value = %s\n", value.c_str());
          if (value.length() != 0)
          {
            addNetwork(newlist, value);
          }
        }
      }
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
<TR><TD>+</td><td><input name=newnet /></td></tr>
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

  if (webServer.method() == HTTP_POST)
  {

    for (uint8_t i = 0; i < webServer.args(); i++)
    {
      const String argName = webServer.argName(i);
      Serial.printf("Arg %s, val %s\n", argName.c_str(), webServer.arg(i).c_str());

      if (argName == "ssid")
      {
        Serial.printf("SSID is %s\n", webServer.arg(i).c_str());
        net.ssid = webServer.arg(i);
      }
      else if (argName == "psk")
      {
        net.psk = webServer.arg(i);
        net.openNet = false;
      }
    }
  }
  Serial.printf("Editted network %s\n", net.ssid.c_str());
  updateWiFiDef(net);
  String title("WiFi Network");
  String head3("");
  String body2(R"====(
</div>
<div class=content>
<br><B>WiFi Network Edit: )====" +
               persistant.controllername + R"====(</B><br><br>
)====" + net.ssid +
               R"====( Updated
</div>
</BODY>
)====");

  sendPage(head1, title, head2, style, head3, headEnd, body1, body2);
}

void handleNetEdit()
{
  String ssid;
  for (int i = 0; i < webServer.args(); i++)
  {
    if (webServer.argName(i) == "ssid")
    {
      ssid = webServer.arg(i);
      break;
    }
  }
  String title("WiFi Network");
  String head3("");
  String body2;
  body2 = (String) R"=====(
<button type=submit form=theform>Update</button>
</div>
<div class=content>
<BR><B>WiFi Network Edit: ")=====" +
          persistant.controllername + R"=====("</B>
<FORM id=theform method=post action=)=====" +
          pageWiFiNetAdd + R"=====(>
<table>
<tr>
<td>SSID:</td>
<td><INPUT name=ssid value=")=====" +
          ssid + R"====("/></td>
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

void FanConWebServer::init()
{
  webServer.on(pageRoot, handleRoot);
  webServer.on(pageGen, handleGenConfig);
  webServer.on(pageGenUpdate, handleGenUpdate);
  webServer.on(pageWiFi, handleNetConfig);
  webServer.on(pageWiFiNet, handleNetEdit);
  webServer.on(pageWiFiNetAdd, handleNewNet);
  webServer.on(pageReset, resetMessage);

  Serial.println("Web Server");
  webServer.begin();
}
