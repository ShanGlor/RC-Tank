//
//  RcReceiverSignal Library
//  LedUpdate example
//  Copyright (C) 2016 Antoine Beauchamp
//  The code & updates for the library can be found on http://end2endzone.com
//
// AUTHOR/LICENSE:
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 3.0 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License (LGPL-3.0) for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
// DISCLAIMER:
//  This software is furnished "as is", without technical support, and with no 
//  warranty, express or implied, as to its usefulness for any purpose.
//
// PURPOSE:
//  The following example reads a PWM signal on pin 2. If the value is below the
//  center position, it turns ON the arduino's LED. If the value is close to the
//  center position, it blinks the arduino's LED. If the value is higher than the
//  center position, it turns OFF the arduino's LED.
//

//
//RcReceiverSignal library has a dependency to PinChangeInt library.
#include <PinChangeInt.h>

//
//This library allows one to have a micros() replacement function 
//which has a 1us resolution instead of 4usec.
//For more information on this library, see the following:
// http://electricrcaircraftguy.com/2014/02/Timer2Counter-more-precise-Arduino-micros-function.html
// http://www.instructables.com/id/How-to-get-an-Arduino-micros-function-with-05us-pr/
//
#include <eRCaGuy_Timer2_Counter.h>

//that's the example's library!
#include <RcReceiverSignal.h>

//project's contants
#define RECEIVER_AUX1_IN_PIN 2 // we could choose any pin
#define LED_OUT_PIN 13
#define LED_MODE_ON       0
#define LED_MODE_OFF      1
#define LED_MODE_BLINK    2
#define LED_MODE_BLINK_TIME 100 //blinks every 100 ms
#define PWM_CENTER_EPSILON 100 //defines the epsilon value which allows to detect "center" position

//project's switches
#define ENABLE_SERIAL_OUTPUT

DECLARE_RECEIVER_SIGNAL(receiver_aux1_handler);
char ledMode = LED_MODE_OFF;  //initialize LED to OFF by default
int ledState = LOW;           //initialize LED to OFF by default
unsigned long prevBlinkTime = 0;

uint32_t timer2CounterWrapperFunction() {
  return timer2.get_count();
}

void setup() {
  //configure Timer2
  timer2.setup(); //this MUST be done before the other Timer2_Counter functions work; Note: since this messes up PWM outputs on pins 3 & 11, as well as 
                  //interferes with the tone() library (http://arduino.cc/en/reference/tone), you can always revert Timer2 back to normal by calling 
                  //timer2.unsetup()
  
  //configure RcReceiverSignal with an external time counter
  //eRCaGuy_Timer2_Counter lirary has 0.5us resolution.
  //The counter value must be divided by 2 to convert from 0.5us steps to 1us steps
  //which results in microseconds resolution.
  RcReceiverSignal::setExternalTimeCounter(&timer2CounterWrapperFunction, 1, 2);

  //link RcReceiverSignal to use PinChangeInt library
  RcReceiverSignal::setAttachInterruptFunction(&PCintPort::attachInterrupt);
  RcReceiverSignal::setPinStatePointer(&PCintPort::pinState);

  #ifdef ENABLE_SERIAL_OUTPUT
    Serial.begin(115200);
    Serial.println("ready");
  #endif

  // initialize digital pin 13 as an output.
  pinMode(LED_OUT_PIN, OUTPUT);
  
  receiver_aux1_handler_setup(RECEIVER_AUX1_IN_PIN);
}

void blinkLed()
{
  // See https://www.arduino.cc/en/Tutorial/BlinkWithoutDelay
  
  // check to see if it's time to blink the LED; that is, if the
  // difference between the current time and last time you blinked
  // the LED is bigger than the interval at which you want to
  // blink the LED.
  unsigned long currentMillis = millis();

  if (currentMillis - prevBlinkTime >= LED_MODE_BLINK_TIME) {
    // save the last time you blinked the LED
    prevBlinkTime = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    // set the LED with the ledState of the variable:
    digitalWrite(LED_OUT_PIN, ledState);
  }  
}

void loop() {

  //update LED status
  if (ledMode == LED_MODE_ON) {
    ledState = HIGH;
    digitalWrite(LED_OUT_PIN, ledState);  // turn the LED on (HIGH is the voltage level)
  }
  else if (ledMode == LED_MODE_OFF) {
    ledState = LOW;
    digitalWrite(LED_OUT_PIN, ledState);   // turn the LED off by making the voltage LOW
  }
  else if (ledMode == LED_MODE_BLINK)
  {
    blinkLed();
  }
  
  //detect when the receiver AUX1 value has changed
  if (receiver_aux1_handler.hasChanged())
  {
    unsigned long pwmValue = receiver_aux1_handler.getPwmValue();

    if (pwmValue < 1500-PWM_CENTER_EPSILON) {
      #ifdef ENABLE_SERIAL_OUTPUT
      if (ledMode != LED_MODE_ON)
        Serial.println("Setting LED to ON");
      #endif
      ledMode = LED_MODE_ON;
    }
    else if (pwmValue > 1500+PWM_CENTER_EPSILON) {
      #ifdef ENABLE_SERIAL_OUTPUT
      if (ledMode != LED_MODE_OFF)
        Serial.println("Setting LED to OFF");
      #endif
      ledMode = LED_MODE_OFF;
    }
    else {
      #ifdef ENABLE_SERIAL_OUTPUT
      if (ledMode != LED_MODE_BLINK)
        Serial.println("Setting LED to BLINK");
      #endif
      ledMode = LED_MODE_BLINK;
    }
  }
}

