#include <stdio.h>
#include <stdlib.h>
#include <Arduino.h>

//RcReceiverSignal library has a dependency to PinChangeInt library.
#include <PinChangeInt.h>
#include <RcReceiverSignal.h>

#define PIN_RC_STEERING 3
#define PIN_RC_THROTTLE 2

#define MOTOR_A_PWM 6 // supports PWM
#define MOTOR_A_DIRECTION 7 // does not support PWM

DECLARE_RECEIVER_SIGNAL(receiver_throttle);
DECLARE_RECEIVER_SIGNAL(receiver_steering);

#define ENABLE_SERIAL_OUTPUT

#define DEADBAND 15

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

  #ifdef ENABLE_SERIAL_OUTPUT
    Serial.begin(115200);
    Serial.println("ready");
  #endif


  receiver_throttle_setup(PIN_RC_THROTTLE);
  receiver_steering_setup(PIN_RC_STEERING);
}

void printReceiver(const char * iName, RcReceiverSignal * iReceiverSignal)
{
  unsigned long pwmValue = iReceiverSignal->getPwmValue();
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

void drive(RcReceiverSignal * receiver_throttle) {
  const unsigned long pwmValue = receiver_throttle->getPwmValue();
  const short throttleValue = receiver_throttle->getSignalValue(pwmValue);

  // throttleValue range [-125, 125], 0 is center stick position
  const int speed = map(abs(throttleValue) + DEADBAND, 0, 125, 0, 255);
  Serial.println(throttleValue);

  // the RC signal oscillates from -10 to +10
  if (throttleValue > -DEADBAND and throttleValue < DEADBAND) {
    // stop motor
    digitalWrite(MOTOR_A_DIRECTION, LOW);
    digitalWrite(MOTOR_A_PWM, LOW);

  } else if (throttleValue > DEADBAND) {
    // accelerate forward
    digitalWrite(MOTOR_A_DIRECTION, HIGH);
    analogWrite(MOTOR_A_PWM, 255 - speed);

  } else if (throttleValue < -DEADBAND) {
    // accelerate backwards
    digitalWrite(MOTOR_A_DIRECTION, LOW);
    analogWrite(MOTOR_A_PWM, speed);
  }
}

void loop()
{

  if (receiver_throttle.hasChanged()) {
    drive(&receiver_throttle);
  }
}
