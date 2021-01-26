#ifndef motor_h
#define motor_h

#include "Arduino.h"

#define BRAKEA 8 //Define motor control pins
#define BRAKEB 9
#define DIRA 12
#define DIRB 13
#define PWMA 3
#define PWMB 11

class motor {
  public:
    motor(); //Constructor
    void forward(int t); //Movement functions
    void backward(int t);
    void right(int t);
    void left(int t);
    volatile int timercount; //Timer count
  private:
};

#endif
