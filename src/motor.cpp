#include <Arduino.h>
#include <math.h>
#include "motor.h"

#define clamp(x) (min(255, max(x, 0)))

Motor::Motor(short pwmPin, short directionPin):
  pwmPin(pwmPin),
  directionPin(directionPin)
  {};

void Motor::driveForward(int speed) {
  digitalWrite(directionPin, LOW);
  analogWrite(pwmPin, clamp(speed));

  #ifdef DEBUG
    Serial.println("FORWARD");
    Serial.println(speed);
  #endif
};

void Motor::driveBackwards(int speed) {
  digitalWrite(directionPin, HIGH);
  analogWrite(pwmPin, clamp(255 - speed));

  #ifdef DEBUG
    Serial.println("BACK");
    Serial.println(255 - speed);
  #endif
};

void Motor::stop() {
  digitalWrite(directionPin, LOW);
  digitalWrite(pwmPin, LOW);
};