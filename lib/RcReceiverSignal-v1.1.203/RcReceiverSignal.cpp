// ---------------------------------------------------------------------------
//  RcReceiverSignal Library - v1.1 - 05/29/2016
//  Copyright (C) 2016 Antoine Beauchamp
//  The code & updates for the library can be found on http://end2endzone.com
//
// See "RcReceiverSignal.h" for license, purpose, syntax, version history, links, and more.
// ---------------------------------------------------------------------------

#include "Arduino.h"
#include "RcReceiverSignal.h"

#define CLAMP_VALUE(value_min, actual_value, value_max) (actual_value < value_min ? value_min : (actual_value > value_max ? value_max : actual_value)  )

typedef struct {
  double a2;
  double a1;
  double a0;
} POLYMONIAL_FUNCTION;

// Define all polynomial functions for all RcTxRxCombo enumations
static POLYMONIAL_FUNCTION comboFunctions[] = {
  //a2,     a1,           a0
  {0.0,     0.220219436,  -331.6504702},  //LEGACY,
  {-8.0e-8, 0.2203,       -331.37},       //SPEKTRUM_DX9_ORANGE_R620X,
  {0.0,     0.25089444,   -382.665703},   //SPEKTRUM_DX9_SPEKTRUM_AR8000,
  {-3.0e-6, 0.2221,       -327.22},       //TACTIC_TTX600_TACTIC_TR624_CH1,
  {-3.0e-6, 0.2135,       -318.97},       //TACTIC_TTX600_TACTIC_TR624_CH2,
  {+4.0e-7, 0.1653,       -249.78}        //CCPM_SERVO_TESTER,
};

#ifdef RCRECEIVERSIGNAL_USE_EXT_32BITS_COUNTER
RcReceiverSignal::CounterFuncPtr RcReceiverSignal::mExtCntFunc = NULL;
uint32_t RcReceiverSignal::mExtCntMul = 1;
uint32_t RcReceiverSignal::mExtCntDiv = 1;
#endif //RCRECEIVERSIGNAL_USE_EXT_32BITS_COUNTER

RcReceiverSignal::RcReceiverSignal()
{
#ifdef RCRECEIVERSIGNAL_USE_INT_CHANGE_EVENT
  mChangedFunction = NULL;
#else
  mRisingFunction = NULL;
  mFallingFunction = NULL;
#endif
  mRisingTime = 0;
  mFallingTime = 0;
  mPwmValue = 0xFFFF;
  mReceiverPin = 0;
  mChanged = false;
}

RcReceiverSignal::~RcReceiverSignal()
{
}

/****************************************************************************
 * PinChangeInt indirect dependency support
 ****************************************************************************/
RcReceiverSignal::PCIntAttachInterruptFuncPtr RcReceiverSignal::mAttachIntFuncPtr = NULL;
volatile byte * RcReceiverSignal::mPinStatePtr = NULL;
void RcReceiverSignal::setAttachInterruptFunction(PCIntAttachInterruptFuncPtr iAttachInterruptPtr)
{
  mAttachIntFuncPtr = iAttachInterruptPtr;
}
void RcReceiverSignal::setPinStatePointer(volatile uint8_t * iPinStatePtr)
{
  mPinStatePtr = iPinStatePtr;
}

