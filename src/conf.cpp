#include "conf.h"
#include <WiFiSerial.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

const String conffilename("/config.json");

void ConfBlk::dump(Stream& s)
{
    for (auto iterator : *this)
    {
        s.printf("%s  = %s\n", iterator.first.c_str(), iterator.second.c_str());
    }
}

bool ConfBlk::writeStream(Stream &s)
{
    StaticJsonDocument<512> doc;
    for (auto iterator : *this)
    {
        doc[iterator.first] = iterator.second;
    }
    serializeJson(doc, s);
    return true;
}

bool ConfBlk::writeFile()
{
    bool result = true;

    LittleFS.begin();

    File configFile = LittleFS.open(conffilename, "w");
    if (!configFile)
    {
        perror("");
        WSerial.println("Config file open for write failed");
        result = false;
    }
    else
    {
        writeStream(configFile);
    }
    configFile.close();
    LittleFS.end();
    return result;
}

bool ConfBlk::readStream(Stream &s)
{
    bool result = false;
    StaticJsonDocument<512> doc;

    DeserializationError error = deserializeJson(doc, s);
    if (error)
    {
        WSerial.println(F("Failed to read file; using default configuration"));
        result = false;
    }
    else
    {
        JsonObject root = doc.as<JsonObject>();
        for (JsonPair kv : root)
        {
            (*this)[kv.key().c_str()] = (const char*)kv.value();
        }
        result = true;
    }
    return result;
}

bool ConfBlk::readFile()
{
    bool result = false;

    LittleFS.begin();

    File configFile = LittleFS.open(conffilename, "r");
    if (!configFile)
    {
        WSerial.println("Config file open for read failed");
    }
    else
    {
        result = readStream(configFile);
        configFile.close();
        result = true;
    }

    LittleFS.end();
    return result;
}