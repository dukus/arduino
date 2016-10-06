#include <AccelStepper.h>

#include <Bounce2.h>

//----------------
const int displayLine = 4;
//----------------
#define motorPin1 12
#define motorPin2 11
#define motorPin3 10
#define motorPin4 9
//----------------
String inputString = "";  
const int buttonPin1 = 8;
Bounce debouncer = Bounce(); 

AccelStepper stepper1(8, motorPin1, motorPin3, motorPin2, motorPin4);

//----------------
// 0 - not started
// 1 - running
// 2 - left end 
int deviceState = 0;
//----------------
void setup()
{
  Serial.begin(9600);
  Serial.println("Device started");
  debouncer.attach(buttonPin1);
  debouncer.interval(5); // interval in ms

}

void loop() 
{
  debouncer.update();
  if(debouncer.read() == 1 && deviceState==1)
  {
    deviceState=2;
    Stop();
  }

   stepper1.run();
}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read(); 
    // add it to the inputString:
    inputString += inChar;
    if (inputString="#S#")
    {
      inputString ="";
      Start();
    }
  }
}

void Stop()
{
  stepper1.stop();
  DisplayData();
}

void Start()
{
  stepper1.enableOutputs(); 
  stepper1.setCurrentPosition(0);
  stepper1.setMaxSpeed(300);
  stepper1.setAcceleration(70);
  stepper1.moveTo(20000);
  deviceState = 1;
  DisplayData();
}

void DisplayData()
{
  for (int i = 0; i < displayLine; i++)
  {
    Serial.println();
  }
  if(deviceState==1) Serial.println("Device is running!");
  if(deviceState==2) Serial.println("Run to left end");
}








