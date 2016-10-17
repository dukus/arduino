#include <SoftwareSerial.h>
#include <SoftEasyTransfer.h>


struct DATA_STRUCTURE {
	char topic[100];
	byte payload[100];
	unsigned int length;
	byte flag;
	byte dummy;
};

DATA_STRUCTURE rxdata;
DATA_STRUCTURE txdata;
SoftEasyTransfer ETin, ETout;

SoftwareSerial mySerial(7, 8);
//-------PINS--------------
const int relayPin = 6;
const int flowSensorPin1 = 2;
const int flowSensorPin2 = 3;
const int levelPin1 = 4;
const int levelPin2 = 5;
//-------------------------
volatile long flowRpm1;
volatile long flowRpm2;

volatile long oldFlowRpm1;
volatile long oldFlowRpm2;

long lastMillis;

void rpm1()     //This is the function that the interupt calls 
{
	flowRpm1++;  //This function measures the rising and falling edge of the 	hall effect sensors signal
}

void rpm2()     //This is the function that the interupt calls 
{
	flowRpm2++;  //This function measures the rising and falling edge of the 	hall effect sensors signal
}

void pumpOn()
{
	digitalWrite(relayPin, LOW);
	Send("status/pump", "ON");
}

void pumpOff()
{
	digitalWrite(relayPin, HIGH);
	Send("status/pump", "OFF");
}

void setup()
{
	Serial.begin(9600);
	mySerial.begin(9600);
	//start the library, pass in the data details and the name of the serial port.
	ETin.begin(details(rxdata), &mySerial);
	ETout.begin(details(txdata), &mySerial);

	pinMode(relayPin, OUTPUT);
	pinMode(levelPin1, INPUT);
	pinMode(levelPin2, INPUT);

	pumpOff();
	attachInterrupt(digitalPinToInterrupt(flowSensorPin1), rpm1, RISING);
	attachInterrupt(digitalPinToInterrupt(flowSensorPin2), rpm2, RISING);

	// delay startup until esp started
	Send("Status", "Started");
	SendAll();
	sei();
}

void loop()
{
	if (oldFlowRpm1 != flowRpm1)
	{
		Send("status/flow1", flowRpm1);
		oldFlowRpm1 = flowRpm1;
	}

	if (oldFlowRpm2 != flowRpm2)
	{
		Send("status/flow2", flowRpm2);
		oldFlowRpm2 = flowRpm2;
	}

	if (millis() - lastMillis > 30000 || millis() < lastMillis)
	{
		SendAll();
	}

	if (ETin.receiveData())
	{
		//message_buff[0] = 0;
		//strcat(message_buff, settings.clientId);
		//strcat(message_buff, "/");
		//strcat(message_buff, rxdata.topic);
		//client.publish(message_buff, rxdata.payload, rxdata.length);
		Serial.println(rxdata.topic);
		Serial.println((char*)rxdata.payload);
		Serial.println(rxdata.length);
		if (strcmp(rxdata.topic, "pump") == 0) {
			Serial.println("changing");
			if (rxdata.payload[0] == '1')
			{
				pumpOn();
			}
			else
			{
				pumpOff();
			}
		}
	}

	/* add main program code here */

}

void SendAll()
{
	Serial.println(flowRpm1);
	Serial.println(flowRpm2);
	lastMillis = millis();
	Send("status/flow1", flowRpm1);
	Send("status/flow2", flowRpm2);
	if (digitalRead(levelPin1) == HIGH)
		Send("status/level1", "ON");
	else
		Send("status/level1", "OFF");
	if (digitalRead(levelPin2) == HIGH)
		Send("status/level2", "ON");
	else
		Send("status/level2", "OFF");
}

void Send(char* topic, char* payload)
{
	txdata.length = strlen(payload);
	strcpy(txdata.topic, topic);
	memcpy(txdata.payload, payload, txdata.length + 1);
	ETout.sendData();
}

void Send(char* topic, int payload)
{
	char intStr[3];
	itoa(payload, intStr, 10);
	Send(topic, intStr);
}