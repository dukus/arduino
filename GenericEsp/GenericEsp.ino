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

void callback(char* topic, byte* payload, unsigned int length) {
	Serial.print("Message arrived [");
	Serial.print(topic);
	Serial.print("] ");


	message_buff[0] = 0;
	strcat(message_buff, settings.clientId);
	strcat(message_buff, "/relay");
	Serial.print(message_buff);

	if (strcmp(topic, message_buff) == 0) {
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
			strcat(message_buff, "/status");
			client.publish(message_buff, "Started");
			//------ reinit
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
			delay(1000);
		}
	}
}

void setup()
{

  /* add setup code here */

}

void loop()
{

  /* add main program code here */

}
