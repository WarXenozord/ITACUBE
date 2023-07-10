// This module formats handles the ESP32 wifi module and http requisition according to OBSAT Specs.
// J. Libonatti

#include <WiFi.h>
#include "Curie.h"
#include <HTTPClient.h>

WiFiClient client; // Cliente que comunica com o server

void SetWifi(){
  // Para qualquer aplicação bluetooth que possa existir
  btStop();

  // Começa WiFi se conectando ao SSID fornecido com a senha fornecida
  WiFi.begin("OBSAT_WIFI", "OBSatZenith1000");
  
  // Espera o Status de conectado
  /*while (WiFi.status() != WL_CONNECTED) {
    cubeSat.setRGB(BLUE);
    delay(1000);
    cubeSat.setRGB(OFF);
    delay(1000);
  }*/
}

void SendHttpRequest(const String query){
    if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;
    
      http.begin(client, serverName);
      int httpResponseCode = http.POST(query);

      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
        
      http.end();
    } /*else {
      //fazer algo sobre esse erro
    }*/
}
