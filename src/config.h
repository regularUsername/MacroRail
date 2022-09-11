#pragma once
#include <pins_arduino.h>

/* Pins */
const uint8_t SHUTTER_PIN = A3;
const uint8_t STEPPER_PIN1 = 9;
const uint8_t STEPPER_PIN2 = 8;
const uint8_t STEPPER_PIN3 = 7;
const uint8_t STEPPER_PIN4 = 6;
//rotary encoder
const uint8_t ENCODER_A = A1;
const uint8_t ENCODER_B = A0;
const uint8_t ENCODER_BTN = A2;
// display
const uint8_t LCD_DC = 5;
const uint8_t LCD_CS = 4;
const uint8_t LCD_RST = 3;
// because we are using Hardware SPI connect D13 to CLK and D11 to DIN
// D12 and LCD_CS are not needed but are still Read and Written to during
// SPI transfer


const uint8_t BACKLASH_STEPS = 47; // my version of the 28BYJ-48 stepper has roughly 50 steps of backlash


// stepper configuration
const unsigned int STEPS_PER_MM = 183;
const float DEFAULT_MAXSPEED = 400;
const float DEFAULT_ACCELERATION = 150;


//display
const uint8_t DEFAULT_CONTRAST = 55;

// set precision for the interval setting menu
//  e.g. 1=1mm, 2=0.5mm, 4=0.25mm, 10=0.1mm
const uint8_t INTERVAL_DIV = 10;

const uint8_t DEFAULT_DISTANCE = 10; //mm
const uint8_t DEFAULT_INTERVAL = 1; //mm