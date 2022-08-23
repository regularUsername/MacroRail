#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

class LCDMenu
{
private:
    Adafruit_PCD8544 display;
    void displayIntMenuPage(const char *, int, const char * = nullptr);
    void displayStringMenuPage(const char *menuItem, const char *value);
    void displayMenuItem(const char *item, int position, boolean selected);
    void displayFractionalIntMenuPage(const char *menuItem, int value, const char *unit);

    const uint8_t default_contrast = 55;
    uint8_t contrast = default_contrast;
    bool forward = true;
    uint8_t distance = 10;
    uint8_t exposureTime = 1;
    uint8_t interval = 1;

    // TODO use enum instead of flags?
    bool startFlag = false;
    bool dryRunFlag = false;
    bool jogmodeFlag = false;

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

    // divisor for interval setting precision
    //  e.g. 1=1mm, 2=0.5mm, 4=0.25mm, 10=0.1mm
    const uint8_t interval_div = 4;

    uint8_t getDistance();
    uint8_t getExposureTime();
    uint8_t getInterval();
    bool getForward();
    bool checkStartFlag();
    bool checkDryRunFlag();
    bool checkJogmodeFlag();
};
