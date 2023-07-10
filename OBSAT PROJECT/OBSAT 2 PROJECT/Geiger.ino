#include "Curie.h"

#define COUNT_TIME 10000
#define GEIGER_PIN 36

hw_timer_t * Timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

volatile unsigned int counts = 0;
volatile unsigned int totalCounts = 0;
volatile unsigned long lastTf = 0;


void IRAM_ATTR onTime() {
	portENTER_CRITICAL_ISR(&timerMux);
    totalCounts = counts;
    lastTf = millis();
    counts = 0;
	portEXIT_CRITICAL_ISR(&timerMux);
}

void countInterrupt(){ //adds counts on interruption
  counts++;
}

int SetGeiger(){
  pinMode(GEIGER_PIN, INPUT);
  attachInterrupt(GEIGER_PIN, countInterrupt, FALLING);
  // Configure Prescaler to 80, as our timer runs @ 80Mhz
	// Giving an output of 80,000,000 / 80 = 1,000,000 ticks / second
	Timer = timerBegin(0, 80, true);                
	timerAttachInterrupt(Timer, &onTime, true);    
	// Fire Interrupt every 1m ticks, so 1s
	timerAlarmWrite(Timer, 10000000, true);			
	timerAlarmEnable(Timer);
  return 0;
}

void ReadGeiger(DataGeiger *Geiger){
  Geiger->counts = totalCounts;
  Geiger->tf = lastTf;
}
