#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include "config.h"
#include "configurator.h"
#include "lamp.h"
#include "fan.h"

extern Fan fan;
extern Lamp lamp;
Configurator configurator;

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

    /*
    Configurator entry uses the fan speed up keys on the remote. but ony when the fan is off.
    The button must be pressed numberOfPresses times with a delay of no more than keyPressDelay
    */

    const int numberOfPresses = 5;
    const unsigned int keyPressDelay = 2000;

    static int state = 0;
    static unsigned long stateChange = 0;
    unsigned long now = millis();


    if ((stateChange == 0) || ((now - stateChange) < keyPressDelay))
    {
        stateChange = now;


        if (fan.getSpeed() == 0)
        {
            if (code == IRREMOTE_CONFIGURATOR_START)
            {
                state++;
                if (state >= numberOfPresses)
                {
                    state = 0;
                    start();
                }
            }

            if (code == IRREMOTE_CONFIGURATOR_STOP)
                stop();
        }
    }
    else
    {
        state = 0;
    }
    
}

void Configurator::start()
{
    String m = WiFi.macAddress();
    String ssid = persistant.controllername + m.substring(9,11) + m.substring(12,14) + m.substring(15);
    // char ssid[24];
    const char *password = "configure";
    // sprintf(ssid, "conf_%s", persistant.controllername);
    Serial.printf("SoftAP SSID will be %s\n", ssid.c_str());

    WiFi.softAP(ssid, password); // Start the access point
    Serial.printf("Access Point %s/%s started", ssid.c_str(), password);

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
        if ((now - startedAt) > (15 * 60 * 1000))
        {
            Serial.println("Configurator timeout");
            stop();
        }
    }
}
