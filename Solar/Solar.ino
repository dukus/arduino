#include <DHT.h>
#include <SoftwareSerial.h>
#include <SoftEasyTransfer.h>

#define DHTPIN 7 
#define DHTTYPE DHT11

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

DHT dht(DHTPIN, DHTTYPE);

//-------PINS--------------
const int relayPin = 6;
const int ledPin = 9;
//-------------------------

int oldLevel1Status;
int oldLevel2Status;

long lastMillis;
long lastPingMillis;


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
	pinMode(relayPin, OUTPUT);
	pinMode(ledPin, OUTPUT);
	digitalWrite(ledPin, HIGH);

	Serial.begin(9600);
	mySerial.begin(9600);
	//start the library, pass in the data details and the name of the serial port.
	ETin.begin(details(rxdata), &mySerial);
	ETout.begin(details(txdata), &mySerial);


	pumpOff();
	dht.begin();


	// delay startup until esp started
	Send("Status", "Started");
	SendAll();	
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

		lastPingMillis = millis();
	}

	if (millis() - lastMillis > 5000 || millis() < lastMillis)
	{
		SendAll();
		lastMillis = millis();
	}

	/* add main program code here */

}

void SendAll()
{
	float t = dht.readTemperature();
	float h = dht.readHumidity();
	if (isnan(h) || isnan(t) ) {
		Serial.println("Failed to read from DHT sensor!");
	}
	else
	{
		Serial.print("Humidity: ");
		Serial.print(h);
		Serial.print(" %\t");
		Serial.print("Temperature: ");
		Serial.print(t);
		Serial.println(" *C ");

	}
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