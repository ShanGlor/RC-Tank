//
//  RcReceiverSignal Library - v1.1 - 05/29/2016
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
//  The RcReceiverSignal library allows one to easily configure an input pin to
//  read a PWM signal sent from a Remote Controlled (RC) receiver by attaching an
//  interrupt to a pin connected the receiver and automatically monitor each
//  pulse sent by the receiver.
//
//  It allows the main loop to retrieve the latest received pulse length (PWM)
//  and convert the pulse length back to the actual transmitter signal value
//  (ranging from -150% to 150%).
//
//  The library supports automatic PWM signal change detection. The library's
//  ability to convert a PWM value to the actual transmitter numeric value is
//  based on empirical tests available at
//  http://www.end2endzone.com/mapping-rc-transmitter-pwm-signal-to-actual-transmitter-values/
//
// USAGE:
//  Use the DECLARE_RECEIVER_SIGNAL macro to declare an instance of RcReceiverSignal.
//  ie: DECLARE_RECEIVER_SIGNAL(receiver_aux1_handler);
//  This will declare the following:
//    RcReceiverSignal receiver_aux1_handler;
//    receiver_aux1_handler_setup() function.
//    receiver_aux1_handler_pin_change() ISR function.
//  In the setup() function, you need to setup each instance by calling
//  the receiver_aux1_handler_setup() function with the interrupt pin as argument:
//  ie: receiver_aux1_handler_setup(RECEIVER_AUX1_IN_PIN);
//  In the loop function, one can call the hasChanged() method to know if the
//  PWM value has changed since the last call or the getPwmValue() function
//  to get the last PWM value observed by the RcReceiverSignal instance.
//
//  See examples for details
//
// HISTORY:
// 03/28/2016 v1.0 - Initial release.
// 05/29/2016 v1.1 - Removed hardcoded dependencies to PinChangeInt.h and
//                   eRCaGuy_Timer2_Counter.h libraries.
//
#ifndef RCRECEIVERSIGNAL_H
#define RCRECEIVERSIGNAL_H

#define RCRECEIVERSIGNAL_USE_INT_CHANGE_EVENT
#define RCRECEIVERSIGNAL_USE_EXT_32BITS_COUNTER

#include "Arduino.h"

#define MIN_RECEIVER_PWM 820
#define MAX_RECEIVER_PWM 2200
#define MIN_RECEIVER_SIGNAL -150
#define MAX_RECEIVER_SIGNAL +150

enum RcTxRxCombo {
  LEGACY = 0, //make sure the enum is 0 based to be usable as an array index
  SPEKTRUM_DX9_ORANGE_R620X,
  SPEKTRUM_DX9_SPEKTRUM_AR8000,
  TACTIC_TTX600_TACTIC_TR624_CH1,
  TACTIC_TTX600_TACTIC_TR624_CH2,
  CCPM_SERVO_TESTER,
};

class RcReceiverSignal
{
public:
  RcReceiverSignal();
  ~RcReceiverSignal();

  typedef short VALUE;

  /****************************************************************************
   * PinChangeInt indirect dependency support
   ****************************************************************************/
public:
  typedef void (*ISR)(void);
  typedef int8_t (*PCIntAttachInterruptFuncPtr)(uint8_t, ISR, int16_t);
  static void setAttachInterruptFunction(PCIntAttachInterruptFuncPtr iAttachInterruptPtr);
  static void setPinStatePointer(volatile uint8_t * iPinStatePtr);
private:
  static PCIntAttachInterruptFuncPtr mAttachIntFuncPtr;
  static volatile uint8_t * mPinStatePtr;
public:


#ifdef RCRECEIVERSIGNAL_USE_INT_CHANGE_EVENT
  /****************************************************************************
   * Description:
   *   setup() function setups the instance to be ready for use.
   * Parameters:
   *   iReceiverPin:      The pin which is connected to the Rc Receiver.
   *   iChangedFunction:  A pointer to the ISR function which handles the pin
   *                      change event.
   *
   ****************************************************************************/
  void setup(uint8_t iReceiverPin, ISR iChangedFunction);
#else
  /****************************************************************************
   * Description:
   *   setup() function setups the instance to be ready for use.
   * Parameters:
   *   iReceiverPin:      The pin which is connected to the Rc Receiver.
   *   iRisingFunction:   A pointer to the ISR function which handles the pin
   *                      rising event.
   *   iFallingFunction:  A pointer to the ISR function which handles the pin
   *                      falling event.
   *
   ****************************************************************************/
  void setup(uint8_t iReceiverPin, ISR iRisingFunction, ISR iFallingFunction);
#endif

  /****************************************************************************
   * Description:
   *   getPwmValue() returns the last PWM value observed by the instance of the
   *   configured pin. The method is ISR-safe which means that it will alter
   *   hardware intterupts on all pins if the function is call to often.
   *   Use hasChanged() to minimize the impact.
   *
   ****************************************************************************/
  unsigned long getPwmValue();

