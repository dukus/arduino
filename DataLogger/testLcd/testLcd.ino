//YWROBOT
//Compatible with the Arduino IDE 1.0
//Library version:1.1
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DS3232RTC.h>        //http://github.com/JChristensen/DS3232RTC
#include <Time.h>             //http://playground.arduino.cc/Code/Time
#include <Adafruit_BMP085.h>



LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display
Adafruit_BMP085 bmp;

//----------------------
String time = "";
int internTemp = 0;
float temp=0;
float input_voltage = 0.0;
float baterry_voltage = 0.0;
float r1=6830.0;
float r2=2180.0;
//----------------------
void setup()
{
  lcd.init();                      // initialize the lcd

  setSyncProvider(RTC.get);
//  if (!bmp.begin()) {
//  Serial.println("Could not find a valid BMP085 sensor, check wiring!");
  bmp.begin();

  // Print a message to the LCD.
  lcd.backlight();
  /*
  lcd.setCursor(3, 0);
  lcd.print("Hello, world!");
  lcd.setCursor(2, 1);
  lcd.print("Ywrobot Arduino!");
  lcd.setCursor(0, 2);
  lcd.print("Arduino LCM IIC 2004");
  lcd.setCursor(2, 3);
  lcd.print("Power By Ec-yuan!");
  */
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
}

void mesureVolts()
{
  int analog_value = analogRead(A2);
   temp = (analog_value * 5.0) / 1024.0; 
   input_voltage = temp / (r2/(r1+r2));
   analog_value = analogRead(A3);
   temp = (analog_value * 5.0) / 1024.0; 
   baterry_voltage = temp / (r2/(r1+r2));   
}

void mainDisplay(time_t t)
{

  lcd.setCursor(0, 0);
  lcd.print("Temp ="+ printI00(internTemp,' ')+"C");
  lcd.setCursor(15, 0);
  lcd.print(getTime(t));
  lcd.setCursor(0, 1);
  lcd.print("Pres =" + printI00(bmp.readPressure()*0.00750061561303,' ')+"mmHg");
  lcd.setCursor(0, 2);
  char buffer[10];
  String tem = dtostrf(input_voltage, 4, 1, buffer);
  String tem1 = dtostrf(baterry_voltage, 4, 1, buffer);
  lcd.print("Volts=" +tem+"V/"+tem1+"V");
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

