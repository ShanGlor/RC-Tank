#include <stdio.h>
#include <stdlib.h>
#include <Arduino.h>

//RcReceiverSignal library has a dependency to PinChangeInt library.
#include <PinChangeInt.h>
#include <RcReceiverSignal.h>
#include "fscale.h"
#include "motor.h"

#define PIN_RC_STEERING 2
#define PIN_RC_THROTTLE 3

#define MOTOR_A_PWM 6 // supports PWM
#define MOTOR_A_DIRECTION 7 // does not support PWM

#define MOTOR_B_DIRECTION 8 // does not support PWM
#define MOTOR_B_PWM 9 // supports PWM

#define DEBUG
#define CENTER_STICK_PWM 1500 // RC value for a centered joystick
#define DEADBAND 60 // deadband around the center of the joystick, where nothing should happen

#define MIN_PWM 0
#define MAX_PWM 255
#define MIN_STICK_VALUE 0
#define MAX_STICK_VALUE 500

Motor motorA(MOTOR_A_PWM, MOTOR_A_DIRECTION);
Motor motorB(MOTOR_B_PWM, MOTOR_B_DIRECTION);

DECLARE_RECEIVER_SIGNAL(receiver_throttle);
DECLARE_RECEIVER_SIGNAL(receiver_steering);

void setup()
{
  pinMode(PIN_RC_STEERING, INPUT);
  pinMode(PIN_RC_THROTTLE, INPUT);
  pinMode(MOTOR_A_PWM, OUTPUT);
  pinMode(MOTOR_A_DIRECTION, OUTPUT);
  pinMode(MOTOR_B_PWM, OUTPUT);
  pinMode(MOTOR_B_DIRECTION, OUTPUT);

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

int getStickValue(RcReceiverSignal * receiver) {
  // since the stick is centered, its pwm value should be 1500
  unsigned long pwmValue = receiver->getPwmValue();
  return pwmValue - CENTER_STICK_PWM;
}

void drive() {

  const int throttleValue = getStickValue(&receiver_throttle);
  const int steeringValue = getStickValue(&receiver_steering);

  const int speed = map(abs(throttleValue), MIN_STICK_VALUE, MAX_STICK_VALUE, MIN_PWM, MAX_PWM);

  const int steeringSlowdown = fscale(MIN_STICK_VALUE, MAX_STICK_VALUE, MIN_PWM, MAX_PWM, abs(steeringValue), -3);
  const int steeringSpeed = speed - steeringSlowdown;

  // the RC signal oscillates from -50 to +50
  if (throttleValue > -DEADBAND and throttleValue < DEADBAND) {
    // stop motor
    motorA.stop();
    motorB.stop();

  } else if (throttleValue > DEADBAND) {
    if (steeringValue < 0) {
        motorA.driveForward(speed);
        motorB.driveForward(steeringSpeed);
      } else {
        motorA.driveForward(steeringSpeed);
        motorB.driveForward(speed);
      }

  } else if (throttleValue < -DEADBAND) {
    // accelerate backwards
    motorA.driveBackwards(speed);
    motorB.driveBackwards(speed);
  }
}

void loop()
{
  if (receiver_throttle.hasChanged() || receiver_steering.hasChanged()) {
    drive();
  }
}
