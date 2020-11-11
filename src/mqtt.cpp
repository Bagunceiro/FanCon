#include <Arduino.h>
#include <PubSubClient.h>
#include "config.h"
#include "mqtt.h"

extern PubSubClient mqttClient;

MqttControlled* MqttControlled::list;

MqttControlled::MqttControlled(String devName) : name(devName) {
  Serial.println((String)"Add " + devName);
  next = NULL;

  MqttControlled** ptr = &list;
  while (*ptr != NULL)
  {
    ptr = &((*ptr)->next);
  }
  *ptr = this;
}

void MqttControlled::msgScanDevices(String fullTopic, String msg)
{
  MqttControlled* ptr = list;
  while (ptr != NULL)
  {
    ptr->msgRecd(fullTopic, msg);
    ptr = ptr->next;
  }
}

// void MqttControlled::messageReceived(int messageSize) {
  
void messageReceived(char* fullTopic, byte* payload, unsigned int length)
{
  String msg;
  for (unsigned int i = 0; i < length; i++)
  {
    msg += (char) (payload[i]);
  }

  MqttControlled::msgScanDevices(fullTopic, msg);
}

void MqttControlled::setAllPrefixes() {
  MqttControlled* ptr = list;
  while (ptr != NULL)
  {
    ptr->setPrefix();
    ptr = ptr->next;
  }

}

void MqttControlled::doAllSubscriptions(PubSubClient& mqttClient)
{
  MqttControlled* ptr = list;
  while (ptr != NULL)
  {
    ptr->doSubscriptions(mqttClient);
    ptr = ptr->next;
  }

}

MqttControlled::~MqttControlled()
{
  MqttControlled**ptr = &list;
  while (*ptr != NULL)
  {
    if (*ptr == this)
    {
      (*ptr = next);
      break;
    }
    ptr = &((*ptr)->next);
  }
}


void MqttControlled::setPrefix(String tpcpfx)
{
  topicPrefix = tpcpfx;
}

void MqttControlled::doSubscriptions(PubSubClient& mqttClient)
{

}

void MqttControlled::setPrefix()
{
  setPrefix((String)persistant.mqttroot + "/" + persistant.mqtttopic + "/" + name + "/");
}

String MqttControlled::getPrefix()
{
  return topicPrefix;
}

void MqttControlled::sendStatus()
{
  if (mqttClient.connected())
  {
    char buffer[topicPrefix.length() + strlen(MQTT_TPC_STAT) + 1];
    char mbuff[8];
    sprintf(buffer, "%s%s", topicPrefix.c_str(), MQTT_TPC_STAT);
    getStatus(mbuff);
    mqttClient.publish(buffer, mbuff, true);
  }
}

void MqttControlled::msgRecd(String fullTopic, String msg)
{
  if (fullTopic.startsWith(topicPrefix))
  {
    String topic = fullTopic.substring(topicPrefix.length());
    mqttaction(topic, msg);
  }
}

void initMQTT()
{
  static unsigned long lastAttempt = 0;
  unsigned long now = millis();

  if ((persistant.mqtthost.length() == 0) || (persistant.mqttport.length()) == 0) return;

  MqttControlled::setAllPrefixes();

  if ((lastAttempt == 0) || ((now - lastAttempt) > MQTT_CONNECT_ATTEMPT_PAUSE))
  {
    Serial.print ("Connecting to MQTT ");
    Serial.print(persistant.mqtthost);
    Serial.print(" port ");
    Serial.println(persistant.mqttport);

    lastAttempt = now;

    String clientID = String("ctlr_") + String(persistant.controllername) + String("_") + String(millis() % 1000);

    mqttClient.setServer(persistant.mqtthost.c_str(), persistant.mqttport.toInt());
    mqttClient.setCallback(messageReceived);

    if (mqttClient.connect(clientID.c_str(), persistant.mqttuser.c_str(), persistant.mqttpwd.c_str())) {

      Serial.println("MQTT connected");
      MqttControlled::doAllSubscriptions(mqttClient);

    }
    else
    {
      Serial.print("Failed: ");
      Serial.println(mqttClient.state());
    }
  }
}
