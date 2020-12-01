#ifndef WSERVER_H
#define WSERVER_H

#include <ESP8266WebServer.h>

class FanConWebServer : public ESP8266WebServer
{
public:
    FanConWebServer(unsigned int port) : ESP8266WebServer(port) {}
    void init();
};

extern FanConWebServer webServer;

#endif
