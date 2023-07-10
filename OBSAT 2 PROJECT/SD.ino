// This module formats handles the SD Card Writing.
// J. Libonatti

#include <SD.h>

#define MICROSD_PIN_CHIP_SELECT 2
#define MICROSD_PIN_SCK 17
#define MICROSD_PIN_MOSI 23
#define MICROSD_PIN_MISO 25

File root;

SPIClass SPI2(HSPI);

void SetSD(){

  SPI2.begin(MICROSD_PIN_SCK, MICROSD_PIN_MISO, MICROSD_PIN_MOSI, MICROSD_PIN_CHIP_SELECT);
  if (!SD.begin(MICROSD_PIN_CHIP_SELECT, SPI2)) {
    while(1)    
    Serial.println("initialization failed!");
    
    return;
  }
  Serial.println("initialization done.");
  /* open "test.txt" for writing */
  root = SD.open("/data.txt", FILE_WRITE);
  if (!root) {
    /* if the file open error, print an error */
    Serial.println("error opening test.txt");
  }
  delay(1000);
}

void WriteSD(const String data)
{
  root.println(data);
  root.flush();
}
