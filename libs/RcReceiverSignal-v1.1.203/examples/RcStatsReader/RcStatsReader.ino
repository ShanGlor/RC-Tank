//
//  RcReceiverSignal Library
//  RcStatsReader example
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
//  The following example reads a PWM signal on pin 2. Each times the PWM signal
//  value changes, the new value is added to a list. Once the list if full, the 
//  program shows all observed values and some statistics based on observed values.
//  The list is cleared and the process starts over.
//  The example is useful for getting empiric data to properly detect the average
//  PWM length of a given transmitter value.
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
#define BUFFER_SIZE 250

//project's switches
#define ENABLE_SERIAL_OUTPUT

DECLARE_RECEIVER_SIGNAL(receiver_aux1_handler);
unsigned long readingsBuffer[BUFFER_SIZE];
unsigned long numReadings = 0;

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
  
  receiver_aux1_handler_setup(RECEIVER_AUX1_IN_PIN);
}

void loop() {

  //detect when the receiver AUX1 value has changed
  if (receiver_aux1_handler.hasChanged())
  {
    unsigned long pwmValue = receiver_aux1_handler.getPwmValue();

    //add value to reading buffer
    readingsBuffer[numReadings] = pwmValue;
    numReadings++;

    //is buffer full ?
    if (numReadings == BUFFER_SIZE)
    {
      //flush
      #ifdef ENABLE_SERIAL_OUTPUT
        char pwnStr[10];
        unsigned long sum = 0;
        unsigned long maxValue = 0;
        unsigned long minValue = 0xFFFF;
        for(unsigned long i=0; i<BUFFER_SIZE; i++)
        {
          pwmValue = readingsBuffer[i];
          sum += pwmValue;
          if (pwmValue > maxValue)
            maxValue = pwmValue;
          if (pwmValue < minValue)
            minValue = pwmValue;
          sprintf(pwnStr, "%04d", pwmValue);
          Serial.println(pwnStr);
        }
        double avg = sum/double(BUFFER_SIZE);
        Serial.print("max=");
        Serial.println(maxValue);
        Serial.print("min=");
        Serial.println(minValue);
        Serial.print("sum=");
        Serial.println(sum);
        Serial.print("avg=");
        Serial.println(avg);
      #endif

      numReadings = 0;

      //ignore the next change since serial printing may have impacted the timing...
      while( !receiver_aux1_handler.hasChanged() )
      {
      }
      
    }
  }
}

