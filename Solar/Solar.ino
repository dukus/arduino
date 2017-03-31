#include <SoftwareSerial.h>
#include <SoftEasyTransfer.h>
#include <ModbusMaster.h>

#define DHTPIN 7 
#define DHTTYPE DHT11
#define VOLTPIN A7 
#define AMPPIN A7 
#define INVOLTPIN A0 
#define BUTTONPIN 11
#define RELAY1PIN 3
#define RELAY2PIN 3

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

SoftwareSerial mySerial(9, 8);

int oldLevel1Status;
int oldLevel2Status;

long lastMillis;
long lastPingMillis;

//-------------------MODBUS
#define MAX485_DE      6
#define MAX485_RE_NEG  5

// instantiate ModbusMaster object
ModbusMaster node;
SoftwareSerial myModSerial(7, 4);

float battBhargeCurrent, bvoltage, ctemp, btemp, bremaining, lpower, lcurrent, pvvoltage, pvcurrent, pvpower;
float stats_today_pv_volt_min, stats_today_pv_volt_max;
uint8_t result;
bool rs485DataReceived = true;
const int debug = 1;


void preTransmission()
{
	digitalWrite(MAX485_RE_NEG, 1);
	digitalWrite(MAX485_DE, 1);
}

void postTransmission()
{
	digitalWrite(MAX485_RE_NEG, 0);
	digitalWrite(MAX485_DE, 0);
}

//-------------------

void relay1On()
{
	digitalWrite(RELAY1PIN, LOW);
	Send("status/relay1", "ON");
}

void relay1Off()
{
	digitalWrite(RELAY1PIN, HIGH);
	Send("status/relay1", "OFF");
}

void relay2On()
{
	digitalWrite(RELAY2PIN, LOW);
	Send("status/relay2", "ON");
}


void relay2Off()
{
	digitalWrite(RELAY2PIN, HIGH);
	Send("status/relay2", "OFF");
}

void setup()
{

	pinMode(RELAY1PIN, OUTPUT);
	pinMode(RELAY2PIN, OUTPUT);


	pinMode(BUTTONPIN, INPUT);
	myModSerial.begin(115200);

	Serial.begin(9600);
	mySerial.begin(9600);
	//start the library, pass in the data details and the name of the serial port.
	ETin.begin(details(rxdata), &mySerial);
	ETout.begin(details(txdata), &mySerial);

	relay1Off();
	relay2Off();

	// delay startup until esp started
	Send("Status", "Started");
	SendAll();

	//-----------------
	pinMode(MAX485_RE_NEG, OUTPUT);
	pinMode(MAX485_DE, OUTPUT);
	// Init in receive mode
	digitalWrite(MAX485_RE_NEG, 0);
	digitalWrite(MAX485_DE, 0);

	// Modbus communication runs at 115200 baud
	
	// Modbus slave ID 1
	node.begin(1, myModSerial);
	// Callbacks allow us to configure the RS485 transceiver correctly
	node.preTransmission(preTransmission);
	node.postTransmission(postTransmission);
	//-----------------
}

void loop()
{
	mySerial.listen();
	if (ETin.receiveData())
	{
		Serial.println(rxdata.topic);
		Serial.println((char*)rxdata.payload);
		Serial.println(rxdata.length);
		if (strcmp(rxdata.topic, "start") == 0)
		{
			SendAll();
		}
		if (strcmp(rxdata.topic, "relay1") == 0) {
			Serial.println("changing");
			if (rxdata.payload[0] == '1')
			{
				relay1On();
				Serial.println("relay1 ON");
			}
			else
			{
				relay1Off();
				Serial.println("relay1 OFF");
			}
		}
		if (strcmp(rxdata.topic, "relay2") == 0) {
			Serial.println("changing");
			if (rxdata.payload[0] == '1')
			{
				relay2On();
			}
			else
			{
				relay2Off();
			}
		}
	}

	if (millis() - lastPingMillis > 1000 || millis() < lastPingMillis)
	{

		lastPingMillis = millis();
	}

	if (millis() - lastMillis > 15000 || millis() < lastMillis)
	{
		myModSerial.listen();
		AddressRegistry_3100();
		AddressRegistry_311A();
		AddressRegistry_3300();
		SendAll();
		lastMillis = millis();
	}
}

void SendAll()
{
	float sensorValue = getVPP(AMPPIN);
	// Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
	float amp = ((sensorValue *(5.0 / 1024.0)) - 2.5) / 0.066;
	// print out the value you read:
	Serial.print("Amper ");
	Serial.println(amp);
	Send("status/amper", amp);
	Send("status/pvpower", pvpower);
	Send("status/pvcurrent", pvcurrent);
	Send("status/pvvoltage", pvvoltage);
	Send("status/lcurrent", lcurrent);
	Send("status/lpower", lpower);
	Send("status/btemp", btemp);
	Send("status/bvoltage", bvoltage);
	Send("status/bremaining", bremaining);
	Send("status/battBhargeCurrent", battBhargeCurrent);
	Send("status/ctemp", ctemp);
	
}

