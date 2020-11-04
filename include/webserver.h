#ifndef WSERVER_H
#define WSERVER_H


#include <ESP8266WebServer.h>

extern ESP8266WebServer server;

void handlePost();
void handleRoot();
void handleConfig();
void initWebServer();

#endif
