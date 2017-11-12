//
//  RcReceiverSignal Library
//  CcpmServoTester example
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
//  The following example shows how RcReceiverSignal can be configured to use
//  the native micros() function to getPWM values. This prevents the RcReceiverSignal
//  to have a dependency on an external time counter library.
//

#include <stdio.h>
#include <stdlib.h>
#include <Arduino.h>
//
//RcReceiverSignal library has a dependency to PinChangeInt library.
#include "/Users/therold/Programming/RC-Tank/libs/PinChangeInt/PinChangeInt.h"


//that's the example's library!
#include "/Users/therold/Programming/RC-Tank/libs/RcReceiverSignal-v1.1.203/RcReceiverSignal.h"

//project's contants
#define RECEIVER_AUX1_IN_PIN 2 // we could choose any pin

//project's switches
#define ENABLE_SERIAL_OUTPUT

DECLARE_RECEIVER_SIGNAL(receiver_aux1_handler);

void setup() {
  //configure RcReceiverSignal with the native micros() time counter
  //micros() counter function is already in usec.
  //no need for divisor or multiplicator: divisor=1, multiplicator=1
  RcReceiverSignal::setExternalTimeCounter(&micros, 1, 1);

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

    // convert pwm to actual travel range
    RcReceiverSignal::VALUE value = receiver_aux1_handler.getDeviceSignalValue(LEGACY, pwmValue);

    // show the pwm & travel values on serial
    #ifdef ENABLE_SERIAL_OUTPUT
    Serial.print("pwm=");
    Serial.print(pwmValue);
    Serial.print(", travel=");
    Serial.println(value);
    #endif
  }
}

