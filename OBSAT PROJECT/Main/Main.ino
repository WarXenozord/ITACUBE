//J. Libonatti 09/04/2022


//---------------------------------------------------------------------------------//
//*********************************************************************************//
//---------------------------------------------------------------------------------//
//Libs

#include "NMEAGPS.h" // Traduz os dados brutos do gps
#include "PION_System.h" // Sistema base do cubesat
#include "ArduinoJson.h" // Formata a mensagem JSON
#include "WiFi.h" // Utiliza wifi do ESP32 para conectar ao hotspot do balao
#include "HTTPClient.h" // Cria um cliente para a requisicao http
#include "SPI.h" // protocolo SPI para o LORA

//---------------------------------------------------------------------------------//
//*********************************************************************************//
//---------------------------------------------------------------------------------//
//Defines

#define TEAM 41 //Numero da Equipe

#define RXD2 16 //Seriais do GPS
#define TXD2 17

#define RXD1 13// Seriais do Lora
#define TXD1 14

//---------------------------------------------------------------------------------//
//*********************************************************************************//
//---------------------------------------------------------------------------------//
//Variaveis globais

//Variaveis Gerais
System cubeSat;

//Comunicacao Wifi
const char* serverName = "http://192.168.0.1:8080/"; // Url do Server
WiFiClient client; // Cliente que comunica com o server
time_t lastTime;
const time_t timeDelay = 5; // Delay entre as mensagens para o server, em segundos

//Gps
NMEAGPS gps; // objeto do gps
gps_fix fix; //Ultima posicao medida pelo gps

//---------------------------------------------------------------------------------//
//*********************************************************************************//
//---------------------------------------------------------------------------------//
//Funcoes do LoRa
bool setLoRa(){
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  while(1){
    Serial2.println("Lero Lero");
  }
}

void radioTransmit(const String message){
  Serial1.print(message);
}
//---------------------------------------------------------------------------------//
//*********************************************************************************//
//---------------------------------------------------------------------------------//
//Funcoes Gerais

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
  gpsSetup();
}

void loop(){
  gpsRead();
  if(time(NULL) - lastTime > timeDelay)
  {
    String message;
    cubeSat.setRGB(GREEN);
    createMsg(&message);
    Serial.println(message);
    sendData(message);
    radioTransmit(message);
    lastTime = time(NULL);
    delay(1000);
  } else {
    cubeSat.setRGB(OFF);
  }
}

//---------------------------------------------------------------------------------//
//*********************************************************************************//
//---------------------------------------------------------------------------------//
//Funcoes da Wifi

void createMsg(String *query){
  StaticJsonDocument<400> message; // Documento do Json da mensagem com capacidade de 400 bytes
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
  JsonObject payload = message.createNestedObject("payload");
    JsonArray P = message.createNestedArray("P");
  payload["S"] = fix.satellites;
  if (fix.valid.location){
    P[0] = String(fix.latitude(),6);
    P[1] = String(fix.longitude(),6);
  }else{
    // poderia-se fazer algo para aproveitar o ultimo fix valido feito
    P[0] = "N/A";
    P[1] = "N/A";
  }
  if (fix.valid.altitude){
    P[2] = String(fix.altitude(),1);
  }else{
    P[2] = "N/A";
  }
  payload["T"] =  String(fix.dateTime.hours) + ":" + String(fix.dateTime.minutes) + ":" + String(fix.dateTime.seconds);
  serializeJsonPretty(message, *query); 
}


// Modificação para conectar a uma wifi
void networkConnect(){
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

void sendData(const String query){
    if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;
    
      http.begin(client, serverName);
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

//---------------------------------------------------------------------------------//
//*********************************************************************************//
//---------------------------------------------------------------------------------//
//Funcoes do GPS

void gpsSetup(){
  Serial1.begin(9600, SERIAL_8N1, RXD1, TXD1);
  
  // Muda serial do GPS para 38400 baud
  Serial1.print("$PCAS01,3*1F\r\n"); // obtido via Gnss Toolkit, sempre incluir \r e \n
  Serial1.flush();
  Serial1.end();
  delay(100);
  Serial1.begin(38400, SERIAL_8N1, RXD1, TXD1);

  // Muda a amostragem do GPS
  //Serial2.print("$PCAS02,1000*2E\r\n"); // 1hz
  Serial1.print("$PCAS02,200*1D\r\n"); // 5hz

  // Usa todas as constelacoes disponiveis
  Serial1.print("$PCAS04,2*1B\r\n"); // obtido via Gnss Toolkit, sempre incluir \r e \n
}

void gpsRead(){
  if (gps.available(Serial1))
  {
    fix = gps.read();// Salva os dados coletados pelo gps
    //Salvar dados no SD Card
  }
}
