# Pi Cap Sampler
WAV sampler


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
