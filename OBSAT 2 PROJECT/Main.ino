//Main Execution Sequence with RTOS
//J. Libonatti

#include "Curie.h"                      //Configuration Header with types used in multiple modules
#include <esp_task_wdt.h>               //Header for watchdog

//Globals
static DataGPS GPS = {};                //Stores GPS Data as defined in Curie Header
static SemaphoreHandle_t GPSMutex;      //Mutex for GPS Data

static DataGeiger Geiger = {};          //Stores GY-87 Data as defined in Curie Header
static SemaphoreHandle_t GGMutex;       //Mutex for Geiger Data

static DataGY87 GY87 = {};              //Stores GY-87 Data as defined in Curie Header
static SemaphoreHandle_t GYMutex;       //Mutex for GY-87 Data

static int Battery = 0;                 //Stores Battery power from 0 to 100%
                                        //No need for mutex as using 1 cycle operation variable (int)

//------ReadingTasks-------//

//Those tasks read the sensors data periodically and store them in the global variables above

void readMultiSensor(void *param)       //Reads Acelerometer, Gyrometes, Barometer, Magnetometer and Termometer
{
  esp_task_wdt_add(NULL);               //Adds task to watchdog timer, it will reset cpu if the task does not call a reset
  DataGY87 localGY;                     //Creates a local copy of dataGY to avoid using global variable for long
  while(1){                             //Infinite task loop
    esp_task_wdt_reset();                       //Resets the watchdog timer Note: EVERY task with wdt_add must do this or the cpu will be reset
    ReadGY87(&localGY);                         //Reads GY87 at local GY87 struct
    CorrectMag(&localGY);                       //Corrects Magnetometer according to calibration constants set in Curie.h
    if(xSemaphoreTake(GYMutex, 0) == pdTRUE){   //try to take the mutex (GYMutex) if it is available in the next 0 ticks 
      GY87 = localGY;                           //if mutex is taken, save local GY-87 to GY-87 global
      xSemaphoreGive(GYMutex);                  //gives back the mutex
    }                                           //Note: the task shouldn't wait to take the mutex as the sensor data will update
    vTaskDelay(GY_DELAY / portTICK_PERIOD_MS);  //Waits for the period defined in curie header
  }
}

void readGPS(void *param)               //Reads GPS position and Time
{
  esp_task_wdt_add(NULL);               //Same as readMultiSensor function
  DataGPS localGPS;                     //Same as readMultiSensor function but with DataGPS type instead
  while(1){                             //Same as readMultiSensor function
    esp_task_wdt_reset();                       //Same as readMultiSensor function
    ReadGPS(&localGPS);                         //Same as readMultiSensor function but with DataGPS type instead
    if(xSemaphoreTake(GPSMutex, 0) == pdTRUE){  //Same as readMultiSensor function but with GPS mutex instead
      GPS = localGPS;                           //Same as readMultiSensor function but with DataGPS variable instead 
      xSemaphoreGive(GPSMutex);                 //Same as readMultiSensor function but with GPS mutex instead
    }
    vTaskDelay(GPS_DELAY / portTICK_PERIOD_MS); //Waits for the period defined in curie header
  }
}

void readGGR(void *param)               //Reads Geiger Counts from previous Tf to this DataGeiger Tf
{ 
  esp_task_wdt_add(NULL);               //Same as readMultiSensor function
  DataGeiger localGeiger;               //Same as readMultiSensor function but with DataGeiger type instead
  while(1){                             //Same as readMultiSensor function
    esp_task_wdt_reset();                       //Same as readMultiSensor function
    ReadGeiger(&localGeiger);                   //Same as readMultiSensor function but with DataGeiger type instead
    if(xSemaphoreTake(GGMutex, portMAX_DELAY) == pdTRUE){   //Geiger can wait as the counts are only dependent on the final time
      Geiger = localGeiger;                     //Same as readMultiSensor function but with DataGeiger type instead
      xSemaphoreGive(GGMutex);                  //Same as readMultiSensor but with Geiger mutex instead
    }
    vTaskDelay(GG_DELAY / portTICK_PERIOD_MS);  //Waits for the period defined in curie header
  }
}

