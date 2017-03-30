#include <LiquidCrystal_I2C.h>
#include <Adafruit_BMP085.h>
#include <DHT.h>
#include <SoftwareSerial.h>
#include <SoftEasyTransfer.h>

#define DHTPIN 7 
#define DHTTYPE DHT11
#define VOLTPIN A7 
#define AMPPIN A6 
#define INVOLTPIN A0 
#define BUTTONPIN 11
#define RELAY1PIN 2
#define RELAY2PIN 3

byte wifiChar[8] = {
	0b00001,
	0b00000,
	0b00011,
	0b00000,
	0b00111,
	0b00000,
	0b01111,
	0b11111
};

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

DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp;
LiquidCrystal_I2C lcd(0x27, 16, 2);


int oldLevel1Status;
int oldLevel2Status;

long lastMillis;
long lastPingMillis;

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

	Serial.begin(9600);
	mySerial.begin(9600);
	//start the library, pass in the data details and the name of the serial port.
	ETin.begin(details(rxdata), &mySerial);
	ETout.begin(details(txdata), &mySerial);

	relay1On();
	relay2Off();

	dht.begin();

	if (!bmp.begin()) {
		Serial.println("Could not find a valid BMP085 sensor, check wiring!");
	}

	lcd.init();
	lcd.backlight();
	lcd.createChar(0, wifiChar);
	lcd.clear();
	lcd.print("Started");
	// delay startup until esp started
	Send("Status", "Started");
	SendAll();
}

void loop()
{
	if (ETin.receiveData())
	{
		//Serial.println(rxdata.topic);
		//Serial.println((char*)rxdata.payload);
		//Serial.println(rxdata.length);
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
		SendAll();
		lastMillis = millis();
	}
	if (digitalRead(BUTTONPIN) == HIGH)
	{
		lcd.backlight();
	}
	else
	{
		lcd.noBacklight();
	}
	/* add main program code here */

}

void SendAll()
{
	lcd.setCursor(0, 0);
	float t = dht.readTemperature();
	float h = dht.readHumidity();
	if (isnan(h) || isnan(t)) {
		Serial.println("Failed to read from DHT sensor!");
	}
	else
	{
		Serial.print("Humidity: ");
		Serial.print(h);
		Send("status/humidity", h);
		Serial.print(" %\t");
		Serial.print("Temperature: ");
		Serial.print(t);
		lcd.print((int)t);
		lcd.print("C ");
		Send("status/temperature0", t);
		Serial.println(" *C ");
	}



	Serial.print("Temperature = ");
	Serial.print(bmp.readTemperature());
	Serial.println(" *C");
	Send("status/temperature1", (int)bmp.readPressure());

	Serial.print("Pressure = ");
	Serial.print(bmp.readPressure());
	Serial.println(" Pa");
	Send("status/pressure", (int)bmp.readPressure());

	//Serial.print("Pressure at sealevel (calculated) = ");
	//Serial.print((int)bmp.readSealevelPressure());
	//Serial.println(" Pa");

	Send("status/inttemp", (int)GetTemp());

	//int sensorValue = getVPP(VOLTPIN);
	//// Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
	//float voltage = sensorValue *(5.0/1024.0) *5;
	//// print out the value you read:
	//Serial.print("Voltage ");
	//Serial.println(voltage);
	//Send("status/voltage", voltage);

	int sensorValue = getVPP(INVOLTPIN);
	// Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
	float voltage = sensorValue *(5.0 / 1024.0) * 5;
	// print out the value you read:
	Serial.print("In Voltage ");
	Serial.println(voltage);
	Send("status/batteryvoltage", voltage);

	lcd.print(voltage);
	lcd.print("V ");


	sensorValue = getVPP(AMPPIN);
	// Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
	float amp = ((sensorValue *(5.0 / 1024.0)) - 2.5) / 0.066;
	// print out the value you read:
	Serial.print("Amper ");
	Serial.println(amp);
	Send("status/amper", amp);
	lcd.print(amp);
	lcd.print("A ");

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