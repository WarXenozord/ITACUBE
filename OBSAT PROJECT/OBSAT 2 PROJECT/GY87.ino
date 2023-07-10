// This module formats handles the communication with the Gy-87 multisensor board. Very low level stuff here.
// J. Libonatti

#include <Wire.h>

#define MPU 0x68 // MPU6050 I2C address
#define BMP 0x77 // BMP180 I2C address
#define QMC 0x0D // QMC5883L I2C address

//BMP Resolution and Chip ID
#define BMP180_CHIP_ID 0x55
#define BMP_RES 0xF4 // Low 0x34, Standard 0x74, High 0xB4, UHigh 0xF4 /!\ Do change sample delay!
#define Res_coef 3// 0 low,1 STD,2 H,3 UH

//QMC5883L Start Registers
#define Mode_Standby    0b00000000
#define Mode_Continuous 0b00000001

#define ODR_10Hz        0b00000000
#define ODR_50Hz        0b00000100
#define ODR_100Hz       0b00001000
#define ODR_200Hz       0b00001100

#define RNG_2G          0b00000000
#define RNG_8G          0b00010000

#define OSR_512         0b00000000
#define OSR_256         0b01000000
#define OSR_128         0b10000000
#define OSR_64          0b11000000

float mgPerDigit = 2 / 32.768;

enum : uint8_t
{
  BMP180_CAL_AC1_REG =                0xAA,  //ac1 pressure    computation
  BMP180_CAL_AC2_REG =                0xAC,  //ac2 pressure    computation
  BMP180_CAL_AC3_REG =                0xAE,  //ac3 pressure    computation
  BMP180_CAL_AC4_REG =                0xB0,  //ac4 pressure    computation
  BMP180_CAL_AC5_REG =                0xB2,  //ac5 temperature computation
  BMP180_CAL_AC6_REG =                0xB4,  //ac6 temperature computation
  BMP180_CAL_B1_REG  =                0xB6,  //b1  pressure    computation
  BMP180_CAL_B2_REG  =                0xB8,  //b2  pressure    computation
  BMP180_CAL_MB_REG  =                0xBA,  //mb
  BMP180_CAL_MC_REG  =                0xBC,  //mc  temperature computation
  BMP180_CAL_MD_REG  =                0xBE   //md  temperature computation
}
BMP180_CAL_REG;

struct
{
  int16_t  bmpAC1 = 0;
  int16_t  bmpAC2 = 0;
  int16_t  bmpAC3 = 0;
  uint16_t bmpAC4 = 0;
  uint16_t bmpAC5 = 0;
  uint16_t bmpAC6 = 0;

  int16_t  bmpB1  = 0;
  int16_t  bmpB2  = 0;

  int16_t  bmpMB  = 0;
  int16_t  bmpMC  = 0;
  int16_t  bmpMD  = 0;
}
BMPCalCoef; // Stores calibration coeficients

int SetGY87() {
  Wire.begin();                      // Initialize comunication
  
  // === MPU 6050 Startup === //
  Wire.beginTransmission(MPU);       // Start communication with MPU6050 // MPU=0x68
  Wire.write(0x6B);                  // Talk to the register 6B
  Wire.write(0x08);                  // Make reset - place a 0 into the 6B register
  if(Wire.endTransmission(true)){    // End the transmission
    return -11;
  }        
  delay(100);
  // Configure Accelerometer Sensitivity - Full Scale Range (default +/- 2g)
  Wire.beginTransmission(MPU);
  Wire.write(0x1C);                  //Talk to the ACCEL_CONFIG register (1C hex)
  Wire.write(0x10);                  //Set the register bits as 00010000 (+/- 8g full scale range)
  if(Wire.endTransmission(true)){
    return -12;
  }     
  delay(5);
  // Configure Gyro Sensitivity - Full Scale Range (default +/- 250deg/s)
  Wire.beginTransmission(MPU);
  Wire.write(0x1B);                   // Talk to the GYRO_CONFIG register (1B hex)
  Wire.write(0x10);                   // Set the register bits as 00010000 (1000deg/s full scale)
  if(Wire.endTransmission(true)){
    return -13;
  } 
  delay(5);
  //Pass-trough mode to access QMC5883L
  Wire.beginTransmission(MPU);
  Wire.write(0x37);
  Wire.write(0x02);
  if(Wire.endTransmission(true)){
    return -14;
  } 
  Wire.beginTransmission(MPU);
  Wire.write(0x6A);
  Wire.write(0x00);
  if(Wire.endTransmission(true)){
    return -15;
  } 
  delay(5);

  // === QMC5883L === //
  //Reset
  Wire.beginTransmission(QMC);
  Wire.write(0x0A);
  Wire.write(0x80);
  if(Wire.endTransmission())
  {
    return -21;
  }
  delay(5);
  //Define Set/Reset period
  Wire.beginTransmission(QMC);
  Wire.write(0x0B);
  Wire.write(0x01);
  if(Wire.endTransmission())
  {
    return -22;
  }
  delay(5);
  //Sets Config
  Wire.beginTransmission(QMC);
  Wire.write(0x09);
  Wire.write(Mode_Continuous|OSR_512|RNG_2G|ODR_10Hz); //Configs
  if(Wire.endTransmission())
  {
    return -23;
  }
  
  // === BMP 180 Startup === //
  Wire.beginTransmission(BMP);       // Start communication with BMP
  Wire.write(0xE0);                  // Talk to the register E0
  Wire.write(0xB6);                  // Make reset - place a B6 into the E0 register
  if(Wire.endTransmission(true)){
    return -31;
  }      // End the transmission
  delay(5);
  Wire.beginTransmission(BMP);       // Checks Chip Id
  Wire.write(0xD0);
  Wire.endTransmission(false);
  Wire.requestFrom(BMP,1,true);
  if (Wire.read() != BMP180_CHIP_ID)
  {
    return -32;
  }

  getCalCoef(); // Gets calibration coeficients
  delay(20);
}

