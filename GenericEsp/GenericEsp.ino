#include <EasyTransfer.h>
#include "WifiUtil.h"
#include "Settings.h"
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>


struct DATA_STRUCTURE {
	char topic[100];
	byte payload[100];
	unsigned int length;
	byte flag;
};

DATA_STRUCTURE rxdata;
DATA_STRUCTURE txdata;

WiFiClient espClient;
PubSubClient client(espClient);

EasyTransfer ETin, ETout;

int wifiMode = 0;
const int led = 02;
char message_buff[100];
char pub_status[100]; 

void callback(char* topic, byte* payload, unsigned int length) {

	int len = strlen(settings.clientId)+9;
	
	txdata.topic[0] = 0;
	String s = String(topic);
	s = s.substring(len);
	s.toCharArray(txdata.topic, 100);
	for (int i = 0; i < length; i++) {
		txdata.payload[i] = payload[i];
	}
	txdata.payload[length] = 0;
	txdata.length = length;

	//Serial.println(txdata.topic);
	//Serial.println((char*)txdata.payload);
	//Serial.println(txdata.length);

	ETout.sendData();
}

void reconnect() {
	// Loop until we're reconnected
	int c = 0;
	while (!client.connected()) {
		//Serial.print("Attempting MQTT connection...");
		// Attempt to connect
		if (client.connect(settings.clientId)) {
			//Serial.println("connected");
			// ... and subscribe to topic
			message_buff[0] = 0;

			strcat(message_buff, settings.clientId);
			strcat(message_buff, "/command/#");
			client.subscribe(message_buff);

			message_buff[0] = 0;
			strcat(message_buff, settings.clientId);
			strcat(message_buff, "/status");
			client.publish(message_buff, "Started");
		}
		else {
			c++;
			Serial.println(c);
			if (c > 10)
			{
				wifiMode = 0;
				last_error = "MQTT connection timout !";
				launchWeb();
				return;
			}
			//Serial.print("failed, rc=");
			//Serial.print(client.state());
			//Serial.println(" try again in 5 seconds");
			// Wait 5 seconds before retrying
			delay(1000);
		}
	}
}

void setup()
{
	Serial.begin(9600);
	ETin.begin(details(rxdata), &Serial);
	ETout.begin(details(txdata), &Serial);

	EEPROM.begin(512);
	loadConfig();

	WiFi.begin(settings.ssid, settings.pass);
	if (testWifi() != 20) {
		last_error = "Wifi connection timout !";
		launchWeb();
		return;
	}
	wifiMode = 1;
	
	client.setServer(settings.mqttServer, 1883);
	client.setCallback(callback);
}

void loop()
{
	if (wifiMode == 0) {
		server.handleClient();
		if (millis()>1000 * 10 * 60)
			ESP.restart();
	}
	else
	{
		if (!client.connected()) {
			reconnect();
		}
		if (wifiMode != 0)
			client.loop();
	}
}
