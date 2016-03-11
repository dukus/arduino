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
unsigned long lcd_time;
//----------------------
void setup()
{
  lcd.init();                      // initialize the lcd

  setSyncProvider(RTC.get);
  //  if (!bmp.begin()) {
  //  Serial.println("Could not find a valid BMP085 sensor, check wiring!");
  bmp.begin();
  // bluetooth
  Serial.begin(9600);
  Serial3.begin(9600);
  inputString.reserve(200);
  // Print a message to the LCD.
  activateLcd();
  dht.begin();

  Serial.print("Initializing SD card...");

  if (!SD.begin(53)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("test.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to test.txt...");
    myFile.println("testing 1, 2, 3.");
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

  // re-open the file for reading:
  myFile = SD.open("test.txt");
  if (myFile) {
    Serial.println("test.txt:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

}


void loop()
{
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
    processSerial((char)Serial3.read());
  }

  // read from port 0, send to port 1:
  if (Serial.available()) {
    processSerial((char)Serial.read());
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

void processSerial(char inChar)
{
  activateLcd();
  // add it to the inputString:
  if (inChar == '\n' || inChar == '\r') {
    processCommand(command, inputString);
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

void processCommand(String cmd, String param )
{
  debug("Cmd=" + cmd + " param="    );
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
    info("DATA at "+getTime( now()));
    info("IntTemp :"+printI00(data.IntTemp, ' ')+" *C");
    info("ExtTemp :"+printI00(data.ExtTemp, ' ')+" *C");
    info("Presure :"+printI00(data.Presure, ' ')+" mB");
    info("Humidity:"+printI00(data.Humidity, ' ')+" %");
    info("==================================");
    info("Volt1   :"+printI00(data.Volt1, ' ')+" V");
    info("Volt2   :"+printI00(data.Volt2, ' ')+" V");
  }
  if (cmd == "?=" || param == "?")
  {
    info("Help :");
    info("Set time:");
    info("T=YYYY:MM:DD:HH:mm:ss");
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



//print date and time to Serial
void printDateTime(time_t t)
{
  lcd.setCursor(0, 2);
  lcd.print(printDate(t) + ' ' + printTime(t));
}

//print time to Serial
String printTime(time_t t)
{
  return printI00(hour(t), ':') + printI00(minute(t), ':') + printI00(second(t), ' ');
}

//print time to Serial
String getTime(time_t t)
{ if (second(t) % 2 == 0) {
    return printI00(hour(t), ':') + printI00(minute(t), ' ');
  } else  {
    return printI00(hour(t), ' ') + printI00(minute(t), ' ');
  }
}


//print date to Serial
String printDate(time_t t)
{
  String res = "";
  res += printI00(day(t), 0);
  return res + monthShortStr(month(t)) + String(year(t));
}

//Print an integer in "00" format (with leading zero),
//followed by a delimiter character to Serial.
//Input value assumed to be between 0 and 99.
String printI00(int val, char delim)
{
  String res = "";
  if (val < 10) res += '0';
  res += String(val);
  if (delim > 0) res += delim;
  return res;
}


