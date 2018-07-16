/**
   The MySensors Arduino library handles the wireless radio link and protocol
   between your home built sensors/actuators and HA controller of choice.
   The sensors forms a self healing radio network with optional repeaters. Each
   repeater and gateway builds a routing tables in EEPROM which keeps track of the
   network topology allowing messages to be routed to nodes.

   Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
   Copyright (C) 2013-2015 Sensnology AB
   Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors

   Documentation: http://www.mysensors.org
   Support Forum: http://forum.mysensors.org

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   version 2 as published by the Free Software Foundation.

 *******************************

   REVISION HISTORY
   Version 1.0 - Henrik Ekblad

   DESCRIPTION
   Example sketch showing how to control physical relays.
   This example will remember relay state after power failure.
   http://www.mysensors.org/build/relay
*/

// Enable debug prints to serial monitor
#define MY_DEBUG

// Enable and select radio type attached
#define MY_RADIO_NRF24
//#define MY_RADIO_NRF5_ESB
//#define MY_RADIO_RFM69
//#define MY_RADIO_RFM95

// Enable repeater functionality for this node
//#define MY_REPEATER_FEATURE
#include <SPI.h>
#include <MySensors.h>
#include <Bounce2.h>
#include <Wire.h>


#define FLOW1_PIN 2
#define FLOW2_PIN 3
#define LEVEL1_PIN 4
#define LEVEL2_PIN 5

#define FLOW1_FACTOR 5.5
#define FLOW2_FACTOR 5.5

#define NUMBER_OF_RELAYS 8 // Total number of attached relays
#define RELAY_ON 0  // GPIO value to write to turn on attached relay
#define RELAY_OFF 1 // GPIO value to write to turn off attached relay
#define BUTTON_PIN  2  // Arduino Digital I/O pin number for button 
#define CHILD_ID 1   // Id of the sensor child-

//------------------
// 8574  Address range is 0x20-0x27
// 8574A Address range is 0x38-0x3F
// 9555  Address range is 0x20-0x27 (same as 8574, bummer)

//#define INaddr 0x38  // 8574A addr 000
#define OUTaddr 0x27 // 8574A addr 001

#define NXP_INPUT      (0)  // For NXP9555
#define NXP_OUTPUT     (2)  // See data sheet
#define NXP_INVERT     (4)  // for details...
#define NXP_CONFIG     (6)
//------------------

uint32_t SEND_FREQUENCY =    30000;           // Minimum time between send (in milliseconds). We don't want to spam the gateway.

bool relays[NUMBER_OF_RELAYS] = {};
Bounce debouncer = Bounce();
int oldLevel1 = -1;
int oldLevel2 = -1;


bool state;
bool ack = true;
MyMessage msg(CHILD_ID, V_LIGHT);

MyMessage flowMsg1(NUMBER_OF_RELAYS + 1, V_FLOW);
MyMessage volumeMsg1(NUMBER_OF_RELAYS + 1, V_VOLUME);
MyMessage lastCounterMsg1(NUMBER_OF_RELAYS + 1, V_VAR1);

MyMessage flowMsg2(NUMBER_OF_RELAYS + 2, V_FLOW);
MyMessage volumeMsg2(NUMBER_OF_RELAYS + 2, V_VOLUME);
MyMessage lastCounterMsg2(NUMBER_OF_RELAYS + 2, V_VAR1);

MyMessage levelMsg1(NUMBER_OF_RELAYS + 3, V_STATUS);
MyMessage levelMsg2(NUMBER_OF_RELAYS + 4, V_STATUS);

volatile long flowRpm1;
volatile long flowRpm2;

volatile long flow1;
volatile long flow2;
volatile long oldflow1;
volatile long oldflow2;


volatile long oldFlowRpm1;
volatile long oldFlowRpm2;

volatile uint32_t lastBlink1 = 0;
volatile uint32_t lastBlink2 = 0;

