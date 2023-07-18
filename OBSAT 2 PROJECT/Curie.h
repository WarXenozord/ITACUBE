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
#define GY_DELAY 5            //Note: those delays wont change sensor aquisition frequency, 
#define GG_DELAY 10000        //only the frequency at which they will be read
#define GPS_DELAY 100         //i.e. the gps will still operate at 1Hz even if GPS delay is 
#define BAT_DELAY 10000       //set to 100ms, which corresponds to a reading 10Hz frequency.
#define SD_DELAY 10
#define LORA_DELAY 5000
#define WIFI_DELAY 10000

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
//#define SERIAL_DEBUG_SD // Prints SD output
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
};

struct DataGPS{
  bool Valid[2] = {0,0};      // Pos and Time. True if they are valid
  double Pos[3]   = {0,0,0};  // Latitude (deg), Longitude (deg), Altitude (m);
  uint8_t Time[3] = {0,0,0};  // Second, Minute, Hour, universal time stamp (+0 fuse)
};

struct DataGeiger{
  unsigned int counts = 0;    // number of counts starting from last tf to the end of this tf
  unsigned long tf = 0;       //last measure final Time
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


//-----------Gyrometer Calib Coef-----------//


#endif