void ReadGY87(DataGY87 *GY87) {
  // === Read acceleromter data === //
  Wire.beginTransmission(MPU);
  Wire.write(0x3B); // Start with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true); // Read 6 registers total, each axis value is stored in 2 registers
  
  //For a range of +-8g, we need to divide the raw values by 16384 / 4, according to the datasheet
  GY87->Acc[0] = (int16_t (Wire.read() << 8 | Wire.read())) / 16384.0 * 4; // X-axis value
  GY87->Acc[1] = (int16_t (Wire.read() << 8 | Wire.read())) / 16384.0 * 4; // Y-axis value
  GY87->Acc[2]  = (int16_t (Wire.read() << 8 | Wire.read())) / 16384.0 * 4; // Z-axis value

  // === Read gyroscope data === //
  Wire.beginTransmission(MPU);
  Wire.write(0x43); // Gyro data first register address 0x43
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true); // Read 6 registers total, each axis value is stored in 2 registers
  GY87->Gyro[0] = (int16_t (Wire.read() << 8 | Wire.read())) / 131.0 * 4; // For a 250deg/s range we have to divide first the raw value by 131.0, according to the datasheet
  GY87->Gyro[1] = (int16_t (Wire.read() << 8 | Wire.read())) / 131.0 * 4;
  GY87->Gyro[2] = (int16_t (Wire.read() << 8 | Wire.read())) / 131.0 * 4;

  // === Read Magnetometer data === //
   Wire.beginTransmission(QMC);
   Wire.write(0x00);
   Wire.endTransmission(false);
   Wire.requestFrom(QMC, 6, true); // Read 6 registers total, each axis value is stored in 2 registers
   GY87->Mag[0] = int16_t (Wire.read() | Wire.read() << 8) * mgPerDigit;
   GY87->Mag[1] = int16_t (Wire.read() | Wire.read() << 8) * mgPerDigit;
   GY87->Mag[2] = int16_t (Wire.read() | Wire.read() << 8) * mgPerDigit;
  // === Read barometer data === //
  
  //Temperature
  Wire.beginTransmission(BMP);
  Wire.write(0xF4);
  Wire.write(0x2E);
  Wire.endTransmission(true);
  delay(5);
  Wire.beginTransmission(BMP);
  Wire.write(0xF6);
  Wire.endTransmission(true);
  Wire.requestFrom(BMP, 2, true);
  int16_t rawTemp;
  rawTemp = int16_t (Wire.read() << 8 | Wire.read());
  int32_t B5 = computeB5(rawTemp);
  GY87->Temp = ((B5 + 8) >> 4) / 10;
  
  //Pressure
  Wire.beginTransmission(BMP);
  Wire.write(0xF4);
  Wire.write(0xF4);
  Wire.endTransmission(true);
  delay(26); //Sample delay Low 5, Standard 8, High 14, UHigh 26
  Wire.beginTransmission(BMP);
  Wire.write(0xF6);
  Wire.endTransmission(true);
  Wire.requestFrom(BMP, 3, true);
  uint32_t rawPres;
  rawPres = Wire.read() << 8 | Wire.read();
  rawPres <<= 8;
  rawPres |= Wire.read();       //19-bits
  rawPres >>= (8 - Res_coef);          // 8 - (0 low, 1 std, 2 high, 3 UH)
  GY87->Pres = processPres(rawPres, B5);

  // Print the values on the serial monitor
  /*Serial.print("Acceleration: ");
  Serial.print(GY87->Acc[0]);
  Serial.print("/");
  Serial.print(GY87->Acc[1]);
  Serial.print("/");
  Serial.println(GY87->Acc[2]);
  Serial.print("Gyro: ");
  Serial.print(GY87->Gyro[0]);
  Serial.print("/");
  Serial.print(GY87->Gyro[1]);
  Serial.print("/");
  Serial.println(GY87->Gyro[2]);
  Serial.print("Magnetometer: ");
  Serial.print(GY87->Mag[0]);
  Serial.print("/");
  Serial.print(GY87->Mag[1]);
  Serial.print("/");
  Serial.println(GY87->Mag[2]);
  Serial.print("Temperature: ");
  Serial.println(GY87->Temp);
  Serial.print("Pressure: ");
  Serial.println(GY87->Pres);*/
}

