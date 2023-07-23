// This module formats handles the SD Card Writing.
// J. Libonatti

#include <SD.h>                     // SD Lib

#define MICROSD_PIN_CHIP_SELECT 2   // MicroSD SPI Pins
#define MICROSD_PIN_SCK 17
#define MICROSD_PIN_MOSI 23
#define MICROSD_PIN_MISO 25

File dataFile[5];                    // files to save data

SPIClass SPI2(HSPI);                // Selects ESP32 SPI interface not used by LoRa

int SetSD(){                        // Starts the SD card and opens data file

  SPI2.begin(MICROSD_PIN_SCK, MICROSD_PIN_MISO, MICROSD_PIN_MOSI, MICROSD_PIN_CHIP_SELECT); // Begins SPI protocol
  if (!SD.begin(MICROSD_PIN_CHIP_SELECT, SPI2))                                             // Tries to start SD card
    return -61;                                                                             // Returns Error if failed
    
  dataFile[0] = SD.open("/GY87.txt", FILE_APPEND);                      // Tries to open <name>.txt in file append (add to end) mode
  dataFile[1] = SD.open("/GPS.txt", FILE_APPEND); 
  dataFile[2] = SD.open("/GGR.txt", FILE_APPEND); 
  dataFile[3] = SD.open("/BAT.txt", FILE_APPEND); 
  dataFile[4] = SD.open("/LOG.txt", FILE_APPEND); 
  
  for(int i = 0; i < 4; i++)
    if (!dataFile[i])                                                   // Check if root pointer is valid
      return -62 - i;                                                   // Returns Error if not                                 
  return 0;                                                 
}

void WriteSD(const String data, uint8_t fileNumber)     // Writes the string to specified SD file
{
  (dataFile[fileNumber]).println(data);           // Writes to data file with a \n at the end
  (dataFile[fileNumber]).flush();                 // Does a flush to ensure the SD saves all data
}
