
/*
  Getting Started example sketch for nRF24L01+ radios
  This is a very basic example of how to send data from one node to another
  Updated: Dec 2014 by TMRh20
*/

#include <SPI.h>
#include "RF24.h"

/****************** User Config ***************************/
/***      Set this radio as radio number 0 or 1         ***/
#define RELAY1PIN  2
#define RELAY2PIN  3
#define RELAY3PIN  4
#define RELAY4PIN  5

bool radioNumber = 1;

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(7, 8);
int msg[1];
/**********************************************************/

byte addresses[][6] = {"MASTE", "2Node"};

void setup() {
  Serial.begin(9600);
  Serial.println(F("RF24/examples/GettingStarted"));
  Serial.println(F("*** PRESS 'T' to begin transmitting to the other node"));

  pinMode(RELAY1PIN, OUTPUT);
  pinMode(RELAY2PIN, OUTPUT);
  pinMode(RELAY3PIN, OUTPUT);
  pinMode(RELAY4PIN, OUTPUT);
  digitalWrite(RELAY1PIN, HIGH);
  digitalWrite(RELAY2PIN, HIGH);
  digitalWrite(RELAY3PIN, HIGH);
  digitalWrite(RELAY4PIN, HIGH);


  radio.begin();

  // Set the PA Level low to prevent power supply related issues since this is a
  // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  radio.setPALevel(RF24_PA_LOW);

  radio.begin();
  radio.openReadingPipe(1, addresses[0]);
  radio.startListening();
}

void loop() {

  if (radio.available()) {
    bool done = false;
    while (!done) {
      radio.read(msg, 1);
      done = true;
      Serial.println(msg[0]);
      if (msg[0] == 111) {
        delay(10);
        digitalWrite(RELAY1PIN, LOW);
      }
      if (msg[0] == 110) {
        delay(10);
        digitalWrite(RELAY1PIN, HIGH);
      }      
      if (msg[0] == 121) {
        delay(10);
        digitalWrite(RELAY1PIN, LOW);
      }
      if (msg[0] == 120) {
        delay(10);
        digitalWrite(RELAY1PIN, HIGH);
      }      
      if (msg[0] == 131) {
        delay(10);
        digitalWrite(RELAY1PIN, LOW);
      }
      if (msg[0] == 130) {
        delay(10);
        digitalWrite(RELAY1PIN, HIGH);
      }      
      if (msg[0] == 141) {
        delay(10);
        digitalWrite(RELAY1PIN, LOW);
      }
      if (msg[0] == 140) {
        delay(10);
        digitalWrite(RELAY1PIN, HIGH);
      }      
      
      delay(10);
    }
  }
  else {
   // Serial.println("No radio available");
  }


  /****************** Change Roles via Serial Commands ***************************/

  if ( Serial.available() )
  {
    char c = toupper(Serial.read());
    if ( c == '1') {
      digitalWrite(RELAY1PIN, HIGH);
      Serial.println("Relay on");
    }
  }


} // Loop

