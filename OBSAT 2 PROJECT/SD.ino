// This module formats handles the SD Card Writing.
// J. Libonatti

#include <SD.h>                     // SD Lib

#define MICROSD_PIN_CHIP_SELECT 2   // MicroSD SPI Pins
#define MICROSD_PIN_SCK 17
#define MICROSD_PIN_MOSI 23
#define MICROSD_PIN_MISO 25

File dataFile;                          // file to save data

SPIClass SPI2(HSPI);                // Selects ESP32 SPI interface not used by LoRa

int SetSD(){                        // Starts the SD card and opens data file

  SPI2.begin(MICROSD_PIN_SCK, MICROSD_PIN_MISO, MICROSD_PIN_MOSI, MICROSD_PIN_CHIP_SELECT); // Begins SPI protocol
  if (!SD.begin(MICROSD_PIN_CHIP_SELECT, SPI2))                                             // Tries to start SD card
    return -61;                                                                             // Returns Error if failed
    
  dataFile = SD.open("/data.txt", FILE_APPEND);                 // Tries to open data.txt in file append (add to end) mode
  if (!dataFile) {                                              // Check if root pointer is valid
    return -62;                                             // Returns Error if not
  }  
                                     
  return 0;                                                 
}

void WriteSD(const String data)     // Writes the string to SD file
{
  dataFile.println(data);               // Writes to data file with a \n at the end
  dataFile.flush();                     // Does a flush to ensure the SD saves all data
}
