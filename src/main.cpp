#include <Arduino.h>
#include <AccelStepper.h>
#include <ClickEncoder.h>
#include "lcdmenu.h"
#include <TimerOne.h>

#include <Config.h>


#define SHUTTER_PIN  A7
#define STEPPER_PIN1 A3
#define STEPPER_PIN2 A4
#define STEPPER_PIN3 A5
#define STEPPER_PIN4 A6


int8_t readRotaryEncoder();
void timerIsr();

ClickEncoder *encoder;
int16_t last, value;

LCDMenu lcdmenu;

AccelStepper stepper(AccelStepper::FULL4WIRE,STEPPER_PIN1,STEPPER_PIN2,STEPPER_PIN3,STEPPER_PIN4);

void takePhoto(int exposure_time) {
    delay(200);
    digitalWrite(SHUTTER_PIN, HIGH);
    delay(200);
    digitalWrite(SHUTTER_PIN, LOW);
    delay(exposure_time);
}

enum State {
    READY,
    BRACKETING,
    HOMING,
    PREVIEW,
};

int targetPos;
int i;
State state;
Config config;

void setup() {

    lcdmenu.initialize();
    lcdmenu.splashscreen();
    delay(2000);
    encoder = new ClickEncoder(A1,A0,A2);
    encoder->setAccelerationEnabled(true);

    Timer1.initialize(1000);
    Timer1.attachInterrupt(timerIsr);

    last = encoder->getValue();


    config = {};
    config.stepsPerMM = 183; // 183 bei fullstepping auf novoflex
    config.totalDistance = 5*config.stepsPerMM;
    config.interval = 0.5*config.stepsPerMM; 
    config.backwards = false;
    config.exposureTime = 1000;
    if (config.backwards){
        config.interval = -config.interval;
    }

    targetPos = 0;
    i = 0;
    state = READY;

    Serial.begin(9600);
    Serial.setTimeout(5000);

    stepper.setMaxSpeed(500);
    stepper.setAcceleration(200);

    pinMode(SHUTTER_PIN,OUTPUT);
}

void loop() {
    switch (state) {
    case READY:
            if(lcdmenu.checkStartFlag()){
                state = BRACKETING;
                config.totalDistance = lcdmenu.getDistance()*config.stepsPerMM;
                config.interval = lcdmenu.getInterval()*config.stepsPerMM;
                config.exposureTime = lcdmenu.getExposureTime();

                targetPos = 0;
                if (lcdmenu.getForward()){
                    config.interval = abs(config.interval);
                } else {
                    config.interval = -abs(config.interval);
                }
                stepper.moveTo(targetPos);
                delay(2000);
            } else {
                // do menu stuff
                lcdmenu.drawMenu();
                int8_t dir = readRotaryEncoder();
                if(encoder->getButton() == ClickEncoder::Clicked){
                    lcdmenu.select();
                }
                lcdmenu.navigate(dir);
            }
        break;
    case BRACKETING:
        stepper.run();
        if (stepper.currentPosition() == targetPos) {
            i++;
            // Serial.print("taking picture "); Serial.print(i); Serial.print(" of "); Serial.println(config.totalDistance/config.interval+2);
            takePhoto(config.exposureTime);
            if (abs(targetPos)>config.totalDistance){
                state = HOMING;
                targetPos = 0;
                i = 0;
            } else {
                targetPos+=config.interval;
            }
            stepper.moveTo(targetPos);
        }
        break;
    case PREVIEW:
        stepper.run();
        if(abs(stepper.currentPosition())>=config.totalDistance){
            state = HOMING;
            stepper.moveTo(0);
        }

        break;
    case HOMING:
        stepper.run();
        if(stepper.currentPosition() == 0){
            state = READY;
            stepper.disableOutputs();
        }
        break;
    }
}

void timerIsr() {
  encoder->service();
}

int8_t readRotaryEncoder()
{
  value += encoder->getValue();
  if (value > last) {
    last = value;
    return 1;
  }else if (value < last) {
    last = value;
    return -1;
  }
  return 0;
}