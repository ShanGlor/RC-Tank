//
//  RcReceiverSignal Library
//  Read2Signal example
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
//  The following example shows how one can create 2 instance of RcReceiverSignal
//  to read two independent signals attached on two different pins.
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
#define RECEIVER_AUX2_IN_PIN 3 // we could choose any pin

//project's switches
#define ENABLE_SERIAL_OUTPUT

//declare objects
DECLARE_RECEIVER_SIGNAL(receiver_aux1_handler);
DECLARE_RECEIVER_SIGNAL(receiver_aux2_handler);

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
  RcReceiverSignal::setExternalTimeCounter(&micros, 1, 1);

  //link RcReceiverSignal to use PinChangeInt library
  RcReceiverSignal::setAttachInterruptFunction(&PCintPort::attachInterrupt);
  RcReceiverSignal::setPinStatePointer(&PCintPort::pinState);

  #ifdef ENABLE_SERIAL_OUTPUT
    Serial.begin(115200);
    Serial.println("ready");
  #endif

  receiver_aux1_handler_setup(RECEIVER_AUX1_IN_PIN);
  receiver_aux2_handler_setup(RECEIVER_AUX2_IN_PIN);
}

void printReceiver(const char * iName, RcReceiverSignal * iReceiverSignal)
{
  unsigned long pwmValue = receiver_aux1_handler.getPwmValue();
  RcReceiverSignal::VALUE value = iReceiverSignal->getSignalValue(pwmValue);

  #ifdef ENABLE_SERIAL_OUTPUT
    Serial.print(iName);
    Serial.print(": ");
    char signalStr[10];
    sprintf(signalStr, "%4d", value);
    Serial.print(signalStr);
    Serial.print("% ");
    Serial.print("PWM=");
    Serial.print(iReceiverSignal->getPwmValue());
    Serial.println();
  #endif
}

void loop() {
  //detect when the receiver AUX1 value has changed
  if (receiver_aux1_handler.hasChanged())
  {
    printReceiver("AUX1", &receiver_aux1_handler);
  }

  //detect when the receiver AUX2 value has changed
  if (receiver_aux2_handler.hasChanged())
  {
    printReceiver("AUX2", &receiver_aux2_handler);
  }
}
