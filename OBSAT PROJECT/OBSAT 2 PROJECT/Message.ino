// This module formats the data for use in Http, SD and LoRa.
// J. Libonatti

#include <ArduinoJson.h>
#include "Curie.h"

long long msgId = 0; //main Id used in SD mensages
long long lMsgId = 0; //main Id used in lora mensages

void CreateHttpMessage(struct DataGY87 GY87, struct DataGPS GPS, struct DataGeiger Geiger, int Battery, String *query){
  *query = "";
  StaticJsonDocument<400> message; // Documento do Json da mensagem com capacidade de 400 bytes
  message["equipe"] = TEAM;
  message["bateria"] = Battery;
  message["temperatura"] = GY87.Temp;
  message["pressao"] = GY87.Pres;

  JsonArray giroscopio = message.createNestedArray("giroscopio");
  JsonArray acelerometro = message.createNestedArray("acelerometro");
  for(int i = 0; i < 3; i++){
    acelerometro[i] = GY87.Acc[i];
    giroscopio[i] = GY87.Gyro[i];
  }
  JsonObject payload = message.createNestedObject("payload");
  JsonArray P = payload.createNestedArray("P"); //6 bytes
  JsonArray M = payload.createNestedArray("M"); //6 + 6 bytes
  for(int i = 0; i < 3; i++){
    P[i] = (int)(GPS.Pos[i]*100000 + 0.5)/100000.0; // 6 + 6 + 21 bytes
    M[i] = (int)(GY87.Mag[i]*100 + 0.5)/100.0; // 6 + 6 + 21 + 15 bytes
  }
  
  payload["T"] =  GPS.Time[0] + GPS.Time[1] * 60 + GPS.Time[2] * 3600; //6 + 6 + 21 + 15 + 4 + 5(pior caso) bytes
  payload["C"] = Geiger.counts; ////6 + 6 + 21 + 15 + 4 + 5(pior caso) + 4 + 5(Pior caso) bytes
  payload["Tf"] = Geiger.tf; ////6 + 6 + 21 + 15 + 4 + 5(pior caso) + 4 + 5(Pior caso) + 10(pior caso) bytes
  serializeJson(message, *query); // 76 bytes de 90 disponiveis
}

void CreateSDMessage(struct DataGY87 GY87, struct DataGPS GPS, struct DataGeiger Geiger, int Battery, String *query){
  *query = "";
  StaticJsonDocument<430> message; // Documento do Json da mensagem com capacidade de 400 bytes
  message["id"] = msgId; //ID da Mensagem
  msgId++;
  message["ms"] = millis(); //Tempo, em ms(), da mensagem contado do proprio ESP-32
  message["bat"] = Battery; // Nivel da bateria
  message["temp"] = GY87.Temp; //Temperatura do BMP180
  message["pres"] = GY87.Pres; //Pressão do BMP180

  JsonArray giroscopio = message.createNestedArray("giro"); //giro do mpu 6050
  JsonArray acelerometro = message.createNestedArray("acel"); // aceleração do mpu 6050
  JsonArray magnetometro = message.createNestedArray("mag"); // magnetomagia do HMC5883L
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
  gpsArray["T"] =  GPS.Time[0] + GPS.Time[1] * 60 + GPS.Time[2] * 3600; //Tempo em segundos no dia

  JsonObject geigerArray = message.createNestedObject("ggr");

  geigerArray["C"] = Geiger.counts;
  geigerArray["Tf"] = Geiger.tf;

  serializeJson(message, *query); 
}

void CreateLoRaMessage(struct DataGY87 GY87, struct DataGPS GPS, struct DataGeiger Geiger, int Battery, String *query){
  *query = "";
  StaticJsonDocument<430> message; // Documento do Json da mensagem com capacidade de 400 bytes
  message["id"] = lMsgId; //ID da Mensagem
  lMsgId++;
  message["ms"] = millis(); //Tempo, em ms(), da mensagem contado do proprio ESP-32
  message["bat"] = Battery; // Nivel da bateria
  message["temp"] = GY87.Temp; //Temperatura do BMP180
  message["pres"] = GY87.Pres; //Pressão do BMP180

  JsonArray giroscopio = message.createNestedArray("giro"); //giro do mpu 6050
  JsonArray acelerometro = message.createNestedArray("acel"); // aceleração do mpu 6050
  JsonArray magnetometro = message.createNestedArray("mag"); // magnetomagia do HMC5883L
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
  gpsArray["T"] =  GPS.Time[0] + GPS.Time[1] * 60 + GPS.Time[2] * 3600; //Tempo em segundos no dia
  JsonObject geigerArray = message.createNestedObject("ggr");
  
  geigerArray["C"] = Geiger.counts;
  geigerArray["Tf"] = Geiger.tf;

  serializeJson(message, *query); 
}
