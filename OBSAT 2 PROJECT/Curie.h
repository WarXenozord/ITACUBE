//General header for types used in multiple modules
//J. Libonatti

#ifndef CURIE_H
#define CURIE_H

// #define HMC_CALIB // Subroutine to provide HMC calibration outputs
// #define OLED_PANEL //Activates OLED Debug
#define SERIAL_DEBUG //Activates Serial Debug
#define SILENCE; //Stops sucess sound

#define TEAM 41


// Calib coef (xmax,xmin,ymax,ymin,zmax,zmin) = (494.51,95.95,242.00,-207.64,303.34,-87.59)
const float MagAvgDelta = (494.51 - 95.95 + 242.00 + 207.64 + 303.34 + 87.59)/6;

const float MagXCalib = (494.51 + 95.95)/2; // (xmax + xmin)/2
const float MagXCalib2 = MagAvgDelta/((494.51 - 95.95)/2);// (xmax + xmin)/2
const float MagYCalib = (242.00 - 207.64)/2; // (ymax + ymin)/2
const float MagYCalib2 = MagAvgDelta/((242.00 + 207.64)/2);
const float MagZCalib = (303.34 - 87.59)/2; // (zmax + zmin)/2
const float MagZCalib2 = MagAvgDelta/((303.34 + 87.59)/2);


const char* serverName = "http://192.168.0.1:8080/"; // Url do Server

struct DataGY87{
  float Acc[3]; // X, Y, Z;
  float Gyro[3]; // X, Y, Z;
  float Mag[3]; // X, Y, Z;
  float Pres;
  float Temp;
};

struct DataGPS{
  bool Valid[2];// Pos, Time
  double Pos[3];// Latitude, Longitude, Altitude;
  uint8_t Time[3];// Second, Minute, Hour
};

struct DataGeiger{
  unsigned int counts; // number of counts per minute
  unsigned long tf; //last measure final Time
};

#endif
