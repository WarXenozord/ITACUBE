#include "PION_System.h"
#include "ArduinoJson.h"
#include "WiFi.h"
#include "HTTPClient.h"

#define TEAM 41 //Numero da Equipe

System cubeSat;
const char* serverName = "http://192.168.0.1:8080/"; // Url do Server
WiFiClient client; // Cliente que comunica com o server
time_t lastTime;
const time_t timeDelay = 5; // Delay entre as mensagens para o server, em segundos

void createMsg(JsonDocument& message){
  message["equipe"] = TEAM;
  message["bateria"] = cubeSat.getBattery();
  message["temperatura"] = cubeSat.getTemperature();
  message["pressao"] = cubeSat.getPressure();

  JsonArray giroscopio = message.createNestedArray("giroscopio");
  JsonArray acelerometro = message.createNestedArray("acelerometro");
  for(int i = 0; i < 3; i++){
    acelerometro[i] = cubeSat.getAccelerometer(i);
    giroscopio[i] = cubeSat.getGyroscope(i);
  }
}


// Modificação para conectar a uma wifi
void networkConnect(){
  // Para qualquer aplicação bluetooth que possa existir
  btStop();

  // Começa WiFi se conectando ao SSID fornecido com a senha fornecida
  WiFi.begin("OBSAT_WIFI", "OBSatZenith1000");
  
  // Espera o Status de conectado
  while (WiFi.status() != WL_CONNECTED) {
    cubeSat.setRGB(BLUE);
    delay(1000);
    cubeSat.setRGB(OFF);
    delay(1000);
  }
}

void sendData(const JsonDocument& message){
    if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;
    
      http.begin(client, serverName);
      String query;
      serializeJsonPretty(message, query); 
      Serial.println(query);
      int httpResponseCode = http.POST(query);

      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
        
      http.end();
    } else {
      cubeSat.setRGB(RED);
      cubeSat.setLed(L1, HIGH);
      delay(1000);
      //fazer algo sobre esse erro
    }
}

void setup(){
  // Inicializa seu CubeSat, e seus periféricos 
  cubeSat.init();
  delay(3000); // Da um tempo para o satelite iniciar (nao misturar os sinais de luz das outras etapas)
  networkConnect();
  if(cubeSat.getSDStatus() == 1)
  {
      cubeSat.createSDLogTask();
      cubeSat.activateSDLog();
      cubeSat.setRGB(GREEN);
      delay(1000);
  } else {
      cubeSat.setRGB(RED);
      cubeSat.setLed(L2, HIGH);
      delay(1000);
      //fazer algo sobre esse erro
  }
  lastTime = time(NULL);
}

void loop(){
  if(time(NULL) - lastTime > timeDelay)
  {
    cubeSat.setRGB(GREEN);
    StaticJsonDocument<350> message; // Documento do Json da mensagem com 350 bytes
    createMsg(message);
    sendData(message);
    lastTime = time(NULL);
    delay(1000);
  } else {
    cubeSat.setRGB(OFF);
  }
}
