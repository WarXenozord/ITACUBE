// This module formats handles miscellaneous operations like reading the battery, errors and human I/O.
// J. Libonatti

//-------------------Battery-----------------------------//

#include <driver/adc.h> //Driver for adc

#define R1 9.8          // 1st divider resistor kOhm
#define R2 9.58         // 2nd divider resistor kOhm
#define SAMPLES 8       // Samples at each reading
const float vRefScale = (3.3 / 4096.0) * ((R1 + R2) / R2); // Scale from analogread raw output to vBat

int SetBattery(){                                             // Starts battery sensor and checks battery voltage
  pinMode(37, INPUT);                                         // Input mode as we are going to read only
  adc1_config_width(ADC_WIDTH_12Bit);                         // set ADC resolution to max
  adc1_config_channel_atten(ADC1_CHANNEL_1, ADC_ATTEN_DB_11); // set ADC attenuation (allows higher voltage input)
  int bat =  ReadBattery();                                   // Reads the battery level once (level in %)
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
  float voltage = reading * vRefScale;                  // converts from raw to battery voltage
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

void BuzzerTone(int t){     // Rings the buzzer for t milliseconds (USE ONLY ON SETUP AS IT USES DELAY)
  digitalWrite(33,HIGH);
  delay(t);
  digitalWrite(33,LOW);
}

void BuzzerOn(int t){       // Turns Buzzer on
  digitalWrite(33,HIGH);
}

void BuzzerOff(int t){      // Turns Buzzer off
  digitalWrite(33,LOW);
}

//---------------Error--------------//

void errorHandler(int errorCode){  // Deals with errors in setup

  switch(errorCode){               // Actions for each error
    case 0:   //Not an error
      return;
    case -1:  //General Error 1:
    case -11: //MPU6050 Error 1: Unable to Reset
    case -12: //MPU6050 Error 2: Unable to Config Accel Range
    case -13: //MPU6050 Error 3: Unable to Config Gyro Range
    case -14: //MPU6050 Error 4: Unable to Set 1st byte to acess HML
    case -15: //MPU6050 Error 5: Unable to Set 2nd byte to acess HML
    case -21: //QMC5883L Error 1: Unable to Reset
    case -22: //QMC5883L Error 2: Unable to Set Reset Period
    case -23: //QMC5883L Error 3: Unable to Config
    case -31: //BMP180 Error 1: Unable to Reset
    case -32: //BMP180 Error 2: Unable to Identify
    case -41: //GPS Error 1: Unable to Config
    case -51: //BAT Error 1: Invalid Reading
    case -52: //BAT Error 2: Low Battery
    case -61: //SD Error 1: Unable to Init SD Card
    case -62: //SD Error 2: Unable to Open SD File
    case -71: //Wifi Error 1: Unable to connect to wifi
      break;
    default:
      return;
  }
  while(1)                      // Locks the cubesat on alert message
  {
    alertMessage(-errorCode);   // Sends an alert message with the error code
  }
}

void alertMessage(unsigned int msg){  // Sends the error code on an alert message on the buzzer and LED. USE ONLY ON SETUP AS IT USES DELAY
  if(msg/10 > 10)                     // Checks for invalid error code
    return;

  int msgComp = msg/10;               // Gets the component number
  int msgError = msg%10;              // Gets the error number
  
#ifdef SERIAL_DEBUG                   // if switch is defined
  Serial.print(msgComp);              // prints error on serial
  Serial.print("-> error ");
  Serial.println(msgError);
#endif

  //sad and mischievous failure tone//
  BuzzerTone(50); 
  delay(50);
  BuzzerTone(50);
  delay(50);
  BuzzerTone(50); 

  //also turns led on//
  LEDOn();
  delay(1000);

  //component tone: sends component number in beeps and blinks//
  for(int i = 0; i<msgComp; i++){
      LEDOff();
      BuzzerTone(100);
      LEDOn();
      delay(500);
  }
  delay(2000);
  LEDOff();

  //error tone: sends error number in beeps and blinks//
  for(int i = 0; i<msgError; i++){
      LEDOff();
      BuzzerTone(100);
      LEDOn();
      delay(500);
  }

  //finishes execution with a delay of 5s on LED//
  LEDOn();
  delay(5000);
  LEDOff();
}
