#include "lcdmenu.h"

// TODO Strings Objekte austauschen (dynamic allocation meiden)
String menuItems[] = {
    "Start",
    "Distance(mm)",
    "Interval(mm)",
    "ExposureTime",
    "Direction -->",
    "Contrast",
    "Reset"};

// immer synchron mit menuItems halten
enum
{
  START,
  DISTANCE,
  INTERVAL,
  EXPOSURE,
  DIRECTION,
  CONTRAST,
  RESET
};

const uint8_t menuItemCount = sizeof(menuItems) / sizeof(String);

LCDMenu::LCDMenu() : display(5, 4, 3) {}

// call in void setup(){}
void LCDMenu::initialize()
{
  display.begin(contrast);
  display.clearDisplay();
}

void LCDMenu::splashscreen()
{
  display.setTextSize(1);
  display.clearDisplay();
  display.setTextColor(BLACK, WHITE);
  display.setCursor(5, 10);
  display.print("Macro Rail");
  display.drawFastHLine(0, 20, 83, BLACK);
  display.setCursor(5, 25);
  display.print("v 0.1");
  display.display();
}

void LCDMenu::setContrast(uint8_t contrast)
{
  this->contrast = contrast;
  display.setContrast(contrast);
  display.display();
}

void LCDMenu::resetDefaults()
{
  contrast = default_contrast;
  distance = 50;
  setContrast(contrast);
  forward = true;
  menuItems[4] = "Direction -->";
  // turnBacklightOn();
}

void LCDMenu::drawMenu()
{
  if (page == 1)
  {
    display.setTextSize(1);
    display.clearDisplay();
    display.setTextColor(BLACK, WHITE);
    display.setCursor(15, 0);
    display.print("MAIN MENU");
    display.drawFastHLine(0, 10, 83, BLACK);

    displayMenuItem(menuItems[window], 15, pos - window == 0);
    displayMenuItem(menuItems[window + 1], 25, pos - (window + 1) == 0);
    displayMenuItem(menuItems[window + 2], 35, pos - (window + 2) == 0);
    display.display();
  }
  else if (page == 2)
  {
    auto item = &menuItems[pos];
    switch (pos)
    {
    case CONTRAST:
      displayIntMenuPage(*item, contrast);
      break;
    case DISTANCE:
      displayIntMenuPage(*item, distance, "mm");
      break;
    case INTERVAL:
      displayIntMenuPage(*item, interval, "mm");
      break;
    case EXPOSURE:
      displayIntMenuPage(*item, exposureTime, "sec");
      break;
    }
  }
}

// TODO add display FixedPoint Menupage method
// for exposuretimes and intervals smaller than 1mm
void LCDMenu::displayIntMenuPage(String menuItem, int value, String unit)
{
  display.setTextSize(1);
  display.clearDisplay();
  display.setTextColor(BLACK, WHITE);
  display.setCursor(5, 0);
  display.print(menuItem);
  display.drawFastHLine(0, 10, 83, BLACK);
  display.setCursor(5, 15);
  display.print(unit);
  display.setTextSize(2);
  display.setCursor(5, 25);
  display.print(value);
  display.setTextSize(2);
  display.display();
}

void LCDMenu::displayStringMenuPage(String menuItem, String value)
{
  display.setTextSize(1);
  display.clearDisplay();
  display.setTextColor(BLACK, WHITE);
  display.setCursor(15, 0);
  display.print(menuItem);
  display.drawFastHLine(0, 10, 83, BLACK);
  display.setCursor(5, 15);
  display.print("Value");
  display.setTextSize(2);
  display.setCursor(5, 25);
  display.print(value);
  display.setTextSize(2);
  display.display();
}

void LCDMenu::displayMenuItem(String item, int position, boolean selected)
{
  if (selected)
  {
    display.setTextColor(WHITE, BLACK);
  }
  else
  {
    display.setTextColor(BLACK, WHITE);
  }
  display.setCursor(0, position);
  display.print(item);
}

void LCDMenu::navigate(int8_t dir)
{
  if (page == 1)
  {
    pos = constrain(pos + dir, 0, menuItemCount - 1);

    if (pos > window + 2)
    {
      window++;
    }
    else if (pos < window)
    {
      window--;
    }
  }
  else if (page == 2)
  {
    switch (pos)
    { // nur die einträge mit submenu
    case CONTRAST:
      contrast += dir;
      setContrast(contrast);
      break;
    case DISTANCE:
      distance += dir;
      break;
    case EXPOSURE:
      exposureTime = constrain(exposureTime + dir, 0, 255);
      break;
    case INTERVAL:
      interval += dir;
      break;
    }
  }
}

void LCDMenu::select()
{
  if (page == 1)
  { // main menu
    switch (pos)
    {
    case START:
      startFlag = true;
      break;
    case DIRECTION:
      forward = !forward;
      menuItems[4] = forward ? "Direction -->" : "Direction <--";
      break;
    case RESET:
      resetDefaults();
      break;
    default: // einträge mit submenu
      page = 2;
    }
  }
  else if (page == 2)
  { // if in submenu return to main menu
    page = 1;
  }
}

uint8_t LCDMenu::getDistance()
{
  return distance;
}
uint8_t LCDMenu::getExposureTime()
{
  return exposureTime;
}
uint8_t LCDMenu::getInterval()
{
  return interval;
}
bool LCDMenu::getForward()
{
  return forward;
}
bool LCDMenu::checkStartFlag()
{
  if (startFlag)
  {
    startFlag = false;
    return true;
  }
  return false;
}