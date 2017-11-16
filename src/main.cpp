#include <stdio.h>
#include <stdlib.h>
#include <Arduino.h>

//RcReceiverSignal library has a dependency to PinChangeInt library.
#include <PinChangeInt.h>
#include <RcReceiverSignal.h>
#include "motor.h"

#define PIN_RC_STEERING 2
#define PIN_RC_THROTTLE 3

#define MOTOR_A_PWM 6 // supports PWM
#define MOTOR_A_DIRECTION 7 // does not support PWM

#define DEBUG
#define DEADBAND 15

Motor* motorA = new Motor(MOTOR_A_PWM, MOTOR_A_DIRECTION);

DECLARE_RECEIVER_SIGNAL(receiver_throttle);
DECLARE_RECEIVER_SIGNAL(receiver_steering);

void setup()
{
  pinMode(PIN_RC_STEERING, INPUT);
  pinMode(PIN_RC_THROTTLE, INPUT);
  pinMode(MOTOR_A_PWM, OUTPUT);
  pinMode(MOTOR_A_DIRECTION, OUTPUT);

  //link RcReceiverSignal to use PinChangeInt library
  RcReceiverSignal::setAttachInterruptFunction(&PCintPort::attachInterrupt);
  RcReceiverSignal::setPinStatePointer(&PCintPort::pinState);
  RcReceiverSignal::setExternalTimeCounter(&micros, 1, 1);

  #ifdef DEBUG
    Serial.begin(115200);
    Serial.println("ready");
  #endif

  receiver_throttle_setup(PIN_RC_THROTTLE);
  receiver_steering_setup(PIN_RC_STEERING);
}

void drive(RcReceiverSignal * receiver_throttle) {
  unsigned long pwmValue = receiver_throttle->getPwmValue();
  const short throttleValue = receiver_throttle->getSignalValue(pwmValue);

  // throttleValue range [-125, 125], 0 is center stick position
  const int speed = map(abs(throttleValue) + DEADBAND, 0, 125, 0, 255);

  // the RC signal oscillates from -10 to +10
  if (throttleValue > -DEADBAND and throttleValue < DEADBAND) {
    // stop motor
    motorA->stop();

  } else if (throttleValue > DEADBAND) {
    motorA->driveForward(speed);

  } else if (throttleValue < -DEADBAND) {
    // accelerate backwards
    motorA->driveBackwards(speed);

  }
}

void loop()
{
  if (receiver_throttle.hasChanged()) {
    drive(&receiver_throttle);
  }
}
