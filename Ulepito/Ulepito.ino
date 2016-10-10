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

SoftwareSerial mySerial(9, 8);
//-------PINS--------------
const int relayPin = 6;
//-------------------------

void setup()
{
	Serial.begin(9600);
	mySerial.begin(9600);
	pinMode(relayPin, OUTPUT);
	digitalWrite(relayPin, HIGH);

	//start the library, pass in the data details and the name of the serial port.
	ETin.begin(details(rxdata), &mySerial);
	ETout.begin(details(txdata), &mySerial);
    // delay startup until esp started
	Send("Status", "Started");
}

void loop()
{
	//if (mySerial.available()) {
	//	Serial.write(mySerial.read());
	//}
	
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
				digitalWrite(relayPin, LOW);
			}
			else
			{
				digitalWrite(relayPin, HIGH);
			}
		}

	}

  /* add main program code here */

}

void Send(char* topic, char* payload)
{
	txdata.length = strlen(payload);
	strcpy(txdata.topic, topic);
	memcpy(txdata.payload, payload, txdata.length+1);
	Serial.println(txdata.topic);
	ETout.sendData();
}