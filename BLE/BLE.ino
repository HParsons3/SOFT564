#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <SPI.h>

#define SERVICE_UUID        "310063e3-8f39-4c66-8cd4-5aa149c7a9e7" //Unique UUIDs
#define CHARACTERISTIC_UUID "3b420da2-c831-4cc2-b38d-a0c927d57255"

//static const int spiClk = 1000000; // 1 MHz
volatile boolean complete = false;
volatile boolean timerflag = false;
volatile int count = 0;
volatile char buf[15];
volatile char returned = 'a';
int i;
char* password1 = "password";
boolean locked = true;

SPISettings settings(10000000, MSBFIRST, SPI_MODE0);

hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED; //Set up mutex

class MyCallbacks: public BLECharacteristicCallbacks { //Bluetooth data callback
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue(); //Receive data
      i = 0;
      while (value[i - 1] != '.' && i < 15) { //While not terminated
        buf[i] = value[i]; //Add data to buffer
        i++;
      }
      complete = true; //Completed flag
    }
};

void IRAM_ATTR onTimer() { //Timer interrupt
  portENTER_CRITICAL_ISR(&timerMux); //Mutex lock
  if (count < 3) {
    SPI.begin(18, 19, 23, 05);
    SPI.beginTransaction(settings);
    digitalWrite(05, LOW); //Set low to select the slave
    returned = SPI.transfer(6); //Transfer data to inform that it still connected
    digitalWrite(05, HIGH); //Set high again
    SPI.endTransaction();
    SPI.end();
    timerflag = true;
  }
  else {
    SPI.begin(18, 19, 23, 05);
    SPI.beginTransaction(settings);
    digitalWrite(05, LOW); //Set low to select the slave
    returned = SPI.transfer(7); //Transfer data to inform that it still connected
    digitalWrite(05, HIGH); //Set high again
    SPI.endTransaction();
    SPI.end();
  }
  if (returned) {
    count = 0;
  }
  else {
    count++;
  }
  //  Serial.println(returned)
  //  timerflag = true;
  portEXIT_CRITICAL_ISR(&timerMux); //Release mutex
}

void setup() {
  Serial.begin(115200); //For debugging

  timer = timerBegin(0, 80, true); //Setup timer for microseconds
  timerAttachInterrupt(timer, &onTimer, true); //Timer interrupt
  timerAlarmWrite(timer, 500000, true); //Timer will reset every half second
  timerAlarmEnable(timer);

  BLEDevice::init("Harry's Buggy"); //BLE Name
  BLEServer *pServer = BLEDevice::createServer(); //Initialize server

  BLEService *pService = pServer->createService(SERVICE_UUID); //Start service

  BLECharacteristic *pCharacteristic = pService->createCharacteristic( //Set characteristics
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       ); // Set BLE Characteristic

  pCharacteristic->setCallbacks(new MyCallbacks()); //Set callback

  pService->start(); //Start Service

  BLEAdvertising *pAdvertising = pServer->getAdvertising(); //Setup advertising
  pAdvertising->start(); //Start advertising

  SPI.begin(18, 19, 23, 05); //Set custom pins
  SPI.setClockDivider(SPI_CLOCK_DIV8);  //Set clock divider to 8
  pinMode(05, OUTPUT); //Manually setup the slave select pin

  SPI.beginTransaction(settings);
  digitalWrite(05, LOW); //Set low to select the slave
  SPI.transfer(5); //Transfer data to inform that it is initialised
  digitalWrite(05, HIGH); //Set high again
  SPI.endTransaction();
}

void loop() {
  if (complete == true) { //If data received
    if (locked == true) { //If the ESP is locked
      boolean flag = false; //Initialise a flag
      int c = 0; //Count
      while (password1[c] != '\0') {
        if (buf[c] != password1[c]) { //If a character does not match, raise the flag
          flag = true;
          Serial.println(c);
          break;
        }
        c++;
      }
      if (flag == false) { //If the flag did not get raised
        locked = false; //Unlock the buggy
//        Serial.println("Open");
      }
      else {
//        Serial.println("Incorrect password.");
      }
    }
    else {
      SPI.begin(18, 19, 23, 05);
      SPI.beginTransaction(settings);
      if (buf[i - 1] != '.') { //Check for input termination
        digitalWrite(05, LOW);
        SPI.transfer(3); //Error: Input not terminated
        digitalWrite(05, HIGH);
      }
      else if (i - 1 > 15) { //Check string length
        digitalWrite(05, LOW);
        SPI.transfer(2); //Error: Input exceeds 15
        digitalWrite(05, HIGH);
      }
      else { //If legal data
        digitalWrite(05, LOW);
        for (int j = 0; j < i - 1; j++) {
          SPI.transfer(buf[j]); //Send data byte by byte
        }
        SPI.transfer(1); //Transmission Successful
        digitalWrite(05, HIGH);
      }
      SPI.endTransaction();
      SPI.end();
    }
    complete = false; //Lower completed flag
  }
}