#ifdef RCRECEIVERSIGNAL_USE_INT_CHANGE_EVENT
void RcReceiverSignal::setup(uint8_t iReceiverPin, ISR iChangedFunction)
{
  //check if library was configured
  //this is required to prevent the code from jumping to random locations
  if (mAttachIntFuncPtr == NULL || mPinStatePtr == NULL)
    return;

  pinMode(iReceiverPin, INPUT); digitalWrite(iReceiverPin, HIGH); //use the internal pullup resistor

  mReceiverPin = iReceiverPin;
  mChangedFunction = iChangedFunction;

  //prepare first interruption on pin RISING
  mAttachIntFuncPtr(mReceiverPin, mChangedFunction, CHANGE);
}
#else
void RcReceiverSignal::setup(uint8_t iReceiverPin, ISR iRisingFunction, ISR iFallingFunction)
{
  //check if library was configured
  //this is required to prevent the code from jumping to random locations
  if (mAttachIntFuncPtr == NULL || mPinStatePtr == NULL)
    return;

  pinMode(iReceiverPin, INPUT); digitalWrite(iReceiverPin, HIGH); //use the internal pullup resistor

  mReceiverPin = iReceiverPin;
  mRisingFunction = iRisingFunction;
  mFallingFunction = iFallingFunction;

  //prepare first interruption on pin RISING
  mAttachIntFuncPtr(mReceiverPin, mRisingFunction, RISING);
}
#endif

RcReceiverSignal::VALUE RcReceiverSignal::getSignalValue(unsigned long iPwmValue)
{
  //make sure the PWM value is within acceptable range
  unsigned long pwmValue = CLAMP_VALUE(MIN_RECEIVER_PWM, iPwmValue, MAX_RECEIVER_PWM);

  //map to a signal value
  RcReceiverSignal::VALUE signalValue = 0;

  //Note: The map() function is far from ideal for the calculations.
  //signalValue = map(pwmValue, MIN_RECEIVER_PWM, MAX_RECEIVER_PWM, MIN_RECEIVER_SIGNAL, MAX_RECEIVER_SIGNAL);

  //use a polynomial function of level 1 (only a0 and a1 are defined)
  signalValue = (RcReceiverSignal::VALUE)(0.220219436*pwmValue -331.6504702);

  //make sure output value is within acceptable range
  signalValue = CLAMP_VALUE(MIN_RECEIVER_SIGNAL, (int)(signalValue) , MAX_RECEIVER_SIGNAL);

  return signalValue;
}

RcReceiverSignal::VALUE RcReceiverSignal::getDeviceSignalValue(RcTxRxCombo iComboId, unsigned long iPwmValue)
{
  //make sure the PWM value is within acceptable range
  unsigned long pwmValue = CLAMP_VALUE(MIN_RECEIVER_PWM, iPwmValue, MAX_RECEIVER_PWM);

  //find the polynomial function that matches the device combination
  POLYMONIAL_FUNCTION * poly = NULL;
  switch(iComboId)
  {
  case LEGACY:
  case SPEKTRUM_DX9_ORANGE_R620X:
  case SPEKTRUM_DX9_SPEKTRUM_AR8000:
  case TACTIC_TTX600_TACTIC_TR624_CH1:
  case TACTIC_TTX600_TACTIC_TR624_CH2:
  case CCPM_SERVO_TESTER:
    poly = &comboFunctions[iComboId];
    break;
  default:
    poly = &comboFunctions[LEGACY];
    break;
  };

  //map to a signal value
  RcReceiverSignal::VALUE signalValue = 0;

  //use device's polynomial function for computing actual transmitter value
  signalValue = (RcReceiverSignal::VALUE)(poly->a2*pwmValue*pwmValue + poly->a1*pwmValue + poly->a0);

  //make sure output value is within acceptable range
  signalValue = CLAMP_VALUE(MIN_RECEIVER_SIGNAL, (int)(signalValue) , MAX_RECEIVER_SIGNAL);

  /*
  // debug output values:
  Serial.print("device=");
  Serial.print(iComboId);
  Serial.print(", a2=");
  Serial.print(poly->a2);
  Serial.print(", a1=");
  Serial.print(poly->a1);
  Serial.print(", a0=");
  Serial.print(poly->a0);
  Serial.print(", pwmValue=");
  Serial.print(pwmValue);
  Serial.print(", signalValue=");
  Serial.print(signalValue);
  Serial.println();
  */

  return signalValue;
}

