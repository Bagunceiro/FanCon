#ifndef WSERVER_H
#define WSERVER_H


#include <ESP8266WebServer.h>

extern ESP8266WebServer server;
extern const char* updateStatusMessage;

void handlePost();
void handleRoot();
void initWebServer();

#endif
