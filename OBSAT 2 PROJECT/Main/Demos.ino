//Variety of non-operational code for demonstration purposes
//J. Libonatti
#include "Curie.h"
#ifdef DEMOS

void DemoHandler(){                                 //Handles the various Demos according to the format
  uint8_t buffer[8];                                //Demos:2>DemNam1=parame1,parame2,parame3,--end-->DemNam2=parame1,--end--
  ReadSD(DEMO_FILE, buffer, 8, 0);

  int numberOfDemos;                                
  int pos = 1;
  sscanf((char *) buffer,"Demos:%d",&numberOfDemos);//Gets the number of demos from the buffer
  for(int i = 1; i <= numberOfDemos; i++){
    ReadSD(DEMO_FILE, buffer, 8, 8*pos);
    if(buffer[0] = 'M')                             //"Musicas" Demo: Play songs on buzzer
      pos = Orchestra(pos) + 1;
  }
}

//Orchestra Demo
int Orchestra(int initialPos){                     // Play all musics on the demo parameters.
  int i = initialPos + 1;                          // The parameters are the song archive names.
  uint8_t song[8];
  ReadSD(DEMO_FILE, song, 8, 8*i);
  while(song[0] != '-')                            // Parse all the parameters, playing each
  {
    int *melody;
    int *durations;
    int size = ReadMelody(String((char* )song), &melody, &durations); //Retrieves the melody and durations array from file
    if(size){
      play(size, melody, durations);    //Plays the songs
      free(melody);                     //Frees the allocated memory
      free(durations);
    }
    delay(4000);
    i++;
    ReadSD(DEMO_FILE, song, 8, 8*i);
  }
  return i;
}

#define BUZZER_PIN 33

void play(int size, int* melody, int* durations){   //Plays the songs on buzzer
  for (int note = 0; note < size; note++) {
    //to calculate the note duration, take one second divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int duration = 1000 / durations[note];
    tone(BUZZER_PIN, melody[note], duration);
    //to distinguish the notes, set a minimum time between them.
    //the note's duration + 30% seems to work well:
    int pauseBetweenNotes = duration * 1.30;
    delay(pauseBetweenNotes);
    
    //stop the tone playing:
    noTone(BUZZER_PIN);
  }
}
#endif

