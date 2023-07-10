//Main Execution Sequence
//J. Libonatti

#include "Curie.h"

String Query;
struct DataGY87 GY87;
struct DataGPS GPS;
struct DataGeiger Geiger;

void setup() {
  //Start Debug
  #ifdef SERIAL_DEBUG
    Serial.begin(19200);
  #endif
  //Setup IO
  SetLED();
  SetBuzzer();
  //Start Sensors
  int errCode[4];
  errCode[0] = SetGY87();
  errCode[1] = SetGPS();
  errCode[2] = SetGeiger();
  errCode[3] = SetBattery();
  for(int i = 0; i<4; i++)
    if(errCode[i])
      errorHandler(errCode[i]);

  //Start Comms
  //SetLoRa();
  //SetSD();
  //SetWifi();

  //Set Initial GPS Data
  GPS.Time[0] = 0;
  GPS.Time[1] = 0;
  GPS.Time[2] = 0;
  GPS.Pos[0] = 0;
  GPS.Pos[1] = 0;
  GPS.Pos[2] = 0;

  //Sucess Sound
  #ifndef SILENCE
  BuzzerTone(100);
  delay(200);
  BuzzerTone(100);
  delay(100);
  BuzzerTone(50);
  delay(50);
  BuzzerTone(600);
  #endif

  //Turns Leds Off
  LEDOff();
}

void loop(){
  #ifdef HMC_CALIB
    HMCCalib();
  #endif
 
  ReadGY87(&GY87);
  CorrectMag(&GY87);
  ReadGPS(&GPS);
  ReadGeiger(&Geiger);
  CreateLoRaMessage(GY87, GPS, Geiger, ReadBattery(), &Query);
  //sendHttpRequest(Query);
  //SendLoRa(Query);
  //WriteSD(Query);

  #ifdef SERIAL_DEBUG
    Serial.println(Query);
  #endif

  delay(1000);
}
