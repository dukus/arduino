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
#define MY_REPEATER_FEATURE

#include <MySensors.h>
#include <Bounce2.h>

#define RELAY_PIN 4  // Arduino Digital I/O pin number for first relay (second on pin+1 etc)
#define NUMBER_OF_RELAYS 1 // Total number of attached relays
#define RELAY_ON 0  // GPIO value to write to turn on attached relay
#define RELAY_OFF 1 // GPIO value to write to turn off attached relay
#define BUTTON_PIN  2  // Arduino Digital I/O pin number for button 
#define CHILD_ID 1   // Id of the sensor child-

Bounce debouncer = Bounce();
int oldValue = 0;
bool state;
bool ack = true;
MyMessage msg(CHILD_ID, V_LIGHT);


void before()
{
  for (int sensor = 1, pin = RELAY_PIN; sensor <= NUMBER_OF_RELAYS; sensor++, pin++) {
    // Then set relay pins in output mode
    pinMode(pin, OUTPUT);
    // Set relay to last known state (using eeprom storage)
    digitalWrite(pin, loadState(sensor) ? RELAY_ON : RELAY_OFF);
  }
}

void setup()
{ // Setup the button
  pinMode(BUTTON_PIN, INPUT);
  // Activate internal pull-up
  digitalWrite(BUTTON_PIN, HIGH);

  // After setting up the button, setup debouncer
  debouncer.attach(BUTTON_PIN);
  debouncer.interval(15);

  state = loadState(CHILD_ID);
  send(msg.set(state ), true); // Send new state and request ack back
}

void presentation()
{
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Relay Priza", "1.0");

  for (int sensor = 1, pin = RELAY_PIN; sensor <= NUMBER_OF_RELAYS; sensor++, pin++) {
    // Register all sensors to gw (they will be created as child devices)
    present(sensor, S_BINARY);
  }
}


void loop()
{
  debouncer.update();
  // Get the update value
  int value = debouncer.read();
  if (value != oldValue && value == 0) {
    if (ack) {
      ack = false;
      send(msg.set(state ? false : true), true); // Send new state and request ack back
    }
    else {
       send(msg.set(state ), true); // Send new state and request ack back
       digitalWrite( RELAY_PIN, state ? RELAY_OFF : RELAY_ON);
       state=!state;
    }
  }
  oldValue = value;
}

void receive(const MyMessage &message)
{
  if (message.isAck()) {
    ack = true;
  }
  // We only expect one type of message from controller. But we better check anyway.
  if (message.type == V_STATUS) {
    // Change relay state
    digitalWrite(message.sensor - 1 + RELAY_PIN, message.getBool() ? RELAY_ON : RELAY_OFF);
    // Store state in eeprom
    saveState(message.sensor, message.getBool());
    state = message.getBool();
    // Write some debug info
    Serial.print("Incoming change for sensor:");
    Serial.print(message.sensor);
    Serial.print(", New status: ");
    Serial.println(message.getBool());
  }
}

