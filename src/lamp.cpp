#include <Arduino.h>
#include "config.h"
#include "lamp.h"

const int MAXDEBOUNCE = 5; // Number of loops to allow light switch to settle

Lamp::Lamp(String devName) : MqttControlled(devName) {}

Lamp::~Lamp() {}

void Lamp::sw(int toState)
{
  if (toState == 0) digitalWrite(lpin, HIGH);
  else digitalWrite(lpin, LOW);
  sendStatus();
}

void Lamp::toggle()
{
  char buff[8];
  int isOn;
  getStatus(buff);
  sscanf(buff, "%d", &isOn);
  sw(isOn == 0 ? 1 : 0);
}

void Lamp::pollSwitch()
{
  int newState = digitalRead(spin);;
  if (newState != switchState)
  {
    debounce++;
    if (debounce > MAXDEBOUNCE)
    {
      toggle();
      switchState = newState;
      debounce = 0;
    }
  }
}

char* Lamp::getStatus(char* buff)
{
  int l = digitalRead(lpin);
  sprintf(buff, "%d",  (l == 0 ? 1 : 0));
  return buff;
}

const int Lamp::blip(const int t)
{
  toggle();
  delay(t);
  toggle();
  return (t);
}

void Lamp::blip(const int number, const int length)
{
    for (int i = number; i > 0; i--)
    {
      blip(length);
      if (i > 1) delay(length);
    }
}

void Lamp::init(int inp, int out)
{
  spin = inp;
  lpin = out;
  pinMode(spin, INPUT);
  pinMode(lpin, OUTPUT);
  switchState = digitalRead(spin);
  debounce = 0;

  sw(0);
}

void Lamp::mqttaction(String topic, String msg)
{
  if (topic == MQTT_TPC_SWITCH)
  {
    sw(msg.toInt());
  }
}

void Lamp::doSubscriptions(PubSubClient& mqttClient)
{
  mqttClient.subscribe((getPrefix() + MQTT_TPC_SWITCH).c_str());
  sendStatus();
}

void Lamp::irmsgRecd(uint32_t code)
{
  if (code == IRREMOTE_LIGHT_ONOFF) toggle();
}
