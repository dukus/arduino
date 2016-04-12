#include <SpacebrewYun.h>


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
#define LEDPIN  6
#define BUTTON1PIN  A3
#define BUTTON2PIN  A4
#define BUTTON3PIN  A1
#define BUTTON4PIN  A0

bool radioNumber = 1;

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(7, 8);
int msg[1];
/**********************************************************/

byte addresses[][6] = {"MASTE", "2Node"};
int ledState = LOW;
unsigned long previousMillis = 0;        // will store last time LED was updated
long interval = 1000;
int button1State = LOW;
int button2State = LOW;
int button3State = LOW;
int button4State = LOW;

void setup() {
  Serial.begin(9600);
  Serial.println(F("RF24/examples/GettingStarted"));
  Serial.println(F("*** PRESS 'T' to begin transmitting to the other node"));

  pinMode(RELAY1PIN, OUTPUT);
  pinMode(RELAY2PIN, OUTPUT);
  pinMode(RELAY3PIN, OUTPUT);
  pinMode(RELAY4PIN, OUTPUT);
  pinMode(LEDPIN, OUTPUT);
  pinMode(BUTTON1PIN, INPUT);
  pinMode(BUTTON2PIN, INPUT);
  pinMode(BUTTON3PIN, INPUT);
  pinMode(BUTTON4PIN, INPUT);


  digitalWrite(RELAY1PIN, HIGH);
  digitalWrite(RELAY2PIN, HIGH);
  digitalWrite(RELAY3PIN, HIGH);
  digitalWrite(RELAY4PIN, HIGH);
  digitalWrite(LEDPIN, HIGH);
  digitalWrite(BUTTON1PIN, HIGH);
  digitalWrite(BUTTON2PIN, HIGH);
  digitalWrite(BUTTON3PIN, HIGH);
  digitalWrite(BUTTON4PIN, HIGH);

  button1State = digitalRead(BUTTON1PIN);
  button2State = digitalRead(BUTTON2PIN);
  button3State = digitalRead(BUTTON3PIN);
  button4State = digitalRead(BUTTON4PIN);

  radio.begin();

  // Set the PA Level low to prevent power supply related issues since this is a
  // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  //radio.setPALevel(RF24_PA_LOW);

  radio.begin();
  radio.openReadingPipe(1, addresses[0]);
  radio.startListening();
}

void loop() {

  int b1 = digitalRead(BUTTON1PIN);
  int b2 = digitalRead(BUTTON2PIN);
  int b3 = digitalRead(BUTTON3PIN);
  int b4 = digitalRead(BUTTON4PIN);

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    // set the LED with the ledState of the variable:
    digitalWrite(LEDPIN, ledState);
  }

  if (b1 != button1State)
  {
    if (b1 == HIGH)
    {
      relayOff(1);
    } else
    {
      relayOn(1);
    }
    button1State = b1;
  }

  if (b2 != button2State)
  {
    if (b2 == HIGH)
    {
      relayOff(2);
    } else
    {
      relayOn(2);
    }
    button2State = b2;
  }

  if (b3 != button3State)
  {
    if (b3 == HIGH)
    {
      relayOff(3);
    } else
    {
      relayOn(3);
    }
    button3State = b3;
  }

  if (b4 != button4State)
  {
    if (b4 == HIGH)
    {
      relayOff(4);
    } else
    {
      relayOn(4);
    }
    button4State = b4;
  }



  if (radio.available()) {
    bool done = false;
    while (!done) {
      radio.read(msg, 1);
      done = true;
      Serial.println(msg[0]);
      if (msg[0] == 111) {
        relayOn(1);
      }
      if (msg[0] == 110) {
        relayOff(1);
      }
      if (msg[0] == 121) {
        relayOn(2);
      }
      if (msg[0] == 120) {
        relayOff(2);
      }
      if (msg[0] == 131) {
        relayOn(3);
      }
      if (msg[0] == 130) {
        relayOff(3);
      }
      if (msg[0] == 141) {
        relayOn(4);
      }
      if (msg[0] == 140) {
        relayOff(4);
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
      fastBlink();
    }
  }


} // Loop

void fastBlink()
{
  for (int i = 0; i <= 20; i++) {
    delay(35);
    digitalWrite(LEDPIN, LOW);
    delay(35);
    digitalWrite(LEDPIN, HIGH);
  }
}

void relayOff(int relay)
{
  Serial.print("Relay OFF ");
  Serial.println(relay);
  switch (relay) {
    case 1:
      delay(10);
      digitalWrite(RELAY1PIN, HIGH);
      fastBlink();
      break;
    case 2:
      delay(10);
      digitalWrite(RELAY2PIN, HIGH);
      fastBlink();
      break;
    case 3:
      delay(10);
      digitalWrite(RELAY3PIN, HIGH);
      fastBlink();
      break;
    case 4:
      delay(10);
      digitalWrite(RELAY4PIN, HIGH);
      fastBlink();
      break;
    default:
      // if nothing else matches, do the default
      // default is optional
      break;
  }
}

void relayOn(int relay)
{
  Serial.print("Relay ON ");
  Serial.println(relay);
  switch (relay) {
    case 1:
      delay(10);
      digitalWrite(RELAY1PIN, LOW);
      fastBlink();
      break;
    case 2:
      delay(10);
      digitalWrite(RELAY2PIN, LOW);
      fastBlink();
      break;
    case 3:
      delay(10);
      digitalWrite(RELAY3PIN, LOW);
      fastBlink();
      break;
    case 4:
      delay(10);
      digitalWrite(RELAY4PIN, LOW);
      fastBlink();
      break;
    default:
      // if nothing else matches, do the default
      // default is optional
      break;
  }
}

