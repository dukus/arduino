#include <SoftwareSerial.h>

/*

  RS485_HalfDuplex.pde - example using ModbusMaster library to communicate
  with EPSolar LS2024B controller using a half-duplex RS485 transceiver.

  This example is tested against an EPSolar LS2024B solar charge controller.
  See here for protocol specs:
  http://www.solar-elektro.cz/data/dokumenty/1733_modbus_protocol.pdf

  Library:: ModbusMaster
  Author:: Marius Kintel <marius at kintel dot net>

  Copyright:: 2009-2016 Doc Walker

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

*/

#include <ModbusMaster.h>

/*!
  We're using a MAX485-compatible RS485 Transceiver.
  Rx/Tx is hooked up to the hardware serial port at 'Serial'.
  The Data Enable and Receiver Enable pins are hooked up as follows:
*/
#define MAX485_DE      9
#define MAX485_RE_NEG  8

// instantiate ModbusMaster object
ModbusMaster node;
SoftwareSerial myModSerial(10, 11);

float battBhargeCurrent, bvoltage, ctemp, btemp, bremaining, lpower, lcurrent, pvvoltage, pvcurrent, pvpower;
float stats_today_pv_volt_min, stats_today_pv_volt_max;
uint8_t result;
bool rs485DataReceived = true;
const int debug             =           1; 


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

void setup()
{
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  // Init in receive mode
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);

  // Modbus communication runs at 115200 baud
  Serial.begin(9600);
  myModSerial.begin(115200);
  // Modbus slave ID 1
  node.begin(1, myModSerial);
  // Callbacks allow us to configure the RS485 transceiver correctly
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
}

bool state = true;

void loop()
{
  uint8_t result;
  uint16_t data[6];
  
  // Toggle the coil at address 0x0002 (Manual Load Control)
  ///result = node.writeSingleCoil(0x0002, state);
  //state = !state;

/*
  // Read 16 registers starting at 0x3100)
  result = node.readInputRegisters(0x3100, 16);
  if (result == node.ku8MBSuccess)
  {
    Serial.print("Vbatt: ");
    Serial.println(node.getResponseBuffer(0x04)/100.0f);
    Serial.print("Vload: ");
    Serial.println(node.getResponseBuffer(0x0C)/100.0f);
    Serial.print("Pload: ");
    Serial.println((node.getResponseBuffer(0x0D) +
                    node.getResponseBuffer(0x0E) << 16)/100.0f);
    
    
  }

    result = node.readInputRegisters(0x3100+16, 16);
  if (result == node.ku8MBSuccess)
  {
    Serial.print("Batt temp: ");
    Serial.println(node.getResponseBuffer(0x01));                    
  }
*/
  AddressRegistry_3100();
  AddressRegistry_311A();
  AddressRegistry_3300();
  
  delay(1000);
}

void AddressRegistry_3100() {
  result = node.readInputRegisters(0x3100, 7);
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
  } else {
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
  } else {
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
  } else {
    rs485DataReceived = false;
  }
}

