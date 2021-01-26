# SOFT564
Buggy control via BLE and TCP

Setup
Initially, wire up 4 wires from each of the ESP32 boards. MISO, MOSI, SCLK and SS should be in pins 19, 23, 18 and 5 respectively. The first 3 of these will pass through a resistor, and then to a level shifter to protect the ESP32 boards from the high voltage output from the Arduino. 4 wires can then be plugged from the Arduino. These will fit in ports 50 (MISO), 51 (MOSI), 52 (SCLK), 53 (SS). The first three can be plugged into the other side of the level shifter. 
As for the SS, the lines from each of the ESPs must be passed through an AND gate together, before being sent to the Arduino. Level shifter is not necessary in this case.

Plug in a wire from the Arduino SCL and SDA lines and connect them to the relevant IMU pins. 

Wire the 3.3v, Gnd and 5v from the ESPs and Arduino to the level shifter and the IMU where applicable. 

Finally, wire an LED with resistor from pin 7 of the Arduino to the breadboard. 

Application

Upload Arduino.ino to the Arduino, and BLE.ino and TCP.ino each to separate ESP boards. Modify TCP.ino to include your wifi ID and password in the "ssid" and "password" variables.

Modify the ESP files with your custom password in the "password1" variable if you wish.

Plug in the ESP boards, and the Arduino.

Download a BLE app. Search for "Harry's Buggy". Connect to it, and you may then write commands, as will be discussed below. In order to control the buggy, the password must be entered first.

Open CMD, navigate to your Python directory and run TCP.py via Python. It may be necessary to modify the file with a separate IP address. To do this, wire the TCP controlling ESP into a laptop and open a serial monitor. The ESP will post the IP required to connect to it.

Once again, to control the robot using TCP, a password must be entered first.

Control Commands:

A string of numbers can be entered which will be converted into an integer and stored as the number of seconds to move the buggy for. Then, the letters f, b, r, l, can be placed afterwards in order to mean Forward, Backward, Right and Left respectively. 

Commands can be placed consecutively with no spaces, for example 1f2r3b. 

A command must be less than 15 characters, and must be terminated with a period or full stop '.'

TCP ONLY: Using the exclamation mark '!', you can read data from the Arduino. This will store the data, which will be printed the next time you send a command. This command could be a movement command or another read command.
