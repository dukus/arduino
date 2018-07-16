#include <Stepper.h>
 
const int stepsPerRevolution = 20; 
 
//Connection pins:
Stepper myStepperX(stepsPerRevolution, 8,9,10,11); 
 
void setup() {
 //Set speed:
 myStepperX.setSpeed(100);
 //max 250 steps for dvd/cd stepper motors 
 myStepperX.step(160);
 delay(100);
 
}
 
void loop() {

}
