#include <MPR121.h>
#include <stdio.h>
#include <wiringPi.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <iostream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#define NUM_ELECTRODES 12
#define RED_LED_PIN 22
#define GREEN_LED_PIN 21
#define BLUE_LED_PIN 25

using namespace std;

int main(int argc, char* argv[]) {

  cout << "Welcome to Sampler" << endl;
  
  cout << "WiringPiSetup" << endl;
  wiringPiSetup();
  
    cout << "PiCap Setup" << endl;
  // setup MPR121 address on the Pi Cap
  if (!MPR121.begin(0x5C)) {
    cout << "error setting up MPR121: ";

    switch (MPR121.getError()) {
      case NO_ERROR:
        cout << "no error" << endl;
        break;
      case ADDRESS_UNKNOWN:
        cout << "incorrect address" << endl;
        break;
      case READBACK_FAIL:
        cout << "readback failure" << endl;
        break;
      case OVERCURRENT_FLAG:
        cout << "overcurrent on REXT pin" << endl;
        break;
      case OUT_OF_RANGE:
        cout << "electrode out of range" << endl;
        break;
      case NOT_INITED:
        cout << "not initialised" << endl;
        break;
      default:
        cout << "unknown error" << endl;
        break;
    }

    exit(1);
  }
  
   cout << "Audio Setup" << endl;
  // initialize SDL audio
  SDL_Init(SDL_INIT_AUDIO);

  // initialize SDL mixer
  Mix_Init(MIX_INIT_MP3);

  // open SDL audio - sampling rate, format, channels, chunk size
  Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);
  
  Mix_Chunk* sample = Mix_LoadWAV("SAMPLES/1.wav");

  while(1)
  {
    MPR121.updateBaselineData();
    MPR121.updateFilteredData();
    
    for (int a = 0; a < NUM_ELECTRODES; a++) 
        {
            if(MPR121.getFilteredData(a)<500)
            {
              Mix_PlayChannel(-1, sample, 0);
            }
          }
    delay(10);
  }
  
  
  
}
