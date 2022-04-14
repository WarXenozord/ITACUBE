#include "PION_System.h"

/* 
  Esse código demonstra como interagir com os LEDs RGB presentes na placa de interface do seu kit
*/

System canSat;

void setup(){
  // Inicializa seu CanSat, e seus periféricos 
  canSat.init();
}

void loop(){
  // Ativa o RGB
  canSat.setRGB(BLUE);
  delay(1000);
  // Desativa o RGB
  canSat.setRGB(OFF);
  delay(1000);
}