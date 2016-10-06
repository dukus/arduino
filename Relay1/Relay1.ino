#include "WifiUtil.h"
#include "Settings.h"
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>

WiFiClient espClient;
PubSubClient client(espClient);

int wifiMode = 0;
const int led = 02;
char message_buff[100];
char pub_status[100];

void LedOn()
{
	digitalWrite(led, 0);
	client.publish(pub_status, "ON");

	Serial.print("Relay [ON]");
}

void LedOff()
{
	digitalWrite(led, 1);
	client.publish(pub_status, "OFF");

	Serial.print("Relay [OFF]");
}


void callback(char* topic, byte* payload, unsigned int length) {
	Serial.print("Message arrived [");
	Serial.print(topic);
	Serial.print("] ");

	
	message_buff[0] = 0;
	strcat(message_buff, settings.clientId);
	strcat(message_buff, "/relay");
	Serial.print(message_buff);

	if (strcmp(topic, message_buff) == 0 ) {
		for (int i = 0; i < length; i++) {
			char receivedChar = (char)payload[i];
			Serial.print(receivedChar);
			if (receivedChar == '0')
				LedOff();
			if (receivedChar == '1')
				LedOn();
		}
	}

	Serial.println();
}

void reconnect() {
	// Loop until we're reconnected
	int c = 0;
	while (!client.connected()) {
		Serial.print("Attempting MQTT connection...");
		// Attempt to connect
		if (client.connect(settings.clientId)) {
			Serial.println("connected");
			// ... and subscribe to topic
			message_buff[0] = 0;

			strcat(message_buff, settings.clientId);
			strcat(message_buff, "/relay");
			client.subscribe(message_buff);

			strcat(message_buff, "/status");
			client.publish(message_buff, "Started");
			LedOff();
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
			Serial.print("failed, rc=");
			Serial.print(client.state());
			Serial.println(" try again in 5 seconds");
			// Wait 5 seconds before retrying
			delay(5000);
		}
	}
}


void setup(void) {
	pinMode(led, OUTPUT);
	LedOff();
	EEPROM.begin(512);
	Serial.begin(115200);
	loadConfig();

	Serial.println(settings.ssid);
	Serial.println(settings.pass);

	WiFi.begin(settings.ssid, settings.pass);
	Serial.println("");
	if (testWifi() != 20) {
		last_error = "Wifi connection timout !";
		launchWeb();
		return;
	}
	wifiMode = 1;

	Serial.println("");
	Serial.print("Connected to ");
	Serial.println(settings.ssid);

	pub_status[0] = 0;
	strcat(pub_status, settings.clientId);
	strcat(pub_status, "/relay_status");

	client.setServer(settings.mqttServer, 1883);
	client.setCallback(callback);
}

void loop(void) {
	//
	if (wifiMode == 0) {
		server.handleClient();
		if(millis()>1000*10*60)
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
