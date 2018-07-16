

// Enable debug prints
#define MY_DEBUG

// Enable and select radio type attached
#define MY_RADIO_NRF24
//#define MY_RADIO_NRF5_ESB
//#define MY_RADIO_RFM69
//#define MY_RADIO_RFM95

#include <MySensors.h>
#include <SoftwareSerial.h>
#include <ModbusMaster.h>
#include <Bounce2.h>

//-------------------MODBUS
#define MAX485_DE      6
#define MAX485_RE_NEG  5



#define RELAY_PIN 3  // Arduino Digital I/O pin number for first relay (second on pin+1 etc)
#define NUMBER_OF_RELAYS 1 // Total number of attached relays
#define RELAY_ON 1  // GPIO value to write to turn on attached relay
#define RELAY_OFF 0 // GPIO value to write to turn off attached relay
#define BUTTON_PIN  2  // Arduino Digital I/O pin number for button 

#define CHILD_ID 1              // Id of the sensor child
#define CHILD_ID_PV 2             // Id of the sensor child
#define CHILD_ID_B 3             // Id of the sensor child
#define CHILD_ID_TEMP 4              // Id of the sensor child
#define CHILD_ID_RELAY 5              // Id of the sensor child
#define CHILD_ID_L 6             // Id of the sensor child
long lastMillis;
// instantiate ModbusMaster object
ModbusMaster node;
SoftwareSerial myModSerial(7, 4);

float battBhargeCurrent, bvoltage, ctemp, btemp, bremaining, lpower, lcurrent, pvvoltage, pvcurrent, pvpower;
//float stats_today_pv_volt_min, stats_today_pv_volt_max;
uint8_t result;
bool rs485DataReceived = true;
const int debug = 1;
//-------------------
MyMessage wattMsg(CHILD_ID,V_WATT);
MyMessage b_v_Msg(CHILD_ID_B,V_VOLTAGE);
MyMessage b_a_Msg(CHILD_ID_B,V_CURRENT);
MyMessage pv_v_Msg(CHILD_ID_PV,V_VOLTAGE);
MyMessage pv_a_Msg(CHILD_ID_PV,V_CURRENT);
MyMessage temp_Msg(CHILD_ID_TEMP,V_TEMP);
MyMessage msg(CHILD_ID_RELAY, V_LIGHT);
MyMessage l_a_Msg(CHILD_ID_L,V_CURRENT);
//--------------------


void presentation()
{
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Solar Meter", "1.0");

  // Register this device as power sensor
  present(CHILD_ID, S_POWER);
  present(CHILD_ID_PV, S_MULTIMETER);
  present(CHILD_ID_B, S_MULTIMETER);
  present(CHILD_ID_TEMP, S_TEMP);
  present(CHILD_ID_RELAY, S_BINARY);
  present(CHILD_ID_L, S_MULTIMETER);
}


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

void setup() {
  // put your setup code here, to run once:
   myModSerial.begin(115200);

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

  pinMode(RELAY_PIN, OUTPUT);
  send(msg.set(loadState(CHILD_ID_RELAY) ), true); // Send new state and request ack back
  //-----------------
}

void loop() {
  // put your main code here, to run repeatedly:
    if (millis() - lastMillis > 15000 || millis() < lastMillis)
  {
    myModSerial.listen();
    AddressRegistry_3100();
    AddressRegistry_311A();
    AddressRegistry_3300();
//    SendAll();
    lastMillis = millis();
    send(wattMsg.set(pvpower,2));
    send(b_v_Msg.set(bvoltage,2));
    send(b_a_Msg.set(battBhargeCurrent,2));
    send(pv_v_Msg.set(pvvoltage,2));
    send(pv_a_Msg.set(pvcurrent,2));
    send(l_a_Msg.set(lcurrent,2));
    send(temp_Msg.set(btemp,2));
    sendBatteryLevel(bremaining);
  }
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

}

void receive(const MyMessage &message)
{
  if (message.isAck()) {
   // ack = true;
  }
  // We only expect one type of message from controller. But we better check anyway.
  if (message.type == V_STATUS) {
    // Change relay state
    digitalWrite( RELAY_PIN, message.getBool() ? RELAY_ON : RELAY_OFF);
    // Store state in eeprom
    saveState(message.sensor, message.getBool());
//    state = message.getBool();
    // Write some debug info
    Serial.print("Incoming change for sensor:");
    Serial.print(message.sensor);
    Serial.print(", New status: ");
    Serial.println(message.getBool());
  }
}
