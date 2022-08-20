#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

class LCDMenu
{
private:
    Adafruit_PCD8544 display;
    void displayIntMenuPage(String menuItem, int value, String unit = "value");
    void displayStringMenuPage(String menuItem, String value);
    void displayMenuItem(String item, int position, boolean selected);

    const uint8_t default_contrast = 55;
    uint8_t contrast = default_contrast;
    bool forward = true;
    uint8_t distance = 10;
    uint8_t exposureTime = 1;
    uint8_t interval = 1;
    bool startFlag = false;

    uint8_t page = 1;
    uint8_t pos = 0;
    uint8_t window = 0;

public:
    LCDMenu();
    void initialize();
    void splashscreen();
    void setContrast(uint8_t);
    void resetDefaults();

    void draw();
    void select();
    void navigate(int8_t);

    uint8_t getDistance();
    uint8_t getExposureTime();
    uint8_t getInterval();
    bool getForward();
    bool checkStartFlag(); // check and reset starflag
};
