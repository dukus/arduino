#include <NewPing.h>
#include <SoftwareSerial.h>
#include <SoftEasyTransfer.h>

/*
Level Jack 3.5 common wire the (0) pin of the jack
Flow rate Jack 3.5 (0)<->(0) (middle)<->(+5) (Top)<->(Data)
*/

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
NewPing sonar1(A0, A1, 200);
NewPing sonar2(A2, A3, 200);

//-------PINS--------------
const int relayPin = 6;
const int flowSensorPin1 = 2;
const int flowSensorPin2 = 3;
const int levelPin1 = 4;
const int levelPin2 = 5;
const int ledPin = 9;
//-------------------------
volatile long flowRpm1;
volatile long flowRpm2;

volatile long oldFlowRpm1;
volatile long oldFlowRpm2;

unsigned long sonar1Val;
unsigned long sonar2Val;

unsigned long oldsonar1Val;
unsigned long oldsonar2Val;

int oldLevel1Status;
int oldLevel2Status;

long lastMillis;
long lastPingMillis;

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
	flowRpm1 = 0;
	flowRpm2 = 0;
}

void pumpOff()
{
	digitalWrite(relayPin, HIGH);
	Send("status/pump", "OFF");
}

void setup()
{
	pinMode(relayPin, OUTPUT);
	pinMode(ledPin, OUTPUT);
	pinMode(levelPin1, INPUT);
	pinMode(levelPin2, INPUT);
	digitalWrite(ledPin, HIGH);

	Serial.begin(9600);
	mySerial.begin(9600);
	//start the library, pass in the data details and the name of the serial port.
	ETin.begin(details(rxdata), &mySerial);
	ETout.begin(details(txdata), &mySerial);


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
	if (ETin.receiveData())
	{
		Serial.println(rxdata.topic);
		Serial.println((char*)rxdata.payload);
		Serial.println(rxdata.length);
		if (strcmp(rxdata.topic, "start") == 0)
		{
			SendAll();
		}
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
	if (millis() - lastPingMillis > 1000 || millis() < lastPingMillis)
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

		//sonar1Val = sonar1.ping_cm();
		//sonar2Val = sonar2.ping_cm();

		//if (sonar1Val != oldsonar1Val)
		//{
		//	Send("status/sonar1", sonar1Val);
		//	oldsonar1Val = sonar1Val;
		//}
		//if (sonar2Val != oldsonar2Val)
		//{
		//	Send("status/sonar2", sonar2Val);
		//	oldsonar2Val = sonar2Val;
		//}

		if (oldLevel1Status != digitalRead(levelPin1))
		{
			if (digitalRead(levelPin1) == HIGH)
				Send("status/level1", "ON");
			else
				Send("status/level1", "OFF");
			oldLevel1Status = digitalRead(levelPin1);
		}

		if (oldLevel2Status != digitalRead(levelPin2))
		{
			if (digitalRead(levelPin2) == HIGH)
				Send("status/level2", "ON");
			else
				Send("status/level2", "OFF");
			oldLevel2Status = digitalRead(levelPin2);
		}
		lastPingMillis = millis();
	}

	if (millis() - lastMillis > 45000 || millis() < lastMillis)
	{
		SendAll();
		lastMillis = millis();
	}

	/* add main program code here */

}

void SendAll()
{
	Serial.println(flowRpm1);
	Serial.println(flowRpm2);
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
	
	if (digitalRead(relayPin) == HIGH)
		Send("status/pump", "OFF");
	else
		Send("status/pump", "ON");

	Send("status/sonar1", sonar1.ping_cm());
	Send("status/sonar2", sonar2.ping_cm());
	Send("status/inttemp", (int)GetTemp());
}

void Send(char* topic, char* payload)
{
	digitalWrite(ledPin, HIGH);
	txdata.length = strlen(payload);
	strcpy(txdata.topic, topic);
	memcpy(txdata.payload, payload, txdata.length + 1);
	ETout.sendData();
	digitalWrite(ledPin, LOW);
}

void Send(char* topic, int payload)
{
	char intStr[3];
	itoa(payload, intStr, 10);
	Send(topic, intStr);
	Serial.println(topic);
}

double GetTemp(void)
{
	unsigned int wADC;
	double t;

	// The internal temperature has to be used
	// with the internal reference of 1.1V.
	// Channel 8 can not be selected with
	// the analogRead function yet.

	// Set the internal reference and mux.
	ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
	ADCSRA |= _BV(ADEN);  // enable the ADC

	delay(20);            // wait for voltages to become stable.

	ADCSRA |= _BV(ADSC);  // Start the ADC

						  // Detect end-of-conversion
	while (bit_is_set(ADCSRA, ADSC));

	// Reading register "ADCW" takes care of how to read ADCL and ADCH.
	wADC = ADCW;

	// The offset of 324.31 could be wrong. It is just an indication.
	t = (wADC - 324.31) / 1.22;

	// The returned temperature is in degrees Celsius.
	return (t);
}