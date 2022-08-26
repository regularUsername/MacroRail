#include <Arduino.h>
#include <AccelStepper.h>
#include <ClickEncoder.h>
#include "lcdmenu.h"
#include <TimerOne.h>

const uint8_t SHUTTER_PIN = A3;
const uint8_t STEPPER_PIN1 = 9;
const uint8_t STEPPER_PIN2 = 8;
const uint8_t STEPPER_PIN3 = 7;
const uint8_t STEPPER_PIN4 = 6;
const uint8_t ENCODER_A = A1;
const uint8_t ENCODER_B = A0;
const uint8_t ENCODER_BTN = A2;
const uint8_t LCD_DC = 5;
const uint8_t LCD_CS = 4;
const uint8_t LCD_RST = 3;
// because we are using Hardware SPI connect D13 to CLK and D11 to DIN
// D12 and LCD_CS are not needed but are still Read and Written to during
// SPI transfer

int8_t readRotaryEncoder();
void timerIsr();

ClickEncoder *encoder;
int16_t last, value;

LCDMenu lcdmenu(LCD_DC, LCD_CS, LCD_RST);

AccelStepper stepper(AccelStepper::FULL4WIRE, STEPPER_PIN1, STEPPER_PIN2, STEPPER_PIN3, STEPPER_PIN4);

void takePhoto(int exposure_time)
{
    delay(200);
    digitalWrite(SHUTTER_PIN, HIGH);
    delay(200);
    digitalWrite(SHUTTER_PIN, LOW);
    delay(exposure_time);
}

enum State
{
    READY,
    BRACKETING,
    HOMING,
    DRYRUN,
    JOGMODE,
};

int targetPos;
int i;
State state;

int stepsPerMM = 183;
int totalDistance;
int interval;
bool jogFine = false;

// used for backlash compensation
const uint8_t BACKLASH_STEPS = 50; // my version of the 28BYJ-48 stepper has roughly 50 steps of backlash
int8_t lastDirection = 0;
long stepperLastPosition = 0;

void setup()
{
    lcdmenu.initialize();
    lcdmenu.splashscreen();
    delay(2000);
    encoder = new ClickEncoder(ENCODER_A, ENCODER_B, ENCODER_BTN);
    encoder->setAccelerationEnabled(true);

    Timer1.initialize(1000);
    Timer1.attachInterrupt(timerIsr);

    last = encoder->getValue();

    targetPos = 0;
    i = 0;
    state = READY;

    stepper.setMaxSpeed(400);
    stepper.setAcceleration(150);

    pinMode(SHUTTER_PIN, OUTPUT);
}

void loop()
{
    switch (state)
    {
    case READY:
    {
        auto a = lcdmenu.getMenuAction();
        if (a == LCDMenu::START)
        {
            state = BRACKETING;
            totalDistance = lcdmenu.getDistance() * stepsPerMM;
            interval = (lcdmenu.getInterval() * stepsPerMM) / lcdmenu.interval_div;

            lcdmenu.drawText("Bracketing");
            targetPos = 0;
            if (!lcdmenu.getForward())
            {
                interval = -interval;
            }
            stepper.moveTo(targetPos);
            delay(1000);
        }
        else if (a == LCDMenu::DRYRUN)
        {
            state = DRYRUN;
            lcdmenu.drawText("Dry run");
            totalDistance = lcdmenu.getDistance() * stepsPerMM;
            totalDistance = lcdmenu.getForward() ? totalDistance : -totalDistance;

            stepper.moveTo(totalDistance);
            // wait until button is released
            while (encoder->getButton() != ClickEncoder::Open);
        }
        else if (a == LCDMenu::JOGMODE)
        {
            state = JOGMODE;
            targetPos = stepper.currentPosition();
            lcdmenu.drawText("Jog Mode", jogFine ? "0.1/step" : "1mm/step");
        }
        else
        {
            // do menu stuff
            lcdmenu.drawMenu();
            int8_t dir = readRotaryEncoder();
            switch (encoder->getButton())
            {
            case ClickEncoder::Clicked:
                lcdmenu.select();
                break;
            case ClickEncoder::Held:
                lcdmenu.select(true);
                break;
            }
            lcdmenu.navigate(dir);
        }
    }
    break;
    case BRACKETING:
        stepper.run();
        // cancel bracketing if button is held
        if (encoder->getButton() == ClickEncoder::Held)
        {
            state = HOMING;
            targetPos = 0;
            i = 0;
            lcdmenu.drawText("Cancelled", "Homing");
            stepper.moveTo(targetPos);
        }
        if (stepper.currentPosition() == targetPos)
        {
            i++;
            char strBuf[16];
            snprintf(strBuf, sizeof(strBuf), "%d of %d", i, totalDistance / abs(interval) + 1);
            lcdmenu.drawText("Bracketing", strBuf);
            takePhoto(lcdmenu.getExposureTime() * 1000);
            if (abs(targetPos + interval) > totalDistance)
            {
                state = HOMING;
                targetPos = 0;
                i = 0;
                lcdmenu.drawText("Homing");
            }
            else
            {
                targetPos += interval;
            }
            stepper.moveTo(targetPos);
        }
        break;
    case DRYRUN:
        stepper.run();
        if (abs(stepper.currentPosition()) >= abs(totalDistance))
        {
            state = HOMING;
            lcdmenu.drawText("Homing");
            stepper.moveTo(0);
        }

        break;
    case HOMING:
        stepper.run();
        if (stepper.currentPosition() == stepper.targetPosition())
        {
            state = READY;
            stepper.disableOutputs();
        }
        break;
    case JOGMODE:
        stepper.run();

        // experimental backlash compensation
        // get the direction the stepper currently moves
        int8_t currentDirection = 0;
        long currentPosition = stepper.currentPosition();
        if (currentPosition>stepperLastPosition){
            currentDirection = 1;
        } else if(currentPosition<stepperLastPosition){
            currentDirection = -1;
        }
        stepperLastPosition=currentPosition;
        
        // check if the stepper actually moved
        if(currentDirection!=0){
            //on direction change add backlash_steps
            if (currentDirection != lastDirection){
                targetPos+=BACKLASH_STEPS*currentDirection;
            }
            lastDirection = currentDirection;
        }
        //backlash compensation end


        int x = readRotaryEncoder();
        if(x!=0){
            targetPos += jogFine ? x * stepsPerMM / 10 : x * stepsPerMM;
        }
        stepper.moveTo(targetPos);
        auto b = encoder->getButton();
        if (stepper.currentPosition() == stepper.targetPosition() && b == ClickEncoder::Clicked)
        {
            state = READY;
            stepper.disableOutputs();
            stepper.setCurrentPosition(0);
        }
        else if (b == ClickEncoder::Held)
        {
            jogFine = !jogFine;
            lcdmenu.drawText("Jog Mode", jogFine ? "0.1mm/step" : "1mm/step");
            // wait until button is released
            while (encoder->getButton() != ClickEncoder::Open);
        }
    }
}

void timerIsr()
{
    encoder->service();
}

int8_t readRotaryEncoder()
{
    value += encoder->getValue();
    value += encoder->getValue();
    int x = value - last;
    last = value;
    return x;
}