#include "Motor.h"
#include "IMU.h"
#include <SPI.h>
#include <Wire.h>

#define SCLK 52 //Yellow
#define MISO 50 //White
#define MOSI 51 //Blue
#define SS 53 //Black
#define FORWARD 102
#define BACKWARD 98
#define LEFT 108
#define RIGHT 114

motor Motor;
IMU IMU1(0x68);

volatile char buf[20];
volatile boolean received = false;
volatile boolean errorflag = false;
volatile boolean resetflag = false;
volatile boolean checkflag1 = false;
volatile boolean checkflag2 = false;
volatile int pos = 0;
uint16_t xaccelsum;
uint16_t xaccel;
uint16_t count = 1;
float temp;
int debug;
char dataout[20];

void(* resetFunc) (void) = 0; //To reset the Arduino

void setup (void)
{
  Serial.begin (9600); //For debugging
  noInterrupts();           // Disable all interrupts
  pinMode(7, OUTPUT); //Warning LED
  digitalWrite(7, HIGH);
  pinMode(MISO, OUTPUT); //Manually set MISO

  TCCR5A = 0; //Reset registers
  TCCR5B = 0;
  OCR5A = 62500; //Target value for 16000000/256/1
  TCCR5B |= (1 << WGM12); //Clear timer on compare match
  TCCR5B |= (1 << CS12); //Set 256 prescaler

  TCCR4A = 0; //Reset registers
  TCCR4B = 0;
  TCNT4 = 0; //Reset count
  OCR4A = 62500; //Target value for 16000000/256/1
  TCCR4B |= (1 << WGM12); //Clear timer on compare match
  TCCR4B |= (1 << CS12); //Set 256 prescaler
  TIMSK4 |= (1 << OCIE1A); //Enable interrupt

  SPCR |= _BV(SPE);  //Enable SPI Slave

  SPCR |= _BV(SPIE); //Enable Interrupts

  interrupts(); //Enable interrupts

  SPI.attachInterrupt(); //Attach interrupt

  Wire.begin();
  Wire.beginTransmission(IMU1.address()); // Begins a transmission to the I2C slave (GY-521 board)
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0); // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);

  Serial.println("Initialization complete");

  digitalWrite(7, LOW);

  IMU1.IMUread();
  temp = (IMU1.temperature / 340.00 + 36.53);
  num2ascii(temp);
  Serial.println(temp);

}  // end of setup

void num2ascii(float input) {
  char outputreverse[20]; //Initialise values
  int temporary = input * 100; //int is easiest to work with, but we want two decimal places
  int x = 0;                   //So input is multiplied by ten first
  while (x < 2) { //Decimal places
    outputreverse[x] = (temporary % 10) + 48; //Get remainder when the input is divided by 10
    temporary = temporary / 10; //Move to next digit
    x++;
  }
  outputreverse[x] = '.'; //Decimal point
  x++;
  while (temporary > 0) { //The rest of the values
    outputreverse[x] = (temporary % 10) + 48;
    temporary = temporary / 10;
    x++;
  }
  for (int y = 0; y < x; y++) {
    dataout[y] = outputreverse[x - 1 - y]; //Reverse the string so it is in correct order
  }
}

ISR(TIMER4_COMPA_vect)          // Watchdog timer ISR
{
  if (checkflag1 == false || checkflag2 == false) { //If either flags are untriggered
    resetflag = true;
  }
  checkflag1 = false; //Reset flags
  checkflag2 = false;

}

ISR(TIMER5_COMPA_vect)          // Motor timer ISR
{

  Motor.timercount++; //Increment timer counter

}

ISR (SPI_STC_vect) //SPI received ISR
{
  byte c = SPDR; //Read incoming data
  if (pos < sizeof buf) //If there is room
  {
    buf [pos] = c; //Add current data to the buffer
    SPDR = dataout[pos];
    pos++;
    if ((buf[pos] > 6 && buf[pos] < 46) || (buf[pos] > 57 && buf[pos] != 102 && buf[pos] != 108 && buf[pos] != 114 && buf[pos] != 98 && buf[pos] != 33) || (buf[pos] == 47)) {
      errorflag = true; //Checks against all invalid characters, raises flag if any are detected
    }
  }  // end of room available
  received = true; //Raises flag
}

void loop (void)
{
  if (resetflag == true) {
    resetFunc();
  }
  if (received) { //If data has been received
    //    Serial.println(buf[0]);
    int i = 0; //Count
    long unsigned int t = 0; //Time
    int j = 0; //Count
    while (buf[i] != '.') { //While input is not terminated
      if (errorflag == true) { //Flag raised if invalid character is found
//        Serial.println("Error: Invalid character detected");
        break;
      }
      else if (buf[i] == 5) { //Sent when ESP32 is initialized
//        Serial.println("ESP32 Initialized! BLE Connection ready!");
        break; //Exit while loop
      }
      else if (buf[i] == 4) { //Checkup ping
        checkflag1 = true;
//        Serial.println("TCP Found");
        break; //Exit while loop
      }
      else if (buf[i] == 6) {
        checkflag2 = true;
//        Serial.println("BLE Found");
        break;
      }
      else if (buf[i] == 2) { //Sent when the input is too long
//        Serial.println("Error: Input exceeds 15 characters");
        break; //Exit while loop
      }
      else if (buf[i] == 3) { //Sent when the input is not terminated with a .
//        Serial.println("Error: Input not terminated");
        break; //Exit while loop
      }
      else if (buf[i] == 1) { //Sent when the transmission finishes successfully
        //        Serial.println("Transmission Successful!");
        break; //Exit while loop
      }
      else if (buf[i] >= 48 && buf[i] <= 57) { //If the input is a number
        if (++j > 4) { //Cannot have numbers be too large
          printf("Error: Numbers cannot be larger than 99999");
          break;
        }
        if (t < 0) {
          t = 0;
        }
        t = (t * 10) + (buf[i] - 48);
      }
      else if (buf[i] == FORWARD) {
        TIMSK5 |= (1 << OCIE1A); //Enable timer interrupt
        Motor.forward(t); //Move forwards
        TIMSK5 &= ~(1 << OCIE1A); //Disable timer interrupt
        t = 0; //reset counts
        j = 0;
      }
      else if (buf[i] == BACKWARD) {
        TIMSK5 |= (1 << OCIE1A); //Enable timer interrupt
        Motor.backward(t); //Move backwards
        TIMSK5 &= ~(1 << OCIE1A); //Disable timer interrupt
        t = 0; //Reset counts
        j = 0;
      }
      else if (buf[i] == LEFT) {
        TIMSK5 |= (1 << OCIE1A); //Enable timer interrupt
        Motor.left(t); //Turn left
        TIMSK5 &= ~(1 << OCIE1A); //Disable timer interrupt
        t = 0; //Reset counts
        j = 0;
      }
      else if (buf[i] == RIGHT) {
        TIMSK5 |= (1 << OCIE1A); //Enable timer interrupt
        Motor.right(t); //Turn right
        TIMSK5 &= ~(1 << OCIE1A); //Disable timer interrupt
        t = 0; //Reset counts
        j = 0;
      }
      //      else {
      //
      //      }
      i++;
    }
    digitalWrite(BRAKEA, HIGH); //Apply brakes
    digitalWrite(BRAKEB, HIGH);
    received = false; //Ready for next transmission
    buf[pos] = 0;
    pos = 0;
  }
  IMU1.IMUread(); //Read IMU data
  temp = (IMU1.temperature/340.00+36.53); //Compute Temperature 
  num2ascii(temp); //Convert to string
}  // end of loop
