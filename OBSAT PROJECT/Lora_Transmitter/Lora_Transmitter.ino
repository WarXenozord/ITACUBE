
#include "LoRa.h"

#define RXD 16
#define TXD 17



void setup() {
  Serial.begin(9600, SERIAL_8N1, 3, 1);
  Serial2.begin(9600, SERIAL_8N1, RXD, TXD);
}

void loop() {
  while(Serial2.available() > 0)
  {
    Serial.print(char(Serial2.read()));
  }
}