void readBattery(void *param)           //Reads Battery percentage from 0 to 100%
{
  esp_task_wdt_add(NULL);               //Same as readMultiSensor function
  while(1){                             //Same as readMultiSensor function
    esp_task_wdt_reset();                         //Same as readMultiSensor function
    Battery = ReadBattery();                      //No need for mutex as it is a single cycle operation
    vTaskDelay(BAT_DELAY / portTICK_PERIOD_MS);   //Waits for the period defined in curie header
  }
}


//------Message Tasks-------//

//Those tasks read the globals variables updated by the reading tasks and send/store them 

void sendLoRa(void *param)              //Sends LoRa Radio Transmission
{
  DataGY87 localGY;                     // |
  DataGPS localGPS;                     // |Local Variables to avoid using the globals for too long
  DataGeiger localGeiger;               // |
  esp_task_wdt_add(NULL);               //Adds task to watchdog timer, it will reset cpu if the task does not call a reset 
  while(1){                             //Endless task main loop
    esp_task_wdt_reset();                                   //Resets Watchdog
    if(xSemaphoreTake(GYMutex, portMAX_DELAY) == pdTRUE){   //Try to take GY mutex. Waits until it is available
        localGY = GY87;                                     //Saves a local copy of global GY-87 
        xSemaphoreGive(GYMutex);                            //Gives GY Mutex back
    }
    if(xSemaphoreTake(GPSMutex, portMAX_DELAY) == pdTRUE){  //Same as above but with dataGPS
        localGPS = GPS;
        xSemaphoreGive(GPSMutex);
    }
    if(xSemaphoreTake(GGMutex, portMAX_DELAY) == pdTRUE){   //Same as above but with dataGeiger
        localGeiger = Geiger;
        xSemaphoreGive(GGMutex);
    }
    String msg;                                           //Create the string where the message will be writen
    CreateLoRaMessage(localGY, localGPS, localGeiger, Battery, &msg); //Creates the message to be sent in JSON format
    
    SendLoRa(msg);                                        //Sends the mensage via LoRa Radio
    vTaskDelay(LORA_DELAY / portTICK_PERIOD_MS);          //Waits for the period defined in curie header
  }
}

void sendHttp(void *param)              //Sends Wifi Radio Transmission via http requisition
{
  DataGY87 localGY;                     //All here is the same as sendLoRa
  DataGPS localGPS;                     // -
  DataGeiger localGeiger;               // -
  esp_task_wdt_add(NULL);               // -
  while(1){                                                 // -
    esp_task_wdt_reset();                                   // -
    if(xSemaphoreTake(GYMutex, portMAX_DELAY) == pdTRUE){   // -
        localGY = GY87;                                     // -
        xSemaphoreGive(GYMutex);                            // -
    }                                                       // -
    if(xSemaphoreTake(GPSMutex, portMAX_DELAY) == pdTRUE){  // -
        localGPS = GPS;                                     // -
        xSemaphoreGive(GPSMutex);                           // -
    }                                                       // -
    if(xSemaphoreTake(GGMutex, portMAX_DELAY) == pdTRUE){   // -
        localGeiger = Geiger;                               // -
        xSemaphoreGive(GGMutex);                            // -
    }                                                       // -
    String query;                                           //Same purpose, different name
    CreateHttpMessage(localGY, localGPS, localGeiger, Battery, &query); //Same thing but different format for wifi
    
    esp_task_wdt_reset();                                   //A second reset was needed as the function takes too long waiting
                                                            //Note: if this function is extended to OBSAT 4min, well need a workaround
    SendHttpRequest(query);                                 //Sends the Query via http
    vTaskDelay(WIFI_DELAY / portTICK_PERIOD_MS);            //Waits for the period defined in curie header
  }
}


