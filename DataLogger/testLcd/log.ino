void debug(String string)
{
  time_t t = now();
  string = printDate(t) + ' ' + printTime(t) + "[Debug] " + string;
  //Serial.println(string);
  myFile = SD.open("debug.txt", FILE_WRITE);
  // if the file opened okay, write to it:
  if (myFile) {
    time_t t = now();
    myFile.println(string);
    myFile.close();
  } else {

  }

}

void error(String string)
{
  time_t t = now();
  string = printDate(t) + ' ' + printTime(t) + "[Error] " + string;
  // Serial.println(string);
  myFile = SD.open("debug.txt", FILE_WRITE);
  // if the file opened okay, write to it:
  if (myFile) {
    myFile.println(string);
    myFile.close();
  } else {

  }

}

void info(String string)
{
  Serial3.println(string);
}
