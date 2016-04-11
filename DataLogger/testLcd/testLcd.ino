#include <nRF24L01.h>
#include <printf.h>
#include <RF24.h>
#include <RF24_config.h>

#include <DHT.h>
//YWROBOT
//Compatible with the Arduino IDE 1.0
//Library version:1.1
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DS3232RTC.h>        //http://github.com/JChristensen/DS3232RTC
#include <Time.h>             //http://playground.arduino.cc/Code/Time
#include <Adafruit_BMP085.h>
#include "structs.h"
//----------------
#define DHTPIN 49     // what digital pin we're connected to

// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11
//-----------------------------------------

LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display
Adafruit_BMP085 bmp;
File myFile;
DHT dht(DHTPIN, DHTTYPE);
RF24 radio(45, 47);
int msg[1];
DATA data;
//----------------------
String time = "";
int internTemp = 0;
float temp = 0;
float input_voltage = 0.0;
float baterry_voltage = 0.0;
float r1 = 6830.0;
float r2 = 2180.0;
String inputString = "";
String command = "";
bool radioNumber = 0;
unsigned long lcd_time;
byte addresses[][6] = {"MASTE", "2Node"};
//----------------------
void setup()
{

  lcd.init();                      // initialize the lcd
  setSyncProvider(RTC.get);
  Serial.begin(9600);
  // bluetooth
  Serial3.begin(9600);
  inputString.reserve(200);  
  //===================== SD CARD
  if (!SD.begin(53)) {
    Serial.println("SD card initialization failed.");
    Serial3.println("SD card initialization failed.");
  }
    
  debug("====Device starting======");
  if (!bmp.begin()) {
   error("Could not find a valid BMP085 sensor, check wiring!");
  }
  
  // Print a message to the LCD.
  activateLcd();
  dht.begin();

  radio.begin();
  //radio.setPALevel(RF24_PA_LOW);
  radio.openWritingPipe(addresses[0]);
}


void loop()
{
  //
  ////  //==============test====================
  //      radio.stopListening();                                    // First, stop listening so we can talk.
  //
  //
  //    Serial.println(F("Now sending"));
  //
  //    unsigned long time = micros();                             // Take the time, and send it.  This will block until complete
  //     if (!radio.write( &time, sizeof(unsigned long) )){
  //       Serial.println(F("failed"));
  //     }
  //
  //    radio.startListening();
  //  //======================================
  static time_t tLast;
  time_t t;

  t = now();
  if (t != tLast) {
    tLast = t;
    mainDisplay(t);
    mesureVolts();
    if (second(t) == 0) {
      readDht();
    }
  }

  if (lcd_time != 0 && millis() - lcd_time > 15 * 1000)
  {
    deActivateLcd();
    lcd_time = 0;
  }

  if (Serial3.available()) {
    processSerial((char)Serial3.read(), 3);
  }

  // read from port 0, send to port 1:
  if (Serial.available()) {
    processSerial((char)Serial.read(), 1);
  }
}

void readDht()
{
  data.IntTemp = bmp.readTemperature();
  data.Humidity = dht.readHumidity();
  data.ExtTemp = dht.readTemperature();
  data.Presure = bmp.readPressure() * 0.01;
  // Read temperature as Celsius (the default)
  // Check if any reads failed and exit early (to try again).
  //  if (isnan(h) || isnan(t) || isnan(f)) {
  //Serial.println("Failed to read from DHT sensor!");
  //return;
  //}
}

void processSerial(char inChar,  int port)
{
  activateLcd();
  // add it to the inputString:
  if (inChar == '\n' || inChar == '\r') {
    processCommand(command, inputString, port);
    command = "";
    inputString = "";
  } else {
    inputString += inChar;

  }
  if (inChar == '=')
  {
    command = inputString;
    inputString = "";
  }
}

void processCommand(String cmd, String param, int port)
{
  debug("Cmd=" + cmd + " param=" + param   );
  //T=YYYY:MM:DD:HH:MM:SS
  if (cmd == "T=")
  {
    tmElements_t tm;
    tm.Hour = getValue(param, ':', 3).toInt();             //set the tm structure to 23h31m30s on 13Feb2009
    tm.Minute = getValue(param, ':', 4).toInt();
    tm.Second = getValue(param, ':', 5).toInt();
    tm.Day = getValue(param, ':', 2).toInt();
    tm.Month = getValue(param, ':', 1).toInt();
    tm.Year = getValue(param, ':', 0).toInt() ;    //tmElements_t.Year is the offset from 1970
    setTime(tm.Hour, tm.Minute, tm.Second, tm.Day, tm.Month, tm.Year);   //set the system time to 23h31m30s on 13Feb2009
    RTC.set(now());                     //set the RTC from the system time  }
    Serial.println("Time set to :");
    Serial.println(now());
  }
  if (cmd == "D=" || param == "D")
  {
    info("DATA at :" + printDate(now()) + ' ' + printTime(now()) );
    info("IntTemp :" + printI00(data.IntTemp, ' ') + " *C");
    info("ExtTemp :" + printI00(data.ExtTemp, ' ') + " *C");
    info("Presure :" + printI00(data.Presure, ' ') + " mB");
    info("Humidity:" + printI00(data.Humidity, ' ') + " %");
    info("==================================");
    info("Volt1   :" + printI00(data.Volt1, ' ') + " V");
    info("Volt2   :" + printI00(data.Volt2, ' ') + " V");
  }
  if (cmd == "?=" || param == "?")
  {
    info("Help :");
    info("Set time:");
    info("T=YYYY:MM:DD:HH:mm:ss");
    info("Get current data: D");
    info("List debug data : debug");
    info("Control relays  : V=XX");
  }
  if (cmd == "V=")
  {
    msg[0] = param.toInt() + 100;
    radio.write(msg, 1);
  }
  if (param == "debug")
  {
    printFileSerial("debug.txt",port);
  }
}

void mesureVolts()
{
  int analog_value = analogRead(A2);
  temp = (analog_value * 5.0) / 1024.0;
  input_voltage = temp / (r2 / (r1 + r2));
  data.Volt1 = analogRead(A3);
  temp = (analog_value * 5.0) / 1024.0;
  data.Volt2 = temp / (r2 / (r1 + r2));

}

void mainDisplay(time_t t)
{

  lcd.setCursor(0, 0);
  lcd.print("Temp =" + printI00(data.ExtTemp, ' ') + "C");
  lcd.setCursor(15, 0);
  lcd.print(getTime(t));
  lcd.setCursor(0, 1);
  lcd.print("Pres =" + printI00(data.Presure , ' ') + "mB " + printI00(data.Humidity , ' ') + "%");
  lcd.setCursor(0, 2);
  char buffer[10];
  String tem = dtostrf(data.Volt1, 4, 1, buffer);
  String tem1 = dtostrf(data.Volt2, 4, 1, buffer);
  lcd.print("Volts=" + tem + "V/" + tem1 + "V");
}

void activateLcd()
{
  lcd_time = millis();
  lcd.backlight();
}

void deActivateLcd()
{
  lcd.noBacklight();
}