unsigned long RcReceiverSignal::getPwmValue()
{
  uint8_t pushedSREG = SREG;            // save interrupt flag
  cli();                                // disable interrupts
  unsigned long pwmCopy = mPwmValue;    // access the shared data
  mChanged = false;                     // turn off the shared changed flag
  SREG = pushedSREG;                    // restore the interrupt flag

  return pwmCopy;
}

bool RcReceiverSignal::hasChanged()
{
  return mChanged;
}

#ifdef RCRECEIVERSIGNAL_USE_INT_CHANGE_EVENT
void RcReceiverSignal::onPinChanged()
{
  //check if library was configured
  //this is required to prevent the code from jumping to random locations
  if (mAttachIntFuncPtr == NULL || mPinStatePtr == NULL)
    return;

  if(*mPinStatePtr == HIGH)
  {
    //this is a rising edge
    //record time
#ifdef RCRECEIVERSIGNAL_USE_EXT_32BITS_COUNTER
    mRisingTime = mExtCntFunc(); //use external counter
#else
    mRisingTime = micros(); //count units of 4us each;
#endif
  }
  else
  {
    //this is a falling edge

    //save previous value
    unsigned long prevPwmValue = mPwmValue;

    //read elapsed time since RISING
#ifdef RCRECEIVERSIGNAL_USE_EXT_32BITS_COUNTER
    mFallingTime = mExtCntFunc(); //use external counter
    mPwmValue = (mFallingTime - mRisingTime);
    mPwmValue /= mExtCntDiv; //to get 1us precision
    mPwmValue *= mExtCntMul; //to get 1us precision
#else
    mFallingTime = micros();
    mPwmValue = mFallingTime - mRisingTime;
#endif

    //update changed flag
    mChanged = (mPwmValue != prevPwmValue);
  }
}
#else
void RcReceiverSignal::onPinRising()
{
  //check if library was configured
  //this is required to prevent the code from jumping to random locations
  if (mAttachIntFuncPtr == NULL || mPinStatePtr == NULL)
    return;

  //record time
#ifdef RCRECEIVERSIGNAL_USE_EXT_32BITS_COUNTER
  mRisingTime = mExtCntFunc(); //use external counter
#else
  mRisingTime = micros(); //count units of 4us each;
#endif

  //prepare interrupting on pin FALLING
  mAttachIntFuncPtr(mReceiverPin, mFallingFunction, FALLING);
}

void RcReceiverSignal::onPinFalling()
{
  //check if library was configured
  //this is required to prevent the code from jumping to random locations
  if (mAttachIntFuncPtr == NULL || mPinStatePtr == NULL)
    return;

  //save previous value
  unsigned long prevPwmValue = mPwmValue;

  //read elapsed time since RISING
#ifdef RCRECEIVERSIGNAL_USE_EXT_32BITS_COUNTER
  mFallingTime = mExtCntFunc(); //use external counter
  mPwmValue = (mFallingTime - mRisingTime);
  mPwmValue /= mExtCntDiv; //to get 1us precision
  mPwmValue *= mExtCntMul; //to get 1us precision
#else
  mFallingTime = micros(); //count units of 4us each;
  mPwmValue = mFallingTime - mRisingTime;
#endif

  //update changed flag
  mChanged = (mPwmValue != prevPwmValue);

  //prepare interrupting on pin RISING
  mAttachIntFuncPtr(mReceiverPin, mRisingFunction, RISING);
}
#endif

#ifdef RCRECEIVERSIGNAL_USE_EXT_32BITS_COUNTER
void RcReceiverSignal::setExternalTimeCounter(CounterFuncPtr iExtCntFunc, uint32_t iMultiplicator, uint32_t iDivisor)
{
  mExtCntFunc = iExtCntFunc;
  mExtCntMul = iMultiplicator;
  mExtCntDiv = iDivisor;
}
#endif //RCRECEIVERSIGNAL_USE_EXT_32BITS_COUNTER
