#include "Arduino.h"
#include "Motor.h"

motor::motor() {
  pinMode(BRAKEA, OUTPUT);      //Brake Pins Initialize
  pinMode(BRAKEB, OUTPUT);     
  pinMode(DIRA, OUTPUT);     //Direction Pins Initialize
  pinMode(DIRB, OUTPUT);  
  pinMode(PWMA, OUTPUT);     //PWM Pins Initialize
  pinMode(PWMB, OUTPUT);
  digitalWrite(BRAKEA, HIGH); //Brakes should be on to start
  digitalWrite(BRAKEB, HIGH);
  analogWrite(PWMA, 255); //Set motor speed
  analogWrite(PWMB, 255);
}

void motor::forward(int t) {
  digitalWrite(DIRA, LOW); //Direction
  digitalWrite(DIRB, HIGH);
  digitalWrite(BRAKEA, LOW); //Turn off brakes
  digitalWrite(BRAKEB, LOW);
  TCNT5 = 0; //Reset count
  timercount = 0;
  while (timercount <= t) {
    TCNT4 = 0; //Reset watchdog timer
  }
  timercount = 0;
  digitalWrite(BRAKEA, HIGH); //Turn on brakes
  digitalWrite(BRAKEB, HIGH);
}

void motor::backward(int t) {
  digitalWrite(DIRA, HIGH); //Direction
  digitalWrite(DIRB, LOW);
  digitalWrite(BRAKEA, LOW); //Turn off brakes
  digitalWrite(BRAKEB, LOW);
  TCNT5 = 0; //Reset count
  timercount = 0;
  while (timercount <= t) {
    TCNT4 = 0; //Reset watchdog timer
  }
  timercount = 0;
  digitalWrite(BRAKEA, HIGH); //Turn on brakes
  digitalWrite(BRAKEB, HIGH);
}

void motor::right(int t) {
  digitalWrite(DIRA, HIGH); //Direction
  digitalWrite(DIRB, HIGH);
  digitalWrite(BRAKEA, LOW); //Turn off brakes
  digitalWrite(BRAKEB, LOW);
  TCNT5 = 0; //Reset count
  timercount = 0;
  while (timercount <= t) {
    TCNT4 = 0; //Reset watchdog timer
  }
  timercount = 0;
  digitalWrite(BRAKEA, HIGH); //Turn on brakes
  digitalWrite(BRAKEB, HIGH);
}

void motor::left(int t) {
  digitalWrite(DIRA, LOW); //Direction
  digitalWrite(DIRB, LOW);
  digitalWrite(BRAKEA, LOW); //Turn off brakes
  digitalWrite(BRAKEB, LOW);
  TCNT5 = 0; //Reset count
  timercount = 0;
  while (timercount <= t) {
    TCNT4 = 0; //Reset watchdog timer
  }
  timercount = 0;
  digitalWrite(BRAKEA, HIGH); //Turn on brakes
  digitalWrite(BRAKEB, HIGH);
}
