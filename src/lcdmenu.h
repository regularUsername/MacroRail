#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include "config.h"

class LCDMenu
{
public:
    typedef enum
    {
        DO_NOTHING,
        START,
        DRYRUN,
        JOGMODE,
    } menuAction;

private:
    Adafruit_PCD8544 display;
    void displayHeader(const char *text);
    void displayIntMenuPage(const char *, int, const char * = nullptr);
    void displayStringMenuPage(const char *menuItem, const char *value);
    void displayMenuItem(const char *item, int position, boolean selected);
    void displayFractionalIntMenuPage(const char *menuItem, int value, const char *unit);

    uint8_t contrast = DEFAULT_CONTRAST;
    int8_t direction = 1;
    uint8_t distance = 10;
    uint8_t exposureTime = 1;
    uint8_t interval = 1;

    menuAction action = DO_NOTHING;

    uint8_t page = 1;
    uint8_t pos = 0;
    uint8_t window = 0;

public:
    LCDMenu(uint8_t, uint8_t, uint8_t);
    void initialize();
    void splashscreen();
    void setContrast(uint8_t);
    void resetDefaults();

    void drawMenu();
    void select(bool longpress = false);
    void navigate(int8_t);
    void drawText(const char *, const char * = nullptr);


    uint8_t getDistance()
    {
        return distance;
    }
    uint8_t getExposureTime()
    {
        return exposureTime;
    }
    uint8_t getInterval()
    {
        return interval;
    }
    menuAction getMenuAction()
    {
        auto x = action;
        action = DO_NOTHING;
        return x;
    }
    // 1 = forward, -1 = backward
    int8_t getDirection()
    {
        return direction;
    }
};
