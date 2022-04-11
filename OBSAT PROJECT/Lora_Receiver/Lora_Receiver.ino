#include "heltec.h"

#define BAND 433E6
char st;
void setup () {
  Heltec.begin(false, true, true, true, BAND);
  pinMode(12, OUTPUT);
}

void loop () {
  int packetSize = LoRa.parsePacket();
  
  if(packetSize) {
  Serial.print("Pacote recebido '");
    while(LoRa.available ()) {
      st = (char)LoRa.read ();
      Serial.print(st);
    }
  }
}
