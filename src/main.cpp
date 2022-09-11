#include <Arduino.h>
#include <AccelStepper.h>
#include <ClickEncoder.h>
#include "lcdmenu.h"
#include <TimerOne.h>
#include "config.h"


void timerIsr();

ClickEncoder *encoder;

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

int i;
State state;

int totalDistance;
int interval;
bool jogFine = false;

// used for backlash compensation
int8_t lastDirection = -1;
long stepperLastPosition = 0;

void setup()
{
    lcdmenu.initialize();
    lcdmenu.splashscreen();
    delay(2000);

    //setup RotaryEncoder
    encoder = new ClickEncoder(ENCODER_A, ENCODER_B, ENCODER_BTN);
    encoder->setAccelerationEnabled(true);
    Timer1.initialize(1000);
    Timer1.attachInterrupt(timerIsr);

    i = 0;
    state = READY;

    stepper.setMaxSpeed(DEFAULT_MAXSPEED);
    stepper.setAcceleration(DEFAULT_ACCELERATION);

    pinMode(SHUTTER_PIN, OUTPUT);
}

void loop()
{
    switch (state)
    {
    case READY:
    {
        LCDMenu::menuAction a = lcdmenu.getMenuAction();
        if (a == LCDMenu::START)
        {
            state = BRACKETING;
            totalDistance = lcdmenu.getDistance() * STEPS_PER_MM;
            interval = (lcdmenu.getInterval() * STEPS_PER_MM) / INTERVAL_DIV;

            lcdmenu.drawText("Bracketing");

            // backlash compensation
            int8_t dir = lcdmenu.getDirection();
            interval = interval * dir;
            if (lastDirection != dir)
            {
                stepper.moveTo(stepper.currentPosition() + BACKLASH_STEPS * dir);
            }
            delay(1000);
        }
        else if (a == LCDMenu::DRYRUN)
        {
            state = DRYRUN;
            lcdmenu.drawText("Dry run");
            totalDistance = lcdmenu.getDistance() * STEPS_PER_MM;

            int8_t dir = lcdmenu.getDirection();
            if (lastDirection != dir)
            {
                totalDistance += BACKLASH_STEPS;
            }

            stepper.moveTo(totalDistance * dir);
            // wait until button is released before proceeding
            while (encoder->getButton() != ClickEncoder::Open);
        }
        else if (a == LCDMenu::JOGMODE)
        {
            state = JOGMODE;
            stepperLastPosition = 0;
            lcdmenu.drawText("Jog Mode", jogFine ? "0.1/step" : "1mm/step");
        }
        else
        {
            // do menu stuff
            lcdmenu.drawMenu();
            switch (encoder->getButton())
            {
            case ClickEncoder::Clicked:
                lcdmenu.select();
                break;
            case ClickEncoder::Held:
                lcdmenu.select(true);
                break;
            default:
                break;
            }
            lcdmenu.navigate(encoder->getValue());
        }
    }
    break;
    case BRACKETING:
        stepper.run();
        // cancel bracketing if button is held
        if (encoder->getButton() == ClickEncoder::Held)
        {
            state = HOMING;
            i = 0;
            lcdmenu.drawText("Cancelled", "Homing");

            // backlash compensation
            if (lastDirection != lcdmenu.getDirection())
            {
                stepper.moveTo(0);
            }
            else
            {
                stepper.moveTo(-BACKLASH_STEPS * lcdmenu.getDirection());
            }
        }
        if (stepper.currentPosition() == stepper.targetPosition())
        {
            i++;
            char strBuf[16];
            snprintf(strBuf, sizeof(strBuf), "%d of %d", i, totalDistance / abs(interval) + 1);
            lcdmenu.drawText("Bracketing", strBuf);
            takePhoto(lcdmenu.getExposureTime() * 1000);
            if (i >= totalDistance / abs(interval) + 1)
            {
                state = HOMING;

                // backlash compensation
                if (lastDirection != lcdmenu.getDirection())
                {
                    stepper.moveTo(0);
                }
                else
                {
                    stepper.moveTo(-BACKLASH_STEPS * lcdmenu.getDirection());
                }
                i = 0;
                lcdmenu.drawText("Homing");
            }
            else
            {
                stepper.moveTo(stepper.targetPosition() + interval);
            }
        }
        break;
    case DRYRUN:
        stepper.run();
        if (stepper.currentPosition() == stepper.targetPosition())
        {
            state = HOMING;
            lcdmenu.drawText("Homing");

            // backlash compensation
            if (lastDirection != lcdmenu.getDirection())
            {
                stepper.moveTo(0);
            }
            else
            {
                stepper.moveTo(-BACKLASH_STEPS * lcdmenu.getDirection());
            }
        }
        break;
    case HOMING:
        stepper.run();
        if (stepper.currentPosition() == stepper.targetPosition())
        {
            state = READY;
            delay(500);
            lastDirection = -lcdmenu.getDirection();
            stepperLastPosition = 0;
            stepper.setCurrentPosition(0);
            stepper.disableOutputs();
        }
        break;
    case JOGMODE:
        stepper.run();

        // backlash compensation
        // get the direction the stepper currently moves
        int8_t currentDirection = 0;
        long currentPosition = stepper.currentPosition();
        if (currentPosition > stepperLastPosition)
        {
            currentDirection = 1;
        }
        else if (currentPosition < stepperLastPosition)
        {
            currentDirection = -1;
        }
        stepperLastPosition = currentPosition;
        // on direction change add backlash_steps
        if (currentDirection != 0)
        {
            if (currentDirection != lastDirection)
            {
                stepper.moveTo(stepper.targetPosition() + BACKLASH_STEPS * currentDirection);
            }
            lastDirection = currentDirection;
        }
        int x = encoder->getValue();
        if (x != 0)
        {
            int relative = jogFine ? x * STEPS_PER_MM / 10 : x * STEPS_PER_MM;
            stepper.moveTo(stepper.targetPosition() + relative);
        }
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
            // wait until button is released before proceeding
            while (encoder->getButton() != ClickEncoder::Open);
        }
    }
}

void timerIsr()
{
    encoder->service();
}