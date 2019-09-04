#include <MPR121.h>
#include <wiringPi.h>
#include <signal.h>
#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#define NUM_ELECTRODES 12

#define RED_LED_PIN 22
#define GREEN_LED_PIN 21
#define BLUE_LED_PIN 25

using namespace std;


int i;
string path;

// "volatile" means - I can be modified from elsewhere
bool volatile keepRunning = true;

void lightRGB(int r, int g, int b) {
  // we are inverting the values, because the LED is active LOW
  // LOW - on
  // HIGH - off
  digitalWrite(RED_LED_PIN, !r);
  digitalWrite(GREEN_LED_PIN, !g);
  digitalWrite(BLUE_LED_PIN, !b);
}

// this allows us to exit the program via Ctrl+C while still exiting elegantly
void intHandler(int dummy) {
  keepRunning = false;
  lightRGB(0, 0, 0);
  exit(0);
}

int main(void) {
  // wait for Ctrl+C
  signal(SIGINT, intHandler);

  // default MPR121 address on the Pi Cap
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

  // this is the touch threshold - setting it low makes it more like a proximity trigger
  // default value is 40 for touch
  int touchThreshold = 40;

  // this is the release threshold - must ALWAYS be smaller than the touch threshold
  // default value is 20 for touch
  int releaseThreshold = 20;

  MPR121.setTouchThreshold(touchThreshold);
  MPR121.setReleaseThreshold(releaseThreshold);

  // set up LED
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);



  // initialize SDL audio
  SDL_Init(SDL_INIT_AUDIO);

  // initialize SDL mixer
  Mix_Init(MIX_INIT_MP3);

  // open SDL audio - sampling rate, format, channels, chunk size
  Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);

  Mix_Chunk* sample[NUM_ELECTRODES];
  
  cout << "Load Samples" << endl;
  
  i = 1;
  for (int a = 0; a < NUM_ELECTRODES; a++) 
        {
        path ="SAMPLES/";
        path += to_string(i);
        path += ".wav";
        cout << "loading file: " << path.c_str() << endl;
        sample[a] = Mix_LoadWAV(path.c_str());
        i++;
        }
  

  while (keepRunning) {
   // if (MPR121.touchStatusChanged()) {
    MPR121.updateTouchData();
      
    //MPR121.updateBaselineData();
    MPR121.updateFilteredData();
    
    for (int a = 0; a < NUM_ELECTRODES; a++) 
        {
            if(MPR121.getFilteredData(a)<500)
            {
             Mix_PlayChannel(-1, sample[a], 0);
            }
          }
  

      bool isAnyTouchRegistered = false;

      if (isAnyTouchRegistered) {
        lightRGB(1, 0, 0);
      }
      else {
        lightRGB(0, 0, 0);
      }
  
    delay(100);
  }

  // quit SDL mixer
  Mix_Quit();

  return 0;
}

/*
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

int i;
string path;

void lightRGB(int r, int g, int b) {
  // we are inverting the values, because the LED is active LOW
  // LOW - on
  // HIGH - off
  digitalWrite(RED_LED_PIN, !r);
  digitalWrite(GREEN_LED_PIN, !g);
  digitalWrite(BLUE_LED_PIN, !b);
}


int main(int argc, char* argv[]) {
  
  // set up LED
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);

  cout << "Welcome to Sampler" << endl;
  
  cout << "WiringPiSetup" << endl;
    
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
  
  
    // this is the touch threshold - setting it low makes it more like a proximity trigger
  // default value is 40 for touch
  int touchThreshold = 40;

  // this is the release threshold - must ALWAYS be smaller than the touch threshold
  // default value is 20 for touch
  int releaseThreshold = 20;

  MPR121.setTouchThreshold(touchThreshold);
  MPR121.setReleaseThreshold(releaseThreshold);
  
   cout << "Audio Setup" << endl;
  // initialize SDL audio
  SDL_Init(SDL_INIT_AUDIO);

  // initialize SDL mixer
  Mix_Init(MIX_INIT_MP3);

  // open SDL audio - sampling rate, format, channels, chunk size
  Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);
  
 // Mix_Chunk* sample[NUM_ELECTRODES];
  
  cout << "Load Samples" << endl;
  
  i = 1;
  for (int a = 0; a < NUM_ELECTRODES; a++) 
        {
        path ="SAMPLES/";
        path += to_string(i);
        path += ".wav";
        cout << "loading file: " << path.c_str() << endl;
     //   sample[a] = Mix_LoadWAV(path.c_str());
        }
  
  while(1)
  {
    MPR121.updateBaselineData();
    MPR121.updateFilteredData();
    
    for (int a = 0; a < NUM_ELECTRODES; a++) 
        {
            if(MPR121.getFilteredData(a)<500)
            {
             // Mix_PlayChannel(-1, sample[a], 0);
            }
          }
         
    delay(100);
  }

  // quit SDL mixer
  //Mix_Quit();

  return 0;  
  
}
*/
