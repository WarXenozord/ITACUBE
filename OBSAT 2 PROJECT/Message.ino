// This module formats the data for use in Http, SD and LoRa.
// J. Libonatti

#include <ArduinoJson.h>                    // Lib for formating in Json
#include "Curie.h"

long long msgId = 0;                        // main Id used in SD mensages
long long lMsgId = 0;                       // main Id used in lora mensages

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

void CreateLoRaMessage(struct DataGY87 GY87, struct DataGPS GPS, struct DataGeiger Geiger, int Battery, String *query){
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
}