void Send(char* topic, char* payload)
{
	//digitalWrite(ledPin, HIGH);
	txdata.length = strlen(payload);
	strcpy(txdata.topic, topic);
	memcpy(txdata.payload, payload, txdata.length + 1);
	ETout.sendData();
	//digitalWrite(ledPin, LOW);
}

void Send(char* topic, int payload)
{
	char intStr[3];
	itoa(payload, intStr, 10);
	Send(topic, intStr);
}

void Send(char* topic, float payload)
{
	char intStr[10];
	dtostrf(payload, 4, 2, intStr);
	Send(topic, intStr);
}

float getVPP(int sensorIn)
{
	unsigned int x = 0;
	float AcsValue = 0.0, Samples = 0.0, AvgAcs = 0.0, AcsValueF = 0.0;

	for (int x = 0; x < 25; x++) { //Get 150 samples
		AcsValue = analogRead(sensorIn);     //Read current sensor values   
		Samples = Samples + AcsValue;  //Add samples together
		delay(1); // let ADC settle before next sample 3ms
	}
	AvgAcs = Samples / 25.0;//Taking Average of Samples
	return AvgAcs;
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

void AddressRegistry_3100() {
	result = node.readInputRegisters(0x3100, 7);
	Serial.print(result);
	if (result == node.ku8MBSuccess)
	{
		ctemp = node.getResponseBuffer(0x11) / 100.0f;
		if (debug == 1) {
			Serial.println(ctemp);
			Serial.print("Battery Voltage: ");
		}
		bvoltage = node.getResponseBuffer(0x04) / 100.0f;
		if (debug == 1) {
			Serial.println(bvoltage);

		}
		lpower = ((long)node.getResponseBuffer(0x0F) << 16 | node.getResponseBuffer(0x0E)) / 100.0f;
		if (debug == 1) {
			Serial.print("Load Power: ");
			Serial.println(lpower);

		}
		lcurrent = (long)node.getResponseBuffer(0x0D) / 100.0f;
		if (debug == 1) {
			Serial.print("Load Current: ");
			Serial.println(lcurrent);

		}
		pvvoltage = (long)node.getResponseBuffer(0x00) / 100.0f;
		if (debug == 1) {
			Serial.print("PV Voltage: ");
			Serial.println(pvvoltage);

		}
		pvcurrent = (long)node.getResponseBuffer(0x01) / 100.0f;
		if (debug == 1) {
			Serial.print("PV Current: ");
			Serial.println(pvcurrent);

		}
		pvpower = ((long)node.getResponseBuffer(0x03) << 16 | node.getResponseBuffer(0x02)) / 100.0f;
		if (debug == 1) {
			Serial.print("PV Power: ");
			Serial.println(pvpower);
		}
		battBhargeCurrent = (long)node.getResponseBuffer(0x05) / 100.0f;
		if (debug == 1) {
			Serial.print("Battery Charge Current: ");
			Serial.println(battBhargeCurrent);
			Serial.println();
		}
	}
	else {
		rs485DataReceived = false;
	}
}

void AddressRegistry_311A() {
	result = node.readInputRegisters(0x311A, 2);
	if (result == node.ku8MBSuccess)
	{
		bremaining = node.getResponseBuffer(0x00) / 1.0f;
		if (debug == 1) {
			Serial.print("Battery Remaining %: ");
			Serial.println(bremaining);

		}
		btemp = node.getResponseBuffer(0x01) / 100.0f;
		if (debug == 1) {
			Serial.print("Battery Temperature: ");
			Serial.println(btemp);
			Serial.println();
		}
	}
	else {
		rs485DataReceived = false;
	}
}

void AddressRegistry_3300() {
	result = node.readInputRegisters(0x3300, 2);
	if (result == node.ku8MBSuccess)
	{
		stats_today_pv_volt_max = node.getResponseBuffer(0x00) / 100.0f;
		if (debug == 1) {
			Serial.print("Stats Today PV Voltage MAX: ");
			Serial.println(stats_today_pv_volt_max);
		}
		stats_today_pv_volt_min = node.getResponseBuffer(0x01) / 100.0f;
		if (debug == 1) {
			Serial.print("Stats Today PV Voltage MIN: ");
			Serial.println(stats_today_pv_volt_min);
		}
	}
	else {
		rs485DataReceived = false;
	}
}