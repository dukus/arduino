
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



LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display
Adafruit_BMP085 bmp;
File myFile;
//----------------------
String time = "";
int internTemp = 0;
float temp = 0;
float input_voltage = 0.0;
float baterry_voltage = 0.0;
float r1 = 6830.0;
float r2 = 2180.0;
String inputString = "";
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
      internTemp = RTC.temperature() / 4.;
    }
  }

  if (lcd_time!=0 && millis() - lcd_time > 5 * 1000)
  {
    deActivateLcd();
    lcd_time = 0;
  }

  if (Serial3.available()) {
    activateLcd();
    int inByte = Serial3.read();
    Serial.write(inByte);
  }

  // read from port 0, send to port 1:
  if (Serial.available()) {
    int inByte = Serial.read();
    Serial3.write(inByte);
  }
}

void mesureVolts()
{
  int analog_value = analogRead(A2);
  temp = (analog_value * 5.0) / 1024.0;
  input_voltage = temp / (r2 / (r1 + r2));
  analog_value = analogRead(A3);
  temp = (analog_value * 5.0) / 1024.0;
  baterry_voltage = temp / (r2 / (r1 + r2));
}

void mainDisplay(time_t t)
{

  lcd.setCursor(0, 0);
  lcd.print("Temp =" + printI00(internTemp, ' ') + "C");
  lcd.setCursor(15, 0);
  lcd.print(getTime(t));
  lcd.setCursor(0, 1);
  lcd.print("Pres =" + printI00(bmp.readPressure() * 0.00750061561303, ' ') + "mmHg");
  lcd.setCursor(0, 2);
  char buffer[10];
  String tem = dtostrf(input_voltage, 4, 1, buffer);
  String tem1 = dtostrf(baterry_voltage, 4, 1, buffer);
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


