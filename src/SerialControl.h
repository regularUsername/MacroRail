
#include <Arduino.h>
#include <Config.h>
#include <AccelStepper.h>

class SerialControl
{
public:
    SerialControl(Config *config, AccelStepper *accelstepper);
    void parseCommand();

private:
    Config *cfg;
    AccelStepper *stepper;

    int parseConfigString(String *str);
    float parseConfigStringF32(String *str);
};