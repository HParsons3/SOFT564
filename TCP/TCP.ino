#include <WiFi.h>
#include <SPI.h>

volatile char returned;
const char* ssid = "Parsonet"; //WiFi name
const char* password =  "charlierocks"; //WiFi password
char buf[20];
boolean complete = false;
volatile int count = 0;
int i = 0;
volatile boolean timerflag = false;
char* password1 = "password";
//char* password2 = "password";
boolean locked = true;
boolean dataflag = false;
char datain[20];
char* debug = "abcdeghi";

hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED; //Set up mutex

WiFiServer wifiServer(80); //Port

SPISettings settings(5000000, MSBFIRST, SPI_MODE0);

void IRAM_ATTR onTimer() { //Timer interrupt
  portENTER_CRITICAL_ISR(&timerMux); //Mutex lock
  if (count < 3) {
    SPI.begin(18, 19, 23, 05);
    SPI.beginTransaction(settings);
    digitalWrite(05, LOW); //Set low to select the slave
    returned = SPI.transfer(4); //Transfer data to inform that it still connected
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
    timerflag = true;
  }
  if (returned != 00000000) {
    count = 0;
  }
  else {
    count++;
  }
  portEXIT_CRITICAL_ISR(&timerMux); //Release mutex
}

void setup() {

  Serial.begin(115200); //For debugging

  timer = timerBegin(0, 80, true); //Setup timer for microseconds
  timerAttachInterrupt(timer, &onTimer, true); //Timer interrupt
  timerAlarmWrite(timer, 500000, true); //Timer will reset every half second
  timerAlarmEnable(timer);

  WiFi.begin(ssid, password); //Connect to WiFi

  while (WiFi.status() != WL_CONNECTED) { //When not connected...
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println("Connected to the WiFi network");
  Serial.println(WiFi.localIP()); //Print IP address

  wifiServer.begin();

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
  WiFiClient client = wifiServer.available(); //Check if a client is connected
  if (client) { //If a client is connected...
    i = 0;

    while (client.connected()) {
      while (client.available() > 0) {
        if (dataflag == false) { //If data has not been read
          buf[i] = client.read(); //Read the data
          client.write("              "); //Send blank return
          i++;
        }
        else { //If data has been read
          buf[i] = client.read(); //read data from TCP client
          client.write(datain); //Send data to TCP client
          client.write("              "); 
          i++;
          dataflag = false;
        }
      }
      delay(10);
    }
    complete = true; //Raise completed flag
    Serial.println(buf);
    client.stop();
  }
  if (complete == true) { //If data has been received
    if (locked == true) { //If the ESP is locked
      boolean passwordflag = false; //Initialise a flag
      int c = 0;
      while (password1[c] != '\0') {
        if (buf[c] != password1[c]) { //If the characters do not match
          passwordflag = true; //Raise the flag
 //         Serial.println(c);
          break; //Break from the loop
        }
        c++; 
      }
      if (passwordflag == false) { //If the flag was not raised
        locked = false; //Unlock the ESP
//        Serial.println("Open");
      }
      else { //Otherwise, leave locked
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
      else if (buf[i - 2] == '!') {
        Serial.println("Reading data");
        for (int j = 0; j < 8; j++) {
          digitalWrite(05, LOW); //Set low to select the slave
          returned = SPI.transfer('x'); //Transfer data to inform that it still connected
          digitalWrite(05, HIGH); //Set high again
          datain[j] = returned;
          Serial.print(datain[j]);
        }
        Serial.println();
        dataflag = true;
      }
      else { //If legal data
        portENTER_CRITICAL_ISR(&timerMux); //Mutex lock
        for (int j = 0; j < i - 1; j++) {
          digitalWrite(05, LOW);
          SPI.transfer(buf[j]); //Send data byte by byte
          digitalWrite(05, HIGH);
        }
        Serial.println(datain);
        digitalWrite(05, LOW);
        SPI.transfer(1); //Transmission Successful
        digitalWrite(05, HIGH);
        //        digitalWrite(05, HIGH);
        portEXIT_CRITICAL_ISR(&timerMux); //Release mutex
      }
      SPI.endTransaction();
      SPI.end();
    }
    complete = false; //Lower completed flag
    for(int x = 0; x < sizeof buf; x++) { //Reset Buffer
        buf[x] = ' ';
    }
  }
}
