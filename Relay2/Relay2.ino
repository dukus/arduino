#include <OneButton.h>
#include "WifiUtil.h"
#include "Settings.h"
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>


#define RELAY1PIN 12
#define RELAY2PIN 13
#define BUTTONPIN 14

WiFiClient espClient;
PubSubClient client(espClient);
OneButton button(BUTTONPIN, false);


int wifiMode = 0;
const int led = 02;

char message_buff[100];
char pub_status[100];

void Relay1On()
{
	digitalWrite(RELAY1PIN, 0);

	pub_status[0] = 0;
	strcat(pub_status, settings.clientId);
	strcat(pub_status, "/status/relay1");

	client.publish(pub_status, "ON");

	Serial.print("Relay [ON]");
}

void Relay1Off()
{
	digitalWrite(RELAY1PIN, 1);

	pub_status[0] = 0;
	strcat(pub_status, settings.clientId);
	strcat(pub_status, "/status/relay1");

	client.publish(pub_status, "OFF");

	Serial.print("Relay [OFF]");
}

void Relay2On()
{
	digitalWrite(RELAY2PIN, 0);

	pub_status[0] = 0;
	strcat(pub_status, settings.clientId);
	strcat(pub_status, "/status/relay2");

	client.publish(pub_status, "ON");

	Serial.print("Relay [ON]");
}

void Relay2Off()
{
	digitalWrite(RELAY2PIN, 1);

	pub_status[0] = 0;
	strcat(pub_status, settings.clientId);
	strcat(pub_status, "/status/relay2");

	client.publish(pub_status, "OFF");

	Serial.print("Relay [OFF]");
}




void callback(char* topic, byte* payload, unsigned int length) {
	Serial.print("Message arrived [");
	Serial.print(topic);
	Serial.print("] ");


	message_buff[0] = 0;
	strcat(message_buff, settings.clientId);
	strcat(message_buff, "/command/relay1");
	Serial.print(message_buff);

	if (strcmp(topic, message_buff) == 0) {
		for (int i = 0; i < length; i++) {
			char receivedChar = (char)payload[i];
			Serial.print(receivedChar);
			if (receivedChar == '0')
				Relay1Off();
			if (receivedChar == '1')
				Relay1On();
		}
	}

	message_buff[0] = 0;
	strcat(message_buff, settings.clientId);
	strcat(message_buff, "/command/relay2");
	Serial.print(message_buff);

	if (strcmp(topic, message_buff) == 0) {
		for (int i = 0; i < length; i++) {
			char receivedChar = (char)payload[i];
			Serial.print(receivedChar);
			if (receivedChar == '0')
				Relay2Off();
			if (receivedChar == '1')
				Relay2On();
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
			strcat(message_buff, "/command/#");
			client.subscribe(message_buff);
			
			message_buff[0] = 0;
			strcat(message_buff, settings.clientId);
			strcat(message_buff, "/status");
			client.publish(message_buff, "Started");
			Relay1Off();
			Relay2Off();
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

void doubleclick(void)
{
	Relay1On();
	Relay2Off();
}

void click(void)
{
	Relay1Off();
	Relay2Off();
}

void longclick(void)
{
	Relay1On();
	Relay2On();
}

void setup(void) {
	pinMode(led, OUTPUT);
    pinMode(RELAY1PIN, OUTPUT);
	pinMode(RELAY2PIN, OUTPUT);
	pinMode(BUTTONPIN, INPUT);

	button.attachDoubleClick(doubleclick);
	button.attachClick(click);
	button.attachDuringLongPress(longclick);

	Relay1Off();
	Relay2Off();
	EEPROM.begin(512);
	Serial.begin(115200);
	loadConfig();

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
	button.tick();
	if (wifiMode == 0) {
		server.handleClient();
		if (millis()>1000 * 10 * 60)
			ESP.restart();
	}
	else
	{
		if (!client.connected()) {
			Relay1Off();
			Relay2Off();
			reconnect();
		}
		if (wifiMode != 0)
			client.loop();
	}
}
