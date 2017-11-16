#include <Arduino.h>
#include <Math.h>
#include "motor.h"

#define clamp(x) (min(255, max(x, 0)))

Motor::Motor(short pwmPin, short directionPin) {
  pwmPin = pwmPin;
  directionPin = directionPin;
};

void Motor::driveForward(short speed) {
  digitalWrite(directionPin, LOW);
  analogWrite(pwmPin, clamp(255 - speed));
  Serial.println("FORWARD");
  Serial.println(speed);
};

void Motor::driveBackwards(short speed) {
  digitalWrite(directionPin, HIGH);
  analogWrite(pwmPin, clamp(speed));
};

void Motor::stop() {
  digitalWrite(directionPin, LOW);
  digitalWrite(pwmPin, LOW);
};