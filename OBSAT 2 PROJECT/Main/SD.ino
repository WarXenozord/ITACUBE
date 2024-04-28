// This module formats handles the SD Card Writing.
// J. Libonatti
#include "Curie.h"
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

  for(int i = 0; i < 5; i++)
    if (!dataFile[i])                                                   // Check if root pointer is valid
      return -62 - i;                                                   // Returns Error if not   

  (dataFile[4]).close();                                                    // Close log to open the vfs_fat file descriptor                              
  return 0;                                                 
}

void WriteSD(const String data, uint8_t fileNumber)     // Writes the string to specified SD file
{
  if(fileNumber == 4){                             // Low usage files like LOG or DEMOS aren't load by default
    dataFile[4] = SD.open("/LOG.txt", FILE_APPEND); 
  }

  #ifdef DEMOS
  else if(fileNumber == 5){
    dataFile[4] = SD.open("/DEMOS.txt", FILE_APPEND); 
    fileNumber = 4;
  }
  #endif

  (dataFile[fileNumber]).println(data);           // Writes to data file with a \n at the end
  (dataFile[fileNumber]).flush();                 // Does a flush to ensure the SD saves all data
    
  if(fileNumber > 3){
    (dataFile[fileNumber]).close();               // Closes if it is a low usage file
  }

}

void ReadSD(uint8_t fileNumber, uint8_t* buffer, uint16_t len, int32_t pos) // Read from filenumber and store on the buffer data from pos to pos+len-1, returns cstring ('\0' terminated)
{
  if(fileNumber == 4){                             // Low usage files like LOG or DEMOS aren't load by default
    dataFile[4] = SD.open("/LOG.txt", FILE_APPEND); 
  }

  #ifdef DEMOS
    else if(fileNumber == 5){
      dataFile[4] = SD.open("/DEMOS.txt"); 
      fileNumber = 4;                             // Change fileNumber as it is using the same fat decryptor slot as LOG
    }
  #endif

  if(pos){
    (dataFile[fileNumber]).seek(pos);             // Seeks position on filecursor in reference to the start of the file
  }

  for(int i = 0; i < (len - 1); i++){
    buffer[i] = (dataFile[fileNumber]).read();    // Reads data to buffer
  }
  buffer[len - 1] = '\0';                         // Terminates string

  if(fileNumber > 3){
    (dataFile[fileNumber]).close();               // Closes if it is a low usage file (LOG, DEMO, CONFIG)
  }
}

#ifdef DEMOS

int ReadMelody(String melodyName, int** melody, int** durations)  // Read a melody from the archive format
{                                                                 // 0000{0,0,0,0,0}{0,0,0,0,0}
  File melodyFile = SD.open("/Melodies/"+ melodyName + ".txt");   // Where the first 4 numbers are the size of array
  if(!melodyFile){                                                // the first array is melody and the second durations
    return 0;                                                     // **WARNING: THIS FUNCTION ASSUMES THERE IS A FREE FILE DECRYPTOR**
  }

  uint8_t sizeBuffer[4];                                          // Finds array size from file
  for(int i = 0; i < 4; i++)
    sizeBuffer[i] = melodyFile.read();
  uint16_t size; 
  sscanf((char *) sizeBuffer, "%d", &size);
  Serial.println(size);

  *melody = (int*) malloc((size)*sizeof(int));                    // Allocates an array to store the melody
  if(melodyFile.read() == '{'){                                   // Parses the txt array for the values
    uint8_t c = 0;
    int p = 0;
    while(c != '}'){
      c =  melodyFile.read();
      (*melody)[p] = 0;
      while(c != ',' && c != '}'){
        (*melody)[p] = ((*melody)[p])*10 + (c - '0');
        c =  melodyFile.read();
      }
      p++;
    }
  }

  *durations = (int*) malloc((size)*sizeof(int));               // Allocates an array to store the durations
  if(melodyFile.read() == '{'){                                 // Parses the txt array for the values
    uint8_t c = 0;
    int p = 0;
    while(c != '}'){
      c =  melodyFile.read();
      (*durations)[p] = 0;
      while(c != ',' && c != '}'){
        (*durations)[p] = ((*durations)[p])*10 + (c - '0');
        c =  melodyFile.read();
      }
      p++;
    }
  }

  melodyFile.close();                                           //  Closes the File
  return size;
}

#endif