uint32_t lastPulse1 = 0;
uint32_t lastPulse2 = 0;

uint32_t lastSend = 0;

void rpm1()     //This is the function that the interupt calls
{
  uint32_t newBlink = micros();
  uint32_t interval = newBlink - lastBlink1;

  if (interval != 0) {
    lastPulse1 = millis();
    //    if (interval < 500000L) {
    //      // Sometimes we get interrupt on RISING,  500000 = 0.5sek debounce ( max 120 l/min)
    //      return;
    //    }
    flow1 = (60000000.0 / interval) / FLOW1_FACTOR;
  }
  lastBlink1 = newBlink;
  flowRpm1++;  //This function measures the rising and falling edge of the  hall effect sensors signal
}

void rpm2()     //This is the function that the interupt calls
{
  uint32_t newBlink = micros();
  uint32_t interval = newBlink - lastBlink2;

  if (interval != 0) {
    lastPulse2 = millis();
    if (interval < 500000L) {
      // Sometimes we get interrupt on RISING,  500000 = 0.5sek debounce ( max 120 l/min)
      return;
    }
    flow2 = (60000000.0 / interval) / FLOW2_FACTOR;
  }
  lastBlink2 = newBlink;
  flowRpm2++;  //This function measures the rising and falling edge of the  hall effect sensors signal
}


// I2C routines to talk to 8574 and 8574A
void expanderSetInput(int i2caddr, byte dir) {
  Wire.beginTransmission(i2caddr);
  Wire.write(dir);  // outputs high for input
  Wire.endTransmission();
}

void expanderWrite(int i2caddr, byte data)
{
  Wire.beginTransmission(i2caddr);
  Wire.write(data);
  int i = Wire.endTransmission();
  if (i != 0) {
    Serial.print("Write error ");
    Serial.println(i);
  }
}

void writeSpi()
{
  byte data = 0;
  for (int sensor = 0; sensor < NUMBER_OF_RELAYS; sensor++) {
    //Serial.print(relays[sensor] ? RELAY_ON : RELAY_OFF);
    // bitWrite(data, sensor, relays[sensor] );
    bitWrite(data, sensor, !relays[sensor] );
    
  }
  //Serial.print("Write spi :");
  //Serial.println((byte)data);
  
  //!!!!!!
  expanderWrite(OUTaddr, (byte)data);
}

void before()
{
  /*
  for (int sensor = 1 ; sensor <= NUMBER_OF_RELAYS; sensor++) {
    // Then set relay pins in output mode
    //pinMode(pin, OUTPUT);
    // Set relay to last known state (using eeprom storage)
    // digitalWrite(pin, loadState(sensor) ? RELAY_ON : RELAY_OFF);
    //relays[sensor] = loadState(sensor);
    request(sensor, V_STATUS);
  }
//  writeSpi();
*/
}

void setup()
{ // Setup the button
 
  for (int sensor = 1 ; sensor <= NUMBER_OF_RELAYS; sensor++) {
    relays[sensor - 1]=false;
  }
  delay(50);
  Wire.begin();
  //delay(50);
  // prevent write error on start up
  ///expanderWrite(OUTaddr, 0);
  //expanderWrite(OUTaddr, 0);

  writeSpi();
  writeSpi();
  writeSpi();
  //expanderSetInput(INaddr, 0xFF);
  //expanderWrite(OUTaddr, (byte)0xFF);

  pinMode(BUTTON_PIN, INPUT);
  pinMode(LEVEL1_PIN, INPUT);
  pinMode(LEVEL2_PIN, INPUT);
  // Activate internal pull-up
  digitalWrite(BUTTON_PIN, HIGH);
  digitalWrite(LEVEL1_PIN, HIGH);
  digitalWrite(LEVEL2_PIN, HIGH);

  // After setting up the button, setup debouncer
  debouncer.attach(BUTTON_PIN);
  debouncer.interval(15);

  pinMode(FLOW1_PIN, INPUT_PULLUP);
  pinMode(FLOW2_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FLOW1_PIN), rpm1, RISING);
  attachInterrupt(digitalPinToInterrupt(FLOW2_PIN), rpm2, RISING);

  for (int sensor = 1 ; sensor <= NUMBER_OF_RELAYS; sensor++) {
    // Then set relay pins in output mode
    //pinMode(pin, OUTPUT);
    // Set relay to last known state (using eeprom storage)
    // digitalWrite(pin, loadState(sensor) ? RELAY_ON : RELAY_OFF);
    //relays[sensor] = loadState(sensor);
    request(sensor, V_STATUS);
    wait(200);
  }


}