void writeSD(void *param)               //Sends Wifi Radio Transmission via http requisition
{
  DataGY87 localGY;                     //All here is the same as sendLoRa
  DataGPS localGPS;                     // -
  DataGeiger localGeiger;               // -
  esp_task_wdt_add(NULL);               // -
  while(1){                                                 // -
    esp_task_wdt_reset();                                   // -
    if(xSemaphoreTake(GYMutex, portMAX_DELAY) == pdTRUE){   // -
        localGY = GY87;                                     // -
        xSemaphoreGive(GYMutex);                            // -
    }                                                       // -
    if(xSemaphoreTake(GPSMutex, portMAX_DELAY) == pdTRUE){  // -
        localGPS = GPS;                                     // -
        xSemaphoreGive(GPSMutex);                           // -
    }                                                       // -
    if(xSemaphoreTake(GGMutex, portMAX_DELAY) == pdTRUE){   // -
        localGeiger = Geiger;                               // -
        xSemaphoreGive(GGMutex);                            // -
    }                                                       // -
    String dataToStore;                                     //Same purpose, different name
    CreateSDMessage(localGY, localGPS, localGeiger, Battery, &dataToStore); //Same thing but different format for SD
    #ifdef SERIAL_DEBUG_SD                                  //If the switch is defined
      Serial.println(dataToStore);                          //Prints each saved data to serial
    #endif
    WriteSD(dataToStore);                                   //Saves data in SD
    vTaskDelay(SD_DELAY / portTICK_PERIOD_MS);              //Waits for the period defined in curie header
  }
}

//-------Setup Task---------//

//This task starts the system
//Note: at this point the RTOS is "deactivated" as this is the only task up

