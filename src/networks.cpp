#include <LittleFS.h>
#include <ArduinoJson.h>
#include "networks.h"

networkList configuredNets;
networkList scannedNets;

networkList &scanNetworks()
{
    Serial.println("scan start");
    scannedNets.clear();

    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    if (n == 0)
    {
        Serial.println("no networks found");
    }
    else
    {
        Serial.print(n);
        Serial.println(" networks found");
        scannedNets.clear();
        for (int i = 0; i < n; ++i)
        {
            // Print SSID and RSSI for each network found
            WiFiNetworkDef network(WiFi.SSID(i));
            network.openNet = (WiFi.encryptionType(i) == ENC_TYPE_NONE);
            network.rssi = WiFi.RSSI(i);
            scannedNets.push_back(network);

            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(")");
            Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
            // delay(10);
        }
    }
    Serial.println("");
    return scannedNets;
}

/*
networkList &configuredNetworks()
{
    Serial.println("configured Networks");
    networkConfRead(configuredNets);

    return configuredNets;
}
*/

networkList &networkConfRead()
{
    Serial.println("read Network File");

    LittleFS.begin();

    File netsFile = LittleFS.open("/networks.json", "r");
    if (!netsFile)
    {
        perror("");
        Serial.println("Config file open for read failed");
    }
    else
    {
        configuredNets.clear();
        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, netsFile);
        if (error)
        {
            Serial.println(F("Failed to read network file"));
        }
        else
        {
        }
        JsonArray array = doc.as<JsonArray>();
        for (JsonObject net : array)
        {
            const char *ssid = (const char *)net["ssid"];
            const char *psk = (const char *)net["psk"];
            Serial.printf("Configured network: %s/%s\n", ssid, psk);

            WiFiNetworkDef network(ssid, psk);
            configuredNets.push_back(network);
        }
    }
    LittleFS.end();
    Serial.println("done");
    return configuredNets;
}

bool networkConfWrite(networkList &networks)
{
    StaticJsonDocument<1024> doc;
    JsonArray array = doc.to<JsonArray>();

    Serial.println("write Network File");

    LittleFS.begin();

    File netsFile = LittleFS.open("/networks.json", "w");
    if (!netsFile)
    {
        perror("");
        Serial.println("Config file open for write failed");
    }
    else
    {
        Serial.println("Building doc");
        for (unsigned int i = 0; i < networks.size(); i++)
        {
            JsonObject object = array.createNestedObject();
            object["ssid"] = networks[i].ssid;
            object["psk"] = networks[i].psk;
        }
        Serial.println("writing file");
        serializeJson(doc, netsFile);
        netsFile.close();
    }
    LittleFS.end();
    Serial.println("done");
    return true;
}

void addNetwork(networkList &netlist, const String &ssid)
{
    bool added = false;
    for (uint16_t i = 0; i < netlist.size(); i++)
    {
        if (netlist[i].ssid == ssid)
        {
            Serial.printf("Already added %s\n", ssid.c_str());
            return;
        }
    }
    for (uint16_t i = 0; i < configuredNets.size(); i++)
    {
        if (configuredNets[i].ssid == ssid)
        {
            Serial.printf("Adding existing %s\n", ssid.c_str());
            WiFiNetworkDef d = configuredNets[i];
            netlist.push_back(d);
            added = true;
            break;
        }
    }
    if (!added)
    {
        Serial.printf("Adding new %s\n", ssid.c_str());

        WiFiNetworkDef d(ssid);
        netlist.push_back(d);
    }
}