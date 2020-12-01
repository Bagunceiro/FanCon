#include "conf.h"
#include <WiFiSerial.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

const String conffilename("/config.json");

void ConfBlk::dump()
{
    for (auto iterator : *this)
    {
        WSerial.printf("%s  = %s\n", iterator.first.c_str(), iterator.second.c_str());
    }
}

bool ConfBlk::writeFile()
{
    StaticJsonDocument<512> doc;

    LittleFS.begin();

    File configFile = LittleFS.open(conffilename, "w");
    if (!configFile)
    {
        perror("");
        WSerial.println("Config file open for write failed");
    }
    else
    {
        for (auto iterator : *this)
        {
            doc[iterator.first] = iterator.second;
        }
        serializeJson(doc, configFile);
    }
    configFile.close();
    LittleFS.end();
    return true;
}

bool ConfBlk::readFile()
{
    StaticJsonDocument<512> doc;
    bool result = false;

    LittleFS.begin();
    File configFile = LittleFS.open(conffilename, "r");
    if (!configFile)
    {
        WSerial.println("Config file open for read failed");
    }
    DeserializationError error = deserializeJson(doc, configFile);
    if (error)
    {
        WSerial.println(F("Failed to read file; using default configuration"));
    }
    else
    {
        result = true;
    }

    JsonObject root = doc.as<JsonObject>();
    for (JsonPair kv : root)
    {
        (*this)[kv.key()] = kv.value();
    }

    // Close the file (Curiously, File's destructor doesn't close the file)

    configFile.close();

    LittleFS.end();
    return result;
}