#include "SerialControl.h"

SerialControl::SerialControl(Config *config, AccelStepper *accelstepper){
    cfg = config;
    stepper = accelstepper;
    Serial.begin(9600);
    Serial.setTimeout(5000);
};

int SerialControl::parseConfigString(String *str)
{
    String subs = str->substring(str->lastIndexOf("=") + 1);
    // Serial.print("substring: "); Serial.println(subs);
    return subs.toInt();
}
float SerialControl::parseConfigStringF32(String *str)
{
    String subs = str->substring(str->lastIndexOf("=") + 1);
    // Serial.print("substring: "); Serial.println(subs);
    return subs.toFloat();
}

void SerialControl::parseCommand(){
}