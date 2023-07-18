//This module handles the communication with the GPS
//J. Libonatti

#include "Curie.h"
#include <TinyGPSPlus.h>     // Library for decode NMEA 

#define RXD1 12              // RX for serial transmission
#define TXD1 13              // TX for serial transmission

#define GPS_Serial_Baud 9600 // Defines GPS Serial Rate. Default for gps 9600

//Configuration packed for airborne mode
const uint8_t ubx_cfg_nav5[] = { 0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x08, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27,    
                                 0x00, 0x00, 0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x20 };

//Configuration confirmation code sent by gps upon sucess
const uint8_t ubx_cfg_conf[] = { 0xB5, 0x62, 0x05, 0x01, 0x02, 0x00, 0x06, 0x24, 0x32, 0x5B };

TinyGPSPlus gps;              //GPS Class from TinyGPSPlus

int SetGPS() {                                              //Starts the GPS
  Serial1.begin(GPS_Serial_Baud, SERIAL_8N1, RXD1, TXD1);   //Begins Serial at 9600, type 8N1 and RX & TX pins
  
  Serial1.println("$PUBX,40,GLL,0,0,0,0,0,0*5C");           //shuts up all gps NMEA Sentence in order to 
  Serial1.println("$PUBX,40,GSA,0,0,0,0,0,0*4E");           //config and get the confirmation code after.
  Serial1.println("$PUBX,40,ZDA,0,0,0,0,0,0*44");
  Serial1.println("$PUBX,40,GGA,0,0,0,0,0,0*5A");
  Serial1.println("$PUBX,40,RMC,0,0,0,0,0,0*47");
  Serial1.println("$PUBX,40,GSV,0,0,0,0,0,0*59");
  Serial1.println("$PUBX,40,VTG,0,0,0,0,0,0*5E");
  Serial1.flush();                                          //flush() assures that data has been sent before proceeding
  
  while(Serial1.available()){ //clear received buffer       //Clear any data in buffer received before flush has been done
    Serial1.read();
  }
  
  Serial1.write(ubx_cfg_nav5, sizeof(ubx_cfg_nav5));        //Sets GPS to airborne mode (allows >80km and up to 4g accel)
  Serial1.flush();                                          //Again, flush to assure has been sent before checking for confirmation code 
                                
  for(int i=0; i<sizeof(ubx_cfg_conf); i++)                 //Reads the confirmation code
  {
    while(!Serial1.available()){}                           //Waits for Serial input
    if(Serial1.read() !=  ubx_cfg_conf[i])                  //Checks if the read character equals to it's corresponding character in confirmation msg
      return -41;                                           //If any character is not equal, return error
  } 

  Serial1.println("$PUBX,40,GLL,0,1,0,0,0,0*5D");           // let's the GPS talk again
  Serial1.println("$PUBX,40,GSA,0,1,0,0,0,0*4F");
  Serial1.println("$PUBX,40,ZDA,0,1,0,0,0,0*45");
  Serial1.println("$PUBX,40,GGA,0,1,0,0,0,0*5B");
  Serial1.println("$PUBX,40,RMC,0,1,0,0,0,0*48");
  Serial1.println("$PUBX,40,GSV,0,1,0,0,0,0*5A");
  Serial1.println("$PUBX,40,VTG,0,1,0,0,0,0*5F");
  
  Serial1.flush();                                          //Assure serial data has been sent before returning 0 (sucess)
  return 0;                                                 
}

bool ReadGPS(DataGPS *GPS) {                                //Reads Data from GPS into DataGps Struct
  if(!Serial1.available())                                  //if there is no data in serial buffer to read, then return false
      return false;                                         
      
  while(Serial1.available() > 0)                            //while there is data in the serial
    if (gps.encode(Serial1.read())){                        //encodes gps data char by char. gps.encode() returns true when a gps fix is found
      GPS->Valid[0] = gps.location.isValid() && gps.altitude.isValid();  //assure we only use gps location when it is valid
      if (GPS->Valid[0]){                                   //if GPS Pos is valid
        GPS->Pos[0] = gps.location.lat();                   //atribution of GPS latitude, measured in degrees
        GPS->Pos[1] = gps.location.lng();                   //atribution of GPS longitude, measured in degrees
        GPS->Pos[2] = gps.altitude.meters();                //atribution of GPS altitude, measured in meters
      }
      GPS->Valid[1] = gps.time.isValid();                   //self-explaining                   
      if (GPS->Valid[1]){                                   //if GPS Time is valid
        GPS->Time[0] = gps.time.second();                   //atribution of GPS UTC time seconds
        GPS->Time[1] = gps.time.minute();                   //atribution of GPS UTC time minutes
        GPS->Time[2] = gps.time.hour();                     //atribution of GPS UTC time hours
      }
    }
  return true;                                              
}
