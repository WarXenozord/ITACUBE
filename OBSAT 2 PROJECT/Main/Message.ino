// This module formats the data for use in Http, SD and LoRa.
// J. Libonatti

#include <ArduinoJson.h>                    // Lib for formating in Json
#include "Curie.h"

long long GYId = 0;                         // main Id used in SD mensages
long lMsgId = 0;                       // main Id used in lora mensages

void CreateHttpMessage(struct DataGY87 GY87, struct DataGPS GPS, struct DataGeiger Geiger, int Battery, String *query){ //create http request as defined in OBSAT Requirements
  *query = "";                              // Clears any previous message
  StaticJsonDocument<400> message;          // Json with storage capacity for 400 bytes
  message["equipe"] = TEAM;                 // defines Team as in header currie
  message["bateria"] = Battery;             // proceeds to assign each sensor reading value to their Json fields
  message["temperatura"] = GY87.Temp;       
  message["pressao"] = GY87.Pres;    

  JsonArray giroscopio = message.createNestedArray("giroscopio");
  JsonArray acelerometro = message.createNestedArray("acelerometro");
  for(int i = 0; i < 3; i++){
    acelerometro[i] = GY87.Acc[i];
    giroscopio[i] = GY87.Gyro[i];
  }
  JsonObject payload = message.createNestedObject("payload");
  JsonArray P = payload.createNestedArray("P");                         // 6 bytes
  JsonArray M = payload.createNestedArray("M");                         // 6 + 6 bytes
  for(int i = 0; i < 3; i++){
    P[i] = (int)(GPS.Pos[i]*100000 + 0.5)/100000.0;                     // 6 + 6 + 21 bytes
    M[i] = (int)(GY87.Mag[i]*100 + 0.5)/100.0;                          // 6 + 6 + 21 + 15 bytes
  }
  
  payload["T"] =  GPS.Time[0] + GPS.Time[1] * 60 + GPS.Time[2] * 3600;  //6 + 6 + 21 + 15 + 4 + 5(worst case) bytes
  payload["C"] = Geiger.counts;                                         //6 + 6 + 21 + 15 + 4 + 5(worst case) + 4 + 5(worst case) bytes
  payload["Tf"] = Geiger.tf;                                            //6 + 6 + 21 + 15 + 4 + 5(worst case) + 4 + 5(worst case) + 10(worst case) bytes
  serializeJson(message, *query);                                       // 76 bytes used of 90 available per OBSAT requirements
}

const String GYMessage(const struct DataGY87 GY87)
{
  StaticJsonDocument<300> message;        // Json wih 200 bytes capacity
  message["id"] = GYId;                   // GY message unique ID
  GYId++;
  message["ms"] = GY87.tf;                // Time of barometer and temperature measurements
  message["temp"] = GY87.Temp;            //temperature in celsius
  message["pres"] = GY87.Pres;            //pressure in PA
  message["ms2"] = GY87.tf - 31;          // Time of acelerometer, gyrometer and magnetometer measurements
  JsonArray giroscopio = message.createNestedArray("giro"); //Acelerometer and magnetometer readings
  JsonArray acelerometro = message.createNestedArray("acel"); 
  JsonArray magnetometro = message.createNestedArray("mag");
  for(int i = 0; i < 3; i++){
    acelerometro[i] = GY87.Acc[i];
    giroscopio[i] = GY87.Gyro[i];
    magnetometro[i] = GY87.Mag[i];
  }
  String msg;
  serializeJson(message, msg);  //formats in json
  return msg;
}

const String GPSMessage(const struct DataGPS GPS)
{
  StaticJsonDocument<200> message;        // Json wih 200 bytes capacity
  message["ms"] = GPS.tf;                 // time of received measuremente
  JsonObject gpsArray = message.createNestedObject("gps");  
  JsonArray P = gpsArray.createNestedArray("P");  //Gps fix
  for(int i = 0; i < 3; i++){
    P[i] = GPS.Pos[i];
  }
  gpsArray["T"] =  GPS.Time[0] + GPS.Time[1] * 60 + GPS.Time[2] * 3600; //GPS RTC time. Converts time to seconds since the day began
  String msg;
  serializeJson(message, msg);
  return msg;
}

const String GeigerMessage(const struct DataGeiger GG) //Geiger measurement: counts + final time of measurement
{
  return String(GG.counts) + "," + String(GG.tf);
}

const String BatMessage(const int Bat)  //Geiger measurement: battery % + final time of measurement
{
  return String(Bat)+ "," + String(millis());
}

// Used only on debug as storage type stores all data in separated files
#ifdef SERIAL_DEBUG_SD                                  //If the switch is defined
void CreateSDMessage(struct DataGY87 GY87, struct DataGPS GPS, struct DataGeiger Geiger, int Battery, String *query){
  *query = "";                            // Clears any previous message
  StaticJsonDocument<430> message;        // Json wih 430 bytes capacity
  message["id"] = msgId;                  // Message unique ID
  msgId++;                                // Increments ID
  message["ms"] = millis();               // Time in ms retrieve from millis (maybe choose a better function later like the RTC? ~J)
  message["bat"] = Battery;               // proceeds to assign each sensor reading value to their Json fields  
  message["temp"] = GY87.Temp; 
  message["pres"] = GY87.Pres;

  JsonArray giroscopio = message.createNestedArray("giro"); 
  JsonArray acelerometro = message.createNestedArray("acel"); 
  JsonArray magnetometro = message.createNestedArray("mag");
  for(int i = 0; i < 3; i++){
    acelerometro[i] = GY87.Acc[i];
    giroscopio[i] = GY87.Gyro[i];
    magnetometro[i] = GY87.Mag[i];
  }
  JsonObject gpsArray = message.createNestedObject("gps");
  JsonArray P = gpsArray.createNestedArray("P");
  for(int i = 0; i < 3; i++){
    P[i] = GPS.Pos[i];
  }
  gpsArray["T"] =  GPS.Time[0] + GPS.Time[1] * 60 + GPS.Time[2] * 3600; //Converts time to seconds since the day began

  JsonObject geigerArray = message.createNestedObject("ggr");

  geigerArray["C"] = Geiger.counts;
  geigerArray["Tf"] = Geiger.tf;

  serializeJson(message, *query);         // Creates the Json without newlines
}
#endif

