#include <Arduino.h>
#include <AccelStepper.h>
#include <ClickEncoder.h>
#include "lcdmenu.h"
#include <TimerOne.h>

#define SHUTTER_PIN  A7
#define STEPPER_PIN1 9
#define STEPPER_PIN2 8
#define STEPPER_PIN3 7
#define STEPPER_PIN4 6


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
char strBuf[16];

int stepsPerMM = 183;
int totalDistance;
int interval;

void setup() {

    lcdmenu.initialize();
    lcdmenu.splashscreen();
    delay(2000);
    encoder = new ClickEncoder(A1,A0,A2);
    encoder->setAccelerationEnabled(true);

    Timer1.initialize(1000);
    Timer1.attachInterrupt(timerIsr);

    last = encoder->getValue();

    targetPos = 0;
    i = 0;
    state = READY;

    stepper.setMaxSpeed(250);
    stepper.setAcceleration(100);

    pinMode(SHUTTER_PIN,OUTPUT);
}

void loop() {
    switch (state) {
    case READY:
            if(lcdmenu.checkStartFlag()){
                state = BRACKETING;
                totalDistance = lcdmenu.getDistance()*stepsPerMM;
                interval = lcdmenu.getInterval()*stepsPerMM;

                lcdmenu.drawText("Bracketing","");
                targetPos = 0;
                if (lcdmenu.getForward()){
                    interval = abs(interval);
                } else {
                    interval = -abs(interval);
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
        // cancel bracking if button is held
        if(encoder->getButton() == ClickEncoder::Held){ 
            state = HOMING;
            targetPos = 0;
            i = 0;
            lcdmenu.drawText("Cancelled","Homing");
            stepper.moveTo(targetPos);
        }
        if (stepper.currentPosition() == targetPos) {
            i++;
            snprintf(strBuf,sizeof(strBuf),"%d of %d",i,totalDistance/abs(interval)+2);
            lcdmenu.drawText("Bracketing",strBuf);
            // Serial.print("taking picture "); Serial.print(i); Serial.print(" of "); Serial.println(config.totalDistance/config.interval+2);
            takePhoto(lcdmenu.getExposureTime());
            if (abs(targetPos)>totalDistance){
                state = HOMING;
                targetPos = 0;
                i = 0;
                lcdmenu.drawText("Homing","");
            } else {
                targetPos+=interval;
            }
            stepper.moveTo(targetPos);
        }
        break;
    case PREVIEW:
        stepper.run();
        if(abs(stepper.currentPosition())>=totalDistance){
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