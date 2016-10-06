// WifiUtil.h

#ifndef _WIFIUTIL_h
#define _WIFIUTIL_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include <ESP8266WebServer.h>

String st;
String last_error;
ESP8266WebServer server(80);


void handleRoot();
void launchWeb();
void handleSettings();
int testWifi(void);
void handleNotFound();
String getMac();

#endif

