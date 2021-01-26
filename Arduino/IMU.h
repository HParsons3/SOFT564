#ifndef IMU_h
#define IMU_h

#include "Arduino.h"

class IMU
{
  public:
    IMU(int MPU_ADDR); //Constructor
    int address(); //Address of the IMU
    void IMUread(); //Read function
    void IMUprint(); //Print function
    int16_t accelerometer_x, accelerometer_y, accelerometer_z; // variables for accelerometer raw data
    int16_t gyro_x, gyro_y, gyro_z; // variables for gyro raw data
    int16_t temperature; // variables for temperature data
  private:
    char* convert_int16_to_str(int16_t i);
    char tmp_str[7]; // temporary variable used in convert function
    int _address;
};

#endif