void setup() {   
                               
//-------Start Debug--------//
  
#ifdef SERIAL_DEBUG             //If the switch is defined
  Serial.begin(115200);         //Begin Serial at 115200 baud rate
#endif
  
//--------Setup IO----------//

  SetLED();                     //Starts the Led, keeps it on
  SetBuzzer();                  //Starts the buzzer
  int errCode[7];               //Variable to store error codes
  
//------Start Sensors-------//
  
#ifdef ENABLE_GY                //If the switch is defined
    errCode[0] = SetGY87();     //Start GY multisensor. Saves the return code
#else
    errCode[0] = 0;             //No errors from something that we won't even start :D
#endif 
  
#ifdef ENABLE_GPS               //Similar to errCode[0]
  errCode[1] = SetGPS();        //Start GPS. Saves the return code
#else
  errCode[1] = 0;
#endif 
  
#ifdef ENABLE_GG                //Similar to errCode[0]
  errCode[2] = SetGeiger();     //Start Geiger Counter. Saves the return code
#else
  errCode[2] = 0;
#endif 

  errCode[3] = SetBattery();    //Start Battery Reader, Saves the return code

//-------Start Comms---------//
  
#ifdef ENABLE_WIFI              //Similar to errCode[0]
  errCode[4] = SetWifi();       //Start and connect wifi. Saves the return code
#else
  errCode[4] = 0;
#endif 

#ifdef ENABLE_SD                 //Similar to errCode[0] 
  errCode[5] = SetSD();         //Start SD. Saves the return code
#else
  errCode[5] = 0;
#endif 

#ifdef ENABLE_LORA              //Similar to errCode[0]
  errCode[6] = SetLoRa();       //Start LoRa Radio. Saves the return code
#else
  errCode[6] = 0;
#endif

//------Checks Errors--------//

  for(int i = 0; i<7; i++)      //Loops through each return Value
    if(errCode[i])              //If the return value is not 0 (which means it is an error).
    {
      #ifdef SERIAL_DEBUG                     //If the switch is defined
      Serial.print("Detected Error Code: ");  //Prints Error Code to serial
      Serial.println(errCode[i]);
      #endif
      #ifndef UNSAFE_BOOT                     //If the switch is NOT defined
        errorHandler(errCode[i]);             //Calls the error handler, which will deal with each error individually
      #endif
    }

//----Semaphore Creation----//
  GYMutex = xSemaphoreCreateMutex();          //Mutex are used to avoid two tasks using the same resource at same time
  GPSMutex = xSemaphoreCreateMutex();         //what would cause lots of unwanted behaviours. There is one for each
  GGMutex = xSemaphoreCreateMutex();          //shared resource except the battery, as it's update is one cycle

//------Creates Watchdog----//

  esp_task_wdt_init(WDT_TIMEOUT, true);         //This watchdog is a timer that, if not reset by EACH TASK in it, will reset the CPU every WDT_Timeout seconds, defined in Curie.h
  
//------Task Creation------//

//Here we start the other tasks so the RTOS begin to function here

//GY87 task
#ifdef ENABLE_GY                              //If the switch is defined
  xTaskCreatePinnedToCore(readMultiSensor,    //Function (The name of task function)
                        "ReadMultiSensor",    //Stack Name (The name of the stack)
                        3200,                 //Stack Size  (Size, in bytes, of the stack)
                        NULL,                 //Parameter to Pass (the parameter (void*) which can be passed to the task)
                        1,                    //Task Priority (0 is the lowest)
                        NULL,                 //Task Handle (if you want to change the task later, you need to store it in a handle)
                        1                     //Core to Run (core 1 is advised as core 0 is busy with wifi)
  );
#endif

//GPS task
#ifdef ENABLE_GPS                             //If the switch is defined
  xTaskCreatePinnedToCore(readGPS,            //Function
                        "ReadGPS",            //Stack Name
                        2048,                 //Stack Size
                        NULL,                 //Parameter to Pass
                        1,                    //Task Priority
                        NULL,                 //Task Handle
                        1                     //Core to Run
  );
#endif


//GG task
#ifdef ENABLE_GG                              //If the switch is defined
  xTaskCreatePinnedToCore(readGGR,            //Function
                        "ReadGGR",            //Stack Name
                        2048,                 //Stack Size
                        NULL,                 //Parameter to Pass
                        2,                    //Task Priority
                        NULL,                 //Task Handle
                        1                     //Core to Run
  );
#endif

//Battery task
  xTaskCreatePinnedToCore(readBattery,        //Function
                        "ReadBattery",        //Stack Name
                        2048,                 //Stack Size
                        NULL,                 //Parameter to Pass
                        1,                    //Task Priority
                        NULL,                 //Task Handle
                        1                     //Core to Run
  );

//Wifi http task
#ifdef ENABLE_WIFI                            //If the switch is defined
  xTaskCreatePinnedToCore(sendHttp,           //Function
                        "sendHttp",           //Stack Name
                        3200,                 //Stack Size
                        NULL,                 //Parameter to Pass
                        2,                    //Task Priority
                        NULL,                 //Task Handle
                        1                     //Core to Run
  );
#endif

//SD task
#ifdef ENABLE_SD                              //If the switch is defined
  xTaskCreatePinnedToCore(writeSD,            //Function
                        "writeSD",            //Stack Name
                        3200,                 //Stack Size
                        NULL,                 //Parameter to Pass
                        1,                    //Task Priority
                        NULL,                 //Task Handle
                        1                     //Core to Run
  );
#endif

//Radio Task
#ifdef ENABLE_LORA                            //If the switch is defined
  xTaskCreatePinnedToCore(sendLoRa,           //Function
                        "sendLoRa",           //Stack Name
                        3200,                 //Stack Size
                        NULL,                 //Parameter to Pass
                        1,                    //Task Priority
                        NULL,                 //Task Handle
                        1                     //Core to Run
  );
#endif
  
//------Sucess Sound------//
  #ifndef SILENCE                             //If the switch is defined
  BuzzerTone(100);                            //Outputs a classy victory sound!
  delay(200);
  BuzzerTone(100);
  delay(100);
  BuzzerTone(50);
  delay(50);
  BuzzerTone(600);
  #endif
  
//------Turns Leds Off------//
  LEDOff();                                   //Turns off LED to futher signalize everything went okay
  vTaskDelete(NULL);                          //Deletes Setup, so Loop() is never started
}

void loop(){} //Not used and never alive
