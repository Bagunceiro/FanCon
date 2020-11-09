#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include "config.h"
#include "configurator.h"
#include "lamp.h"

extern Lamp lamp;

Configurator::Configurator()
{
    startedAt = 0;
    running = false;
}

Configurator::~Configurator()
{

}

void Configurator::irmsgRecd(uint32_t code)
{
  if (code == IRREMOTE_CONFIGURATOR_START) start();
  if (code == IRREMOTE_CONFIGURATOR_STOP) stop();
}

void Configurator::start()
{
    char ssid[24];
    const char* password = "configure";
    sprintf(ssid, "conf_%s", persistant.controllername);
    
    WiFi.softAP(ssid, password);             // Start the access point
    Serial.printf("Access Point %s/%s started", ssid, password);

    Serial.printf("IP address: %s\n", WiFi.softAPIP().toString().c_str());

    for (int i = 0; i < 5; i++)
    {
        delay(lamp.blip(500));
    }
    startedAt = millis();
    running = true;
}

void Configurator::stop()
{
    ESP.reset();
}

void Configurator::poll()
{
    if (running)
    {
        unsigned long now = millis();
        if ((now - startedAt) > (2 * 60 *1000))
        {
            Serial.println("Configurator timeout");
            stop();
        }
    }
}

