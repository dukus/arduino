// Settings.h

#ifndef _SETTINGS_h
#define _SETTINGS_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#define CONFIG_VERSION "l.0"
#define CONFIG_START 32

struct SETTINGS {
	char ssid[32];
	char pass[32];
	char clientId[32];
	char mqttServer[32];
	char version_of_program[4];
};

SETTINGS settings = { { '1','2' },{ '1','2' },{ '1','2' },{ '1','2' }, CONFIG_VERSION };

void saveConfig();
void loadConfig();

#endif

