#include <Arduino.h>
#include <math.h>
#include "motor.h"

Motor::Motor(short pwmPin, short directionPin):
  pwmPin(pwmPin),
  directionPin(directionPin)
  {};

void Motor::driveForward(int speed) {
  digitalWrite(directionPin, LOW);
  analogWrite(pwmPin, clamp(speed, 0 , 255));

  #ifdef DEBUG
    Serial.println("FORWARD");
    Serial.println(speed);
  #endif
};

void Motor::driveBackward(int speed) {
  digitalWrite(directionPin, HIGH);
  analogWrite(pwmPin, clamp(255 - speed, 0, 255));

  #ifdef DEBUG
    Serial.println("BACK");
    Serial.println(255 - speed);
  #endif
};

void Motor::stop() {
  digitalWrite(directionPin, LOW);
  digitalWrite(pwmPin, LOW);
};