  /****************************************************************************
   * Description:
   *   getSignalValue() convert a given PWM signal to a transmitter value.
   *   The returned a value ranges from -150% to +150%.
   * Parameters:
   *   iPwmValue:   A PWM value. See getPwmValue().
   *
   ****************************************************************************/
  VALUE getSignalValue(unsigned long iPwmValue);

  /****************************************************************************
   * Description:
   *   getDeviceSignalValue() convert a given PWM signal from a known device
   *   combination to a transmitter value.
   *   The returned a value ranges from -150% to +150%.
   * Parameters:
   *   iComboId:  A valid enumation of RcTxRxCombo that defines the device combination.
   *   iPwmValue: A PWM value. See getPwmValue().
   *
   ****************************************************************************/
  VALUE getDeviceSignalValue(RcTxRxCombo iComboId, unsigned long iPwmValue);

  /****************************************************************************
   * Description:
   *   hasChanged() returns true when the PWM value has changed since the last
   *   call to hasChanged(). It allows one to automatically detect a signal change
   *   and process PWM code only on changes instead of each loop execution.
   *
   ****************************************************************************/
  bool hasChanged();

#ifdef RCRECEIVERSIGNAL_USE_INT_CHANGE_EVENT
  /****************************************************************************
   * Description:
   *   onPinChanged() updates the instance when the pin has changed.
   *   This method is meant to be called from the instance ISR routine declared
   *   by the DECLARE_RECEIVER_SIGNAL macro.
   *
   ****************************************************************************/
  void onPinChanged();
#else
  /****************************************************************************
   * Description:
   *   onPinRising() updates the instance when the pin has risen.
   *   This method is meant to be called from the instance ISR routine declared
   *   by the DECLARE_RECEIVER_SIGNAL macro.
   *
   ****************************************************************************/
  void onPinRising();

  /****************************************************************************
   * Description:
   *   onPinFalling() updates the instance when the pin has fallen.
   *   This method is meant to be called from the instance ISR routine declared
   *   by the DECLARE_RECEIVER_SIGNAL macro.
   *
   ****************************************************************************/
  void onPinFalling();
#endif

#ifdef RCRECEIVERSIGNAL_USE_EXT_32BITS_COUNTER
  //external 32 bits counter support
  typedef uint32_t (*CounterFuncPtr)(void);

  /****************************************************************************
   * Description:
   *   setExternalTimeCounter() function setups library to use an external
   *   timer counter.
   * Parameters:
   *   iExtCntFunc:     Pointer to an external 32 bits time counter function.
   *                    The function expects the following signature:
   *                    uint32_t func();
   *   iMultiplicator:  A multiplicator used to convert the counter to
   *                    usec precision. Defaults to 1 if iExtCntFunc already
   *                    has a usec resolution.
   *   iMultiplicator:  A multiplicator used to convert the counter to
   *                    usec precision. Defaults to 1 if iExtCntFunc already
   *                    has a usec resolution.
   *
   ****************************************************************************/
  static void setExternalTimeCounter(CounterFuncPtr iExtCntFunc, uint32_t iMultiplicator, uint32_t iDivisor);

#endif //RCRECEIVERSIGNAL_USE_EXT_32BITS_COUNTER


private:
  //attributes
#ifdef RCRECEIVERSIGNAL_USE_EXT_32BITS_COUNTER
  static CounterFuncPtr mExtCntFunc;
  static uint32_t mExtCntMul;
  static uint32_t mExtCntDiv;
#endif //RCRECEIVERSIGNAL_USE_EXT_32BITS_COUNTER
#ifdef RCRECEIVERSIGNAL_USE_INT_CHANGE_EVENT
  ISR mChangedFunction;
#else
  ISR mRisingFunction;
  ISR mFallingFunction;
#endif
  uint8_t mReceiverPin;
  volatile unsigned long mRisingTime; //time in micros() on last pin rise
  volatile unsigned long mFallingTime; //time in micros() on last pin fall
  volatile unsigned long mPwmValue; //PWM value expected from 1000 us to 2000 us
  volatile bool mChanged;
};

#ifdef RCRECEIVERSIGNAL_USE_INT_CHANGE_EVENT
#define DECLARE_RECEIVER_SIGNAL(variable_name) \
RcReceiverSignal variable_name; \
void variable_name ## _pin_change() \
{ \
  variable_name.onPinChanged(); \
} \
void variable_name ## _setup(uint8_t iReceiverPin) \
{ \
  variable_name.setup(iReceiverPin, &( variable_name ## _pin_change) ); \
}

#else //RCRECEIVERSIGNAL_USE_INT_CHANGE_EVENT

#define DECLARE_RECEIVER_SIGNAL(variable_name) \
RcReceiverSignal variable_name; \
void variable_name ## _pin_rising() \
{ \
  variable_name.onPinRising(); \
} \
void variable_name ## _pin_falling() \
{ \
  variable_name.onPinFalling(); \
} \
void variable_name ## _setup(uint8_t iReceiverPin) \
{ \
  variable_name.setup(iReceiverPin, &( variable_name ## _pin_rising), &( variable_name ## _pin_falling) ); \
}
#endif //RCRECEIVERSIGNAL_USE_INT_CHANGE_EVENT


#endif //RCRECEIVERSIGNAL_H
