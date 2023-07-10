//-------------------Battery-----------------------------//

#define R1 11.0//kOhm
#define R2 9.64 //kOhm
#define SAMPLES 8 //Samples at each reading

float vRefScale = (3.3f / 4096.0f) * ((R1 + R2) / R2);

int SetBattery(){
  pinMode(37, INPUT);
  return 0;
}

int ReadBattery(){
  float reading = 0.0;
  for(int i = 0; i < SAMPLES; i++)
  {
    reading += analogRead(37);
  }
  reading /= SAMPLES;
  float voltage = reading * vRefScale;
  return (int) ((voltage - 3.5)/(4.2 - 3.5) * 100.0);
}

//--------------------LED and Buzzer------------------//

void SetLED(){
  pinMode(32, OUTPUT);
  digitalWrite(32, HIGH);
}

void LEDOn(){
  digitalWrite(32, HIGH);
}

void LEDOff(){
  digitalWrite(32, LOW);  
}

void SetBuzzer(){
  pinMode(33, OUTPUT);
}

void BuzzerTone(int t){
  digitalWrite(33,HIGH);
  delay(t);
  digitalWrite(33,LOW);
}

//---------------Error--------------//

void errorHandler(int errorCode){

  switch(errorCode){
    case 0:
      return;
    case -1: //General Error 1:
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
    default:
      return;
  }
  while(1)
  {
    alertMessage(-errorCode);
  }
}

void alertMessage(unsigned int msg){
  if(msg/10 > 10)
    return;

  int msgSensor = msg/10;
  int msgError = msg%10;
  Serial.println(msgError);


 //failure tone//
  BuzzerTone(50); 
  delay(50);
  BuzzerTone(50);
  delay(50);
  BuzzerTone(50); 

  delay(1000);

  //sensor tone//
  LEDOn();
  for(int i = 0; i<msgSensor; i++){
      BuzzerTone(100);
      delay(500);
  }
  delay(2000);
  LEDOff();

  //error tone//
  for(int i = 0; i<msgError; i++){
      BuzzerTone(100);
      delay(500);
  }
  delay(5000);
}


//--------------------Calibration------------------//

void HMCCalib(){
  int i=0;
  float xmax = -1000,xmin = 1000,ymax = -1000,ymin = 1000,zmax = -1000,zmin = 1000;
  while(1)
  {
    ReadGY87(&GY87);
    float x = GY87.Mag[0];
    float y = GY87.Mag[1];
    float z = GY87.Mag[2];
    
    if(x > xmax)
      xmax = x;
    if(y > ymax)
      ymax = y;
    if(z > zmax)
      zmax = z;
    if(x < xmin)
      xmin = x;
    if(y < ymin)
      ymin = y;
    if(z < zmin)
      zmin = z;

    i++;
    if(i == 500)
    {
      Serial.print("Calib coef (xmax,xmin,ymax,ymin,zmax,zmin) = (");
      Serial.print(xmax);
      Serial.print(",");
      Serial.print(xmin);
      Serial.print(",");
      Serial.print(ymax);
      Serial.print(",");
      Serial.print(ymin);
      Serial.print(",");
      Serial.print(zmax);
      Serial.print(",");
      Serial.print(zmin);
      Serial.println(")");
      i = 0;
    }
    delay(10);
  }
}
