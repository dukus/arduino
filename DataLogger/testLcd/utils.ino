
// http://stackoverflow.com/questions/9072320/split-string-into-string-array
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
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

void printFileSerial(String file, int port)
{
  HardwareSerial *serial = GetSerial(port);
  myFile = SD.open(file);
  if (myFile) {
    serial->println(file);
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      serial->write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    serial->println("error opening " + file);
  }
}

HardwareSerial* GetSerial(int port )
{
  if (port == 3)
    return &Serial3;
  else
    return &Serial;
}

void printState(char Relay[7], int port )
{
  HardwareSerial *serial = GetSerial(port);
  serial->print("Enabled ");
  serial->println((byte)Relay[0] );
  serial->print("Start Days " );
  serial->println((byte)Relay[1] );
  serial->print("Start Hour " );
  serial->print((byte)Relay[2] );
  serial->print(":" );
  serial->println((byte)Relay[3] );
  serial->print("Run time M ");
  serial->println((byte)Relay[4] );
}

void out(String string, int port )
{
  HardwareSerial *serial = GetSerial(port);
  serial->println(string);
}

