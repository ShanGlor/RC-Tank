class Motor {
  public:
    Motor(short pwmPin, short directionPin);
    void driveForward(short speed);
    void driveBackwards(short speed);
    void stop();

  private:
    short pwmPin;
    short directionPin;
};