void CreateLoRaMessage(struct DataGY87 GY87, struct DataGPS GPS, struct DataGeiger Geiger, int Battery, byte *query){
  storeUInt16(lMsgId, query, 0);
  lMsgId++;
  storeUInt(millis(), query, 2);               // Same as the SD function
  storeFloat((float) GPS.Pos[0], query, 6);
  storeFloat((float) GPS.Pos[1], query, 10);
  storeUInt16((uint16_t) GPS.Pos[2], query, 14);
  query[16] = (uint8_t) GPS.Time[2]; 
  query[17] = (uint8_t) GPS.Time[1];
  query[18] = (uint8_t) GPS.Time[0];
  query[19] = (int8_t) Battery;
  query[20] = (int8_t) GY87.Temp;
  storeFloat(GY87.Pres, query, 21);
  storeUInt16(Geiger.counts, query, 25);
  storeUInt(Geiger.tf, query, 27);

  //8bit checksum
  query[31] = 0;
  for(int i=0; i<31; i++)
    query[31] += query[i];

  /*JsonArray giroscopio = message.createNestedArray("giro");
  JsonArray acelerometro = message.createNestedArray("acel");
  JsonArray magnetometro = message.createNestedArray("mag");
  for(int i = 0; i < 3; i++){
    acelerometro[i] = (int)(GY87.Acc[i]*100000 + 0.5)/100000.0;
    giroscopio[i] = (int)(GY87.Gyro[i]*1000 + 0.5)/1000.0;
    magnetometro[i] = (int)(GY87.Mag[i]*100 + 0.5)/100.0;;
  }*/

}

/*void CreateLoRaMessage(struct DataGY87 GY87, struct DataGPS GPS, struct DataGeiger Geiger, int Battery, String *query){
  *query = "";                            // Clears any previous message
  StaticJsonDocument<430> message;        // Json wih 430 bytes capacity
  message["id"] = lMsgId;                 // Lora unique message ID
  lMsgId++;
  message["ms"] = millis();               // Same as the SD function
  message["bat"] = Battery;
  message["temp"] = GY87.Temp;
  message["pres"] = GY87.Pres;  

  JsonArray giroscopio = message.createNestedArray("giro");
  JsonArray acelerometro = message.createNestedArray("acel");
  JsonArray magnetometro = message.createNestedArray("mag");
  for(int i = 0; i < 3; i++){
    acelerometro[i] = (int)(GY87.Acc[i]*100000 + 0.5)/100000.0;
    giroscopio[i] = (int)(GY87.Gyro[i]*1000 + 0.5)/1000.0;
    magnetometro[i] = (int)(GY87.Mag[i]*100 + 0.5)/100.0;;
  }
  JsonObject gpsArray = message.createNestedObject("gps");
  JsonArray P = gpsArray.createNestedArray("P");
  for(int i = 0; i < 3; i++){
    P[i] = GPS.Pos[i];
  }
  gpsArray["T"] =  GPS.Time[0] + GPS.Time[1] * 60 + GPS.Time[2] * 3600;
  JsonObject geigerArray = message.createNestedObject("ggr");
  
  geigerArray["C"] = Geiger.counts;
  geigerArray["Tf"] = Geiger.tf;

  serializeJson(message, *query); 
}*/

void storeUInt16(uint16_t a, byte *buff, int pointer){
  buff[pointer] = a;
  buff[pointer + 1] = a >> 8;
}

void storeInt16(int16_t a, byte *buff, int pointer){
  buff[pointer] = a;
  buff[pointer + 1] = a >> 8;
}

void storeUInt(uint32_t a, byte *buff, int pointer){
  buff[pointer] = a;
  buff[pointer + 1] = a >> 8;
  buff[pointer + 2] = a >> 16;
  buff[pointer + 3] = a >> 24;
}

void storeFloat(float a, byte *buff, int pointer){
    buff[pointer] = ((uint8_t*)&a)[0];
    buff[pointer + 1] = ((uint8_t*)&a)[1];
    buff[pointer + 2] = ((uint8_t*)&a)[2];
    buff[pointer + 3] = ((uint8_t*)&a)[3];
}

void storeDouble(double a, byte *buff, int pointer){
    buff[pointer] = ((uint8_t*)&a)[0];
    buff[pointer + 1] = ((uint8_t*)&a)[1];
    buff[pointer + 2] = ((uint8_t*)&a)[2];
    buff[pointer + 3] = ((uint8_t*)&a)[3];
    buff[pointer + 4] = ((uint8_t*)&a)[4];
    buff[pointer + 5] = ((uint8_t*)&a)[5];
    buff[pointer + 6] = ((uint8_t*)&a)[6];
    buff[pointer + 7] = ((uint8_t*)&a)[7];
}