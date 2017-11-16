# RC Tank

[![Build Status](https://travis-ci.org/hotzenklotz/RC-Tank.svg?branch=master)](https://travis-ci.org/hotzenklotz/RC-Tank)
A small hobby Arduino project to connect some motors to a Flysky RC receiver using PWM signals.

## Hardware Requirements

* PWM RC Receiver, e.g. Flysky FS iA6B
* A matching RC Radio e.g. Turnigy Evolution, set to PWM output mode
* Arduino Nano
* Dual Channel Motor driver, e.g. L9110

## Hardware Setup

* Wire channel 2 and 3 from the RC receiver to the interupt pins of the Arduino:
  * Channel 2 --> Digital Pin 2
  * Channel 3 --> Digital Pin 3
* Wire Motor A from the motor driver to the Arduino:
  * Motor Driver A-1A --> Digital Pin 6
  * Motor Driver A-1B --> Digital Pin 7

## Software Setup

* Install the dependency libraries [PinChangeInt](https://github.com/GreyGnome/PinChangeInt) and
  [RcReceiverSignal](http://www.end2endzone.com/rcreceiversignal-an-arduino-library-for-retreiving-the-rc-transmitter-value-from-an-rc-receiver-pulse/)
* (Adjust used PINs in the Arduino sketch)
* Upload Arduino sketch

## License

MIT @ Tom Herold
