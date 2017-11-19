#define clamp(x, minValue, maxValue) (min(maxValue, max(x, minValue)))

class Motor {
  public:
    Motor(short pwmPin, short directionPin);
    void driveForward(int speed);
    void driveBackward(int speed);
    void stop();

  private:
    short pwmPin;
    short directionPin;
};