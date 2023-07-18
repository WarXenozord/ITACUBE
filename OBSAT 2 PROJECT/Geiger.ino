//This module handles Geiger Counter Code With Interrupts
//J. Libonatti

#include "Curie.h"

#define GEIGER_PIN 36               // Connect to GEIGER INT2 Pin

volatile unsigned int counts = 0;  // Volatile as this variable is changed in interrupts
struct timeval tv_now;             // Stores RTC timeofday
unsigned long t0;                  // Stores t0


void countInterrupt(){              // Adds to the geiger particle counts on interruption
  counts++;
}

int SetGeiger(){                                                        //  Initializes Geiger
  pinMode(GEIGER_PIN, INPUT);                                           // Sets Geiger Pin to input mode
  attachInterrupt(GEIGER_PIN, countInterrupt, FALLING);                 // Everytime the pin voltage falls, which happens when geiger
  gettimeofday(&tv_now, NULL);                                          // Reads Time from ESP32 RTC
  t0 = (unsigned long)(tv_now.tv_sec * 1000000 + tv_now.tv_usec)/1000;  // Gets t0
  return 0;                                                             // Counts a particle, the function countInterrupt will be called
}

void ReadGeiger(DataGeiger *Geiger){
  gettimeofday(&tv_now, NULL);
  Geiger->tf = (unsigned long)((tv_now.tv_sec * 1000000 + tv_now.tv_usec)/1000 - t0);     //Geiger tf from internal RTC
  Geiger->counts = counts;                                                                //Geiger counts from interrupts
  counts -= Geiger->counts;         //If an interrupt happens in this brief time, the counted particle won't be discarded
}
