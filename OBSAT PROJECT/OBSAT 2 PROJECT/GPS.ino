//This module formats handles the communication with the GPS
//J. Libonatti

#include <TinyGPSPlus.h>

#define RXD1 12
#define TXD1 13

#define GPS_Serial_Baud 9600

const uint8_t ubx_cfg_nav5[] = { 0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x08, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 
                                   0x00, 0x00, 0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x20 };

const uint8_t ubx_cfg_conf[] = { 0xB5, 0x62, 0x05, 0x01, 0x02, 0x00, 0x06, 0x24, 0x32, 0x5B};

TinyGPSPlus gps;

int SetGPS() {
  Serial1.begin(GPS_Serial_Baud, SERIAL_8N1, RXD1, TXD1);
  
  Serial1.println("$PUBX,40,GLL,0,0,0,0,0,0*5C"); //shuts up gps for configs
  Serial1.println("$PUBX,40,GSA,0,0,0,0,0,0*4E");
  Serial1.println("$PUBX,40,ZDA,0,0,0,0,0,0*44");
  Serial1.println("$PUBX,40,GGA,0,0,0,0,0,0*5A");
  Serial1.println("$PUBX,40,RMC,0,0,0,0,0,0*47");
  Serial1.println("$PUBX,40,GSV,0,0,0,0,0,0*59");
  Serial1.println("$PUBX,40,VTG,0,0,0,0,0,0*5E");
  Serial1.flush();
  
  while(Serial1.available()){ //clear received buffer
    Serial1.read();
  }
  
  Serial1.write(ubx_cfg_nav5, sizeof(ubx_cfg_nav5)); //airborne config
  Serial1.flush();
  while(!Serial1.available()){}
  while(Serial1.available())     // Reads confirmation code
  {
    for(int i=0; i<sizeof(ubx_cfg_conf); i++)
    {
      uint8_t a = Serial1.read();
      Serial.println(a);
      if(a ==  ubx_cfg_conf[i])
      {
        if(i==sizeof(ubx_cfg_conf) - 1)
          break;
      }else{
        return -1;
      }
    } 
  }

  Serial1.println("$PUBX,40,GLL,0,1,0,0,0,0*5D");  // let's GPS talk again
  Serial1.println("$PUBX,40,GSA,0,1,0,0,0,0*4F");
  Serial1.println("$PUBX,40,ZDA,0,1,0,0,0,0*45");
  Serial1.println("$PUBX,40,GGA,0,1,0,0,0,0*5B");
  Serial1.println("$PUBX,40,RMC,0,1,0,0,0,0*48");
  Serial1.println("$PUBX,40,GSV,0,1,0,0,0,0*5A");
  Serial1.println("$PUBX,40,VTG,0,1,0,0,0,0*5F");
  Serial1.flush();
  return 0;
}

void ReadGPS(DataGPS *GPS) {
    while(Serial1.available() > 0)
      if (gps.encode(Serial1.read()));
      {
        GPS->Valid[0] = gps.location.isValid() && gps.altitude.isValid();
        if (GPS->Valid[0])
        {
          GPS->Pos[0] = gps.location.lat();
          GPS->Pos[1] = gps.location.lng();
          GPS->Pos[2] = gps.altitude.meters();
        }
        GPS->Valid[1] = gps.time.isValid();
        if (GPS->Valid[1])
        {
          GPS->Time[0] = gps.time.second();
          GPS->Time[1] = gps.time.minute();
          GPS->Time[2] = gps.time.hour();
        }
      }
}
