class Motor {
  public:
    Motor(short pwmPin, short directionPin);
    void driveForward(int speed);
    void driveBackwards(int speed);
    void stop();

  private:
    short pwmPin;
    short directionPin;
};