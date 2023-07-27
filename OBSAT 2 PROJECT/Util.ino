// This module formats handles miscellaneous operations like reading the battery, errors and human I/O.
// J. Libonatti

//-------------------Battery-----------------------------//

#include <driver/adc.h> //Driver for adc

#define R1 9.8          // 1st divider resistor kOhm
#define R2 9.58         // 2nd divider resistor kOhm
#define SAMPLES 8       // Samples at each reading
#define OFFSET 0.7   // For some reason the analog reading comes with a constant voltage offset
const float vRefScale = (3.3 / 4096.0) * ((R1 + R2) / R2); // Scale from analogread raw output to vBat

int SetBattery(){                                             // Starts battery sensor and checks battery voltage
  pinMode(37, INPUT);                                         // Input mode as we are going to read only
  adc1_config_width(ADC_WIDTH_12Bit);                         // set ADC resolution to max
  adc1_config_channel_atten(ADC1_CHANNEL_1, ADC_ATTEN_DB_11); // set ADC attenuation (allows higher voltage input)
  int bat = 0;                                                // Reads the battery level 8 times (level in %)
  for(int i = 0; i<7; i++)                    
    bat += ReadBattery();
  bat /= 8;                                                   // Takes the mean value
  if(bat < -5 || bat > 105)                                   // If battery level is invalid
    return -51;                                               // Return error
  if(bat < 20)                                                // If battery level is too low
    return -52;                                               // Return error
  return 0;
}

int ReadBattery(){                                      // Reads Battery level from 0 to 100%
  uint32_t reading = 0;
  for(int i = 0; i < SAMPLES; i++)                      // gets the defined number of samples
    reading += adc1_get_raw(ADC1_CHANNEL_1);            // sum the values of all samples
  reading /= SAMPLES;                                   // divides by the number of samples to get the average
  float voltage = reading * vRefScale + OFFSET;         // converts from raw to battery voltage
  return (int) ((voltage - 3.5)/(4.2 - 3.5) * 100.0);   // converts from battery voltage to power level
}

//--------------------LED and Buzzer------------------//

void SetLED(){              // Starts LED                              
  pinMode(32, OUTPUT);      // Pin on output for led
  digitalWrite(32, HIGH);   // Leds starts on
}

void LEDOn(){               // Turns LED on
  digitalWrite(32, HIGH);
}

void LEDOff(){              // Turns LED off
  digitalWrite(32, LOW);  
}

void SetBuzzer(){           // Starts Buzzer
  pinMode(33, OUTPUT);      // Pin on output for buzzer
}

void BuzzerTone(int t){     // Rings the buzzer for t milliseconds
  digitalWrite(33,HIGH);
  vTaskDelay(t / portTICK_PERIOD_MS);
  digitalWrite(33,LOW);
}

void BuzzerOn(int t){       // Turns Buzzer on
  digitalWrite(33,HIGH);
}

void BuzzerOff(int t){      // Turns Buzzer off
  digitalWrite(33,LOW);
}

//---------------Error--------------//

void initErrorHandler(int *errorCode, uint8_t sz, bool blockExecution){  // Deals with errors in setup

#ifdef SERIAL_DEBUG                   // if switch is defined
  for(int i = 0; i<sz; i++){          //Loops through each return Value
    switch(errorCode[i]){                  // Messages for each error
      case 0:   //Not an error
          break;
      case -1:  //Slot for General Error 1
          break;
      case -11:
          Serial.print("MPU6050 Error 1: Unable to Reset");
          break;
      case -12:
          Serial.print("MPU6050 Error 2: Unable to Reset Sensors");
          break;
      case -13:
          Serial.print("MPU6050 Error 3: Unable to Reset Sensors");
          break;
      case -14:
          Serial.print("MPU6050 Error 4: Unable to Set to no Sleep Mode");
          break;
      case -15:
          Serial.print("MPU6050 Error 5: Unable to Config Gyro Range");
          break;
      case -16:
          Serial.print("MPU6050 Error 6: Unable to Set 1st byte to acess HML");
          break;
      case -17:
          Serial.print("MPU6050 Error 7: Unable to Set 2nd byte to acess HML");
          break;
      case -18:
          Serial.print("MPU6050 Error 8: MPU Froze");
          break;
      case -21:
          Serial.print("QMC5883L Error 1: Unable to Reset");
          break;
      case -22:
          Serial.print("QMC5883L Error 2: Unable to Set Reset Period");
          break;
      case -23:
          Serial.print("QMC5883L Error 3: Unable to Config");
          break;
      case -31:
          Serial.print("BMP180 Error 1: Unable to Reset");
          break;
      case -32:
          Serial.print("BMP180 Error 2: Unable to Identify");
          break;
      case -41:
          Serial.print("GPS Error 1: Unable to Config");
          break;
      case -51:
          Serial.print("BAT Error 1: Invalid Reading");
          break;
      case -52:
          Serial.print("BAT Error 2: Low Battery");
          break;
      case -61:
          Serial.print("SD Error 1: Unable to Init SD Card");
          break;
      case -62:
          Serial.print("SD Error 2: Unable to Open GY SD File");
          break;
      case -63: 
          Serial.print("SD Error 3: Unable to Open GPS SD File");
          break;
      case -64:
          Serial.print("SD Error 4: Unable to Open Geiger SD File");
          break;
      case -65:
          Serial.print("SD Error 5: Unable to Open Battery SD File");
          break;
      case -66:
          Serial.print("SD Error 6: Unable to Open Log SD File");
          break;
      case -71:
          Serial.print("Wifi Error 1: Unable to connect to wifi");
          break;
      default:
    }
  }
#endif

  uint8_t errors = 0;
  for(int i = 0; i<sz; i++)      //Loops through each return Value
    if(errorCode[i] != 0)        //Checking for errors
      errors++;

  if(blockExecution && errors != 0){
    while(1)                      // Locks the cubesat on alert message
        for(int i = 0; i<sz; i++)      //Loops through each return Value
          if(errorCode[i] != 0)
            alertMessage(-errorCode[i]);   // Sends an alert message with the error code
  }
}


void alertMessage(unsigned int msg){  // Sends the error code on an alert message on the buzzer and LED. USE ONLY ON SETUP AS IT USES DELAY
  if(msg/10 > 10)                     // Checks for invalid error code
    return;

  int msgComp = msg/10;               // Gets the component number
  int msgError = msg%10;              // Gets the error number

  //sad and mischievous failure tone//
  BuzzerTone(50); 
  vTaskDelay(50);
  BuzzerTone(50);
  vTaskDelay(50);
  BuzzerTone(50); 

  //also turns led on//
  LEDOn();
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  //component tone: sends component number in beeps and blinks//
  for(int i = 0; i<msgComp; i++){
      LEDOff();
      BuzzerTone(100);
      LEDOn();
      vTaskDelay(500 / portTICK_PERIOD_MS);
  }
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  LEDOff();

  //error tone: sends error number in beeps and blinks//
  for(int i = 0; i<msgError; i++){
      LEDOff();
      BuzzerTone(100);
      LEDOn();
      vTaskDelay(500/ portTICK_PERIOD_MS);
  }

  //finishes execution with a delay of 5s on LED//
  LEDOn();
  vTaskDelay(5000 / portTICK_PERIOD_MS);
  LEDOff();
}
