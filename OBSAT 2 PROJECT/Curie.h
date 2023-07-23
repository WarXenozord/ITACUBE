//General header for types used in multiple modules
//J. Libonatti

#ifndef CURIE_H
#define CURIE_H

//------------------//
//--- PARAMETERS ---//
//------------------//

//Team ID provided by OBSAT
#define TEAM 41

//Delay for task functions in RTOS ticks (1 tick = 1ms if normal RTOS)
#define GY_DELAY 0            //Note: those delays wont change sensor aquisition frequency, 
#define GG_DELAY 10000        //only the frequency at which they will be read
#define GPS_DELAY 500         //i.e. the gps will still operate at 1Hz even if GPS delay is 
#define BAT_DELAY 10000       //set to 100ms, which corresponds to a reading 10Hz frequency.
#define SD_DELAY 10
#define LORA_DELAY 5000
#define WIFI_DELAY 10000

//Files
#define GY87_FILE 0
#define GPS_FILE 1
#define GEIGER_FILE 2
#define BATTERY_FILE 3
#define LOG_FILE 4

//Watchdog Timeout in seconds
#define WDT_TIMEOUT 15

//Wifi Parameters
const char* wifiSSID = "OBSAT_WIFI";                  //Wifi's "name"
const char* wifiPassword = "OBSatZenith1000";         //Wifi's password
const char* serverName = "http://192.168.0.1:8080/";  // Server url = "http://ip:port/"

//----------------//
//--- SWITCHES ---//
//----------------//

//Debug
#define SERIAL_DEBUG      //Activates Serial Debug
//#define SERIAL_DEBUG_SD   // Prints SD output
#define UNSAFE_BOOT       //Runs main code even if errors are detected

//Comms & data
#define ENABLE_LORA       //Enables LoRa Comms
#define ENABLE_WIFI       //Enables Wifi Comms
#define ENABLE_SD         //Enables SD Comms

//Sensors
#define ENABLE_GY         //Enables GY87
#define ENABLE_GPS        //Enables GPS
#define ENABLE_GG         //Enables Geiger

//Miscellaneous
#define SILENCE;          //Stops sucess sound


//-------------------//
//--- DATA TYPES ----//
//-------------------//

// Data for GY87 multisensor is stored in this format
struct DataGY87{
  float Acc[3]  = {0,0,0};    // X, Y, Z; aceleration in g (1g = 9.8 m/s)
  float Gyro[3] = {0,0,0};    // X, Y, Z; spin over the axis in radians per second
  float Mag[3]  = {0,0,0};    // X, Y, Z; magnetic field in milliGauss
  float Pres = 0;             // Pressure in Pascal
  float Temp = 0;             // Temperature in Celsius
  unsigned long tf = 0;       // Final time of measurement for barometer. tf-31 for acc gyro and mag
};

struct DataGPS{
  bool Valid[2] = {0,0};      // Pos and Time. True if they are valid
  double Pos[3]   = {0,0,0};  // Latitude (deg), Longitude (deg), Altitude (m);
  uint8_t Time[3] = {0,0,0};  // Second, Minute, Hour, universal time stamp (+0 fuse)
  unsigned long tf = 0;       // Final time of measurement
};

struct DataGeiger{
  unsigned int counts = 0;    // number of counts starting from last tf to the end of this tf
  unsigned long tf = 0;       // Final time of measurement
};

//-------------------//
//--- CALIBRATION ---//
//-------------------//

//GY-87 requires calibration in order to give nice outputs. 
//The magnetometer needs to perform a perfect sphere around (0,0,0) as you rotate it because it always detects earth's magnetic field
//The Gyrometer needs to be 0 when the gy is still
//The Accelerometer needs to display (1,0,0) when the x positive axis is aligned with the ground. Same with other axis.

//----------Magnetometer Calib Coef----------//

// Calib coef (xmax,xmin,ymax,ymin,zmax,zmin) = (494.51,95.95,242.00,-207.64,303.34,-87.59)

const float MagAvgDelta = (494.51 - 95.95 + 242.00 + 207.64 + 303.34 + 87.59)/6;

const float MagXCalib = (494.51 + 95.95)/2; // (xmax + xmin)/2
const float MagXCalib2 = MagAvgDelta/((494.51 - 95.95)/2);// (xmax + xmin)/2
const float MagYCalib = (242.00 - 207.64)/2; // (ymax + ymin)/2
const float MagYCalib2 = MagAvgDelta/((242.00 + 207.64)/2);
const float MagZCalib = (303.34 - 87.59)/2; // (zmax + zmin)/2
const float MagZCalib2 = MagAvgDelta/((303.34 + 87.59)/2);

//---------Accelerometer Calib Coef---------//

// Accelerometer offset:
const int16_t AX_OFFSET = 0.08 * 16384.0 / 4;       // Use these values to calibrate the accelerometer. The sensor should output 1.0g if held level. 
const int16_t AY_OFFSET = 0.02 * 16384.0 / 4;       // These values are unlikely to be zero.
const int16_t Az_OFFSET = 0.03 * 16384.0 / 4;

// Output scale: 
const float AX_SCALE = 1.00;     // Multiplier for accelerometer outputs. Use this to calibrate the sensor. If unknown set to 1.
const float AY_SCALE = 1.00;
const float AZ_SCALE = 1.00;

//-----------Gyrometer Calib Coef-----------//
// Accelerometer offset:
const int16_t GX_OFFSET = -11.4 * 131.0 / 4;       // Use these values to calibrate the accelerometer. The sensor should output 1.0g if held level. 
const int16_t GY_OFFSET = 5.45 * 131.0 / 4;       // These values are unlikely to be zero.
const int16_t GZ_OFFSET = -1.4 * 131.0 / 4;

// Output scale: 
const float GX_SCALE = 1.0;     // Multiplier for accelerometer outputs. Use this to calibrate the sensor. If unknown set to 1.
const float GY_SCALE = 1.0;
const float GZ_SCALE = 1.0;

#endif
