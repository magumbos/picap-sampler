# Pi Cap Sampler
WAV sampler

Red light will show on Picap when ready to use

Single Click to change sample bank

Double click PiCap button to change play back modes
1-single
2-fast
3-medium

Press and Hold to Shutdown

To add more samples, add 12 .wav files to the samples folder to make a new sample bank. Edit the touch-mp3.cpp file and change line 15 to the total number of banks.

## Requirements

#* Requires [WiringPi](http://wiringpi.com/) (`apt-get install wiringpi`)
#* Requires [Bare Conductive's MPR121 libary for WiringPi](https://github.com/BareConductive/wiringpi-mpr121)

## Install / Build

* Clone this repository with `git clone https://github.com/magumbos/picap-sampler.git`

N.B. must be run as root    

To run at start-up
"sudo nano /etc/rc.local"
add before 'exit 0' "/home/pi/the/location/of/script/run &"
Ctrl+X
Y
Enter