void getCalCoef()
{
  int32_t value = 0;

  for (uint8_t reg = BMP180_CAL_AC1_REG; reg <= BMP180_CAL_MD_REG; reg++)
  {
    Wire.beginTransmission(BMP);
    Wire.write(reg);
    Wire.endTransmission(true);
    Wire.requestFrom(BMP, 2, true);
    value = int16_t (Wire.read() << 8 | Wire.read());
    
    switch (reg)
    {
      case BMP180_CAL_AC1_REG:               //used for pressure computation
        BMPCalCoef.bmpAC1 = value;
        break;

      case BMP180_CAL_AC2_REG:               //used for pressure computation
        BMPCalCoef.bmpAC2 = value;
        break;

      case BMP180_CAL_AC3_REG:               //used for pressure computation
        BMPCalCoef.bmpAC3 = value;
        break;

      case BMP180_CAL_AC4_REG:               //used for pressure computation
        BMPCalCoef.bmpAC4 = value;
        break;

      case BMP180_CAL_AC5_REG:               //used for temperature computation
        BMPCalCoef.bmpAC5 = value;
        break;

      case BMP180_CAL_AC6_REG:               //used for temperature computation
        BMPCalCoef.bmpAC6 = value;
        break;

      case BMP180_CAL_B1_REG:                //used for pressure computation
        BMPCalCoef.bmpB1 = value;
        break;

      case BMP180_CAL_B2_REG:                //used for pressure computation
        BMPCalCoef.bmpB2 = value;
        break;

      case BMP180_CAL_MB_REG:                //???
        BMPCalCoef.bmpMB = value;
        break;

      case BMP180_CAL_MC_REG:                //used for temperature computation
        BMPCalCoef.bmpMC = value;
        break;

      case BMP180_CAL_MD_REG:                //used for temperature computation
        BMPCalCoef.bmpMD = value;
        break;
    }
  }
}

int32_t computeB5(int32_t T)
{
  int32_t X1 = ((T - (int32_t)BMPCalCoef.bmpAC6) * (int32_t)BMPCalCoef.bmpAC5) >> 15;
  int32_t X2 = ((int32_t)BMPCalCoef.bmpMC << 11) / (X1 + (int32_t)BMPCalCoef.bmpMD);

  return X1 + X2;
}

int32_t processPres(uint32_t UP,int32_t B5)
{
  int32_t  B3       = 0;
  int32_t  B6       = 0;
  int32_t  X1       = 0;
  int32_t  X2       = 0;
  int32_t  X3       = 0;
  int32_t  pressure = 0;
  uint32_t B4       = 0;
  uint32_t B7       = 0;

  /* pressure calculation */
  B6 = B5 - 4000;
  X1 = ((int32_t)BMPCalCoef.bmpB2 * ((B6 * B6) >> 12)) >> 11;
  X2 = ((int32_t)BMPCalCoef.bmpAC2 * B6) >> 11;
  X3 = X1 + X2;
  B3 = ((((int32_t)BMPCalCoef.bmpAC1 * 4 + X3) << Res_coef) + 2) / 4; // 

  X1 = ((int32_t)BMPCalCoef.bmpAC3 * B6) >> 13;
  X2 = ((int32_t)BMPCalCoef.bmpB1 * ((B6 * B6) >> 12)) >> 16;
  X3 = ((X1 + X2) + 2) >> 2;
  B4 = ((uint32_t)BMPCalCoef.bmpAC4 * (X3 + 32768L)) >> 15;
  B7 = (UP - B3) * (50000UL >> Res_coef);
  
  if (B4 == 0) return 0;                                     //safety check, avoiding division by zero

  if   (B7 < 0x80000000) pressure = (B7 * 2) / B4;
  else                   pressure = (B7 / B4) * 2;

  X1 = pow((pressure >> 8), 2);
  X1 = (X1 * 3038L) >> 16;
  X2 = (-7357L * pressure) >> 16;

  return pressure = pressure + ((X1 + X2 + 3791L) >> 4);  
}

void CorrectMag(DataGY87 *GY87){
  GY87->Mag[0] = (GY87->Mag[0] - MagXCalib)*MagXCalib2;
  GY87->Mag[1] = (GY87->Mag[1] - MagYCalib)*MagYCalib2;
  GY87->Mag[2] = (GY87->Mag[2] - MagZCalib)*MagZCalib2;
}
