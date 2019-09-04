/*******************************************************************************

  This is heavily based on the work of Stegan Dzisiewski-Smith. 

  Bare Conductive Pi Cap
  ----------------------

  button-utility.cpp - utility for reacting to single-click, long-click and
  double-click events from the Pi Cap button

  Written for Raspberry Pi.

  Bare Conductive code written by Stefan Dzisiewski-Smith.

  This work is licensed under a MIT license https://opensource.org/licenses/MIT
  
  Copyright (c) 2016, Bare Conductive
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

 *******************************************************************************/

#include <MPR121.h>
#include <wiringPi.h>
#include <signal.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>


#define NUM_SAMPLE_BANKS 2 //-----------------------Change here if you have added extra samples to the folder
#define NUM_ELECTRODES 12

#define RED_LED_PIN 22
#define GREEN_LED_PIN 21
#define BLUE_LED_PIN 25

#define SINGLE_TOUCH 1
#define TOUCH_DOUBLE 2
#define TOUCH_TRIPLE 3
int TOTAL_PROG = 3; 

using namespace std;

#define BUTTON_PIN 7 // this is wiringPi pin 7, which just happens to be physical pin 7 too
#define DEBOUNCE_LOCKOUT_MS    10
#define DOUBLEPRESS_TIMEOUT_US 300000
#define LONGPRESS_TIMEOUT_US   750000
string longPressCommand = "sync && halt &";

// enums and variables for state and timeout action
enum state_t {IDLE, PRESSED, RELEASED};
state_t volatile state = IDLE;

enum action_t {NONE, SINGLE_PRESS, LONG_PRESS};
action_t volatile action = NONE;

bool volatile isrEnabled = true;
bool volatile buttonFlag = false;

int sample_no;
string path;
Mix_Chunk* sample[NUM_ELECTRODES];

//programs and delays
int updateDelay = 200;
int updateDelay2 = 100;
int PROG = 1;
int elecTouch[12];

// "volatile" means - I can be modified from elsewhere
bool volatile keepRunning = true;
 
void load_samples(void)
{
  if(sample_no>NUM_SAMPLE_BANKS*12)sample_no=1;
  
  for (int a = 0; a < NUM_ELECTRODES; a++) 
        {
        path ="SAMPLES/";
        path += to_string(sample_no);
        path += ".wav";
        cout << "loading file: " << path.c_str() << endl;
        sample[a] = Mix_LoadWAV(path.c_str());
        sample_no++;
        }
}

void led(int r, int g, int b) {
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
  led(0, 0, 0);
  exit(0);
}

void singlePress() {
  load_samples();
}

void doublePress() {
  cout << "Change Touch" << endl;
  PROG++;
  if(PROG>TOTAL_PROG)PROG=1;
   elecTouch[0] = 1000;
}

void longPress() {
  // long press event handler
  system(longPressCommand.c_str());
}

void alarmHandler(int dummy) {
  // time-based part of state machine
  switch (action) {
    case NONE:
      break;
    case SINGLE_PRESS:
      singlePress(); // call the single press event handler
      action = NONE;
      state = IDLE;
      break;
    case LONG_PRESS:
      longPress(); // call the long press event handler
      action = NONE;
      state = IDLE;
      break;
    default:
      break;
  }
}

void buttonIsr(void) {
  // event based part of state machine
  if(isrEnabled) buttonFlag = true; // set the ISR flag, but only if our soft-gate is enabled
}

void buttonPress(void)
{
        if (!digitalRead(BUTTON_PIN)) {
        // button just pressed
        led(0, 0, 0);
        delay(10);
        led(1,0,0);
        
        switch (state) {
          case IDLE:
            // disable the button ISR, set state to pressed and set long press timeout
            isrEnabled = false;
            state = PRESSED;
            action = LONG_PRESS; // what we'll do if we time out in this state...
            ualarm(LONGPRESS_TIMEOUT_US,0);
            // delay a bit to avoid erroneous double-presses from switch bounce
            usleep(DEBOUNCE_LOCKOUT_MS);
            // re-enable the ISR once we're clear of switch bounce
            isrEnabled = true;
            break;
          case RELEASED:
            // if we get another press when the switch has been released (and before
            // the double-press timeout has occured) we have a double-press
            // so reset the state machine
            action = NONE;
            state = IDLE;
            doublePress(); // call the double press event handler
            break;
          default:
            break;
        }
      }
      else {
        // button just released
        switch (state) {
          case PRESSED:
            // disable the button ISR, set state to released and set double press timeout
            isrEnabled = false;
            action = SINGLE_PRESS; // what we'll do if we timeout in this state
            ualarm(DOUBLEPRESS_TIMEOUT_US,0);
            // delay a bit to avoid erroneous double-presses from switch bounce
            usleep(DEBOUNCE_LOCKOUT_MS);
            state = RELEASED;
            // re-enable the ISR once we're clear of switch bounce
            isrEnabled = true;
            break;
          default:
            break;
        }
      }

      buttonFlag = false;
}


int main(void) {
  
  cout << "PiCap Sampler" << endl;
  
  // wait for Ctrl+C
  signal(SIGINT, intHandler);
  
    // register our interrupt handler for button press
  signal(SIGALRM, alarmHandler);
  wiringPiSetup();
  pinMode(BUTTON_PIN, INPUT);
  pullUpDnControl(BUTTON_PIN, PUD_UP);
  wiringPiISR(BUTTON_PIN, INT_EDGE_BOTH, buttonIsr);

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

  
  
  cout << "Load Samples" << endl;
  
  sample_no = 1;
  load_samples();
  
  led(1,0,0);
  
  cout << "All Ready" << endl;

  while (keepRunning) {
    
    if (buttonFlag) buttonPress();
    
    switch(PROG)
    {
    case SINGLE_TOUCH:
    
    if (MPR121.touchStatusChanged()) {
      MPR121.updateTouchData();

      for (int b = 0; b < NUM_ELECTRODES; b++) {
        if (MPR121.isNewTouch(b)) {
        Mix_PlayChannel(-1, sample[b], 0);
        }
      }
    }
        
    break;
    
    case TOUCH_DOUBLE:
    elecTouch[0]+=10;
    
    if(elecTouch[0]>updateDelay2)
    {
      MPR121.updateFilteredData();
      
      for (int a = 0; a < NUM_ELECTRODES; a++) 
          {
            if(MPR121.getFilteredData(a)<500) Mix_PlayChannel(-1, sample[a], 0);
          }
          
      elecTouch[0]=0;
    }
          
    if(elecTouch[0]>2000) elecTouch[0]=1000; //just incase we never reset
    break;
    
    case TOUCH_TRIPLE:
    elecTouch[0]+=10;
    
    if(elecTouch[0]>updateDelay)
    {
      MPR121.updateFilteredData();
      
      for (int a = 0; a < NUM_ELECTRODES; a++) 
          {
            if(MPR121.getFilteredData(a)<500) Mix_PlayChannel(-1, sample[a], 0);
          }
          
      elecTouch[0]=0;
    }
          
    if(elecTouch[0]>2000) elecTouch[0]=1000; //just incase we never reset
  }
  
    delay(10);
  }

  // quit SDL mixer
  Mix_Quit();

  return 0;
}
