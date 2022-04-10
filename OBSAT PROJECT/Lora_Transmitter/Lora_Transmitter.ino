#include "heltec.h"
#include "ArduinoJson.h"

#define BAND 433E6 //banda do LoRa
#define RXD 16
#define TXD 17

void setup() {
  Heltec.begin(false, true, false, true, BAND);
  Serial.begin(115200, SERIAL_8N1, 3, 1);
  Serial2.begin(115200, SERIAL_8N1, RXD, TXD);
  delay(100);
}

void loop() {    
   String message;
   if(Serial2.available() > 0)
   {
     char c = char(Serial2.read());
     while(c != '!')
     {
       if(Serial2.available() > 0)
       {
         message += c;
         c = char(Serial2.read());
       }
     }
   }
   Serial.print(message);
   LoRa.beginPacket();
   LoRa.print(message);
   LoRa.endPacket();
}