void presentation()
{
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Relay Garden", "1.0");

  for (int sensor = 1; sensor <= NUMBER_OF_RELAYS; sensor++) {
    // Register all sensors to gw (they will be created as child devices)
    present(sensor, S_BINARY);
  }

  present(NUMBER_OF_RELAYS + 1, S_WATER);
  present(NUMBER_OF_RELAYS + 2, S_WATER);
  present(NUMBER_OF_RELAYS + 3, S_BINARY);
  present(NUMBER_OF_RELAYS + 4, S_BINARY);
}


void loop()
{
  //  debouncer.update();
  //  // Get the update value
  //  int value = debouncer.read();
  //  if (value != oldValue && value == 0) {
  //    //    if (ack) {
  //    //      ack = false;
  //    //      send(msg.set(state ? false : true), true); // Send new state and request ack back
  //    //    }
  //    //    else {
  //    //      send(msg.set(state ), true); // Send new state and request ack back
  //    //      digitalWrite( RELAY_PIN, state ? RELAY_OFF : RELAY_ON);
  //    //      state = !state;
  //    //    }
  //  }
  //  oldValue = value;

  int value1 = digitalRead(LEVEL1_PIN);
  if (value1 != oldLevel1)
  {
    send(levelMsg1.set(value1 == 0 ? true : false ), true);
    Serial.print("level1");
    oldLevel1 = value1;
  }

  int value2 = digitalRead(LEVEL2_PIN);
  if (value2 != oldLevel2)
  {
    send(levelMsg2.set(value2 == 0 ? true : false ), true);
    Serial.print("level2");
    oldLevel2 = value2;
  }

  uint32_t currentTime = millis();

  // Only send values at a maximum frequency or woken up from sleep
  if ((currentTime - lastSend > SEND_FREQUENCY)) {
    lastSend = currentTime;
   // Serial.println(flowRpm1);
    if (flow1 != oldflow1) {
      oldflow1 = flow1;
      send(flowMsg1.set(flow1, 2));
      send(volumeMsg1.set(flowRpm1 / (float)FLOW1_FACTOR, 2));
      send(lastCounterMsg1.set(flowRpm1, 0));
      Serial.print("l/min:");
      Serial.println(flow1);
    }

    if (flow2 != oldflow2) {
      oldflow2 = flow2;
      send(flowMsg2.set(flow2, 2));
      send(volumeMsg2.set(flowRpm2 / (float)FLOW2_FACTOR, 2));
      send(lastCounterMsg1.set(flowRpm2, 0));
      Serial.print("l/min:");
      Serial.println(flow2);
    }
  }
}

void receive(const MyMessage & message)
{
  if (message.isAck()) {
    ack = true;
  }
  
  // We only expect one type of message from controller. But we better check anyway.
  if (message.type == V_STATUS) {
    // Change relay state
    // Store state in eeprom
    if(message.sensor<9){
    //saveState(message.sensor, message.getBool());
    state = message.getBool();
    relays[message.sensor - 1] = message.getBool();
    // Write some debug info
    Serial.print("Incoming change for sensor:");
    Serial.println(message.sensor);
    //Serial.print(", New status: ");
    //Serial.println(message.getBool());
    //expanderWrite(OUTaddr, (byte)0x00);
    writeSpi();
    }
  }
  
}

