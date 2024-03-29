#include "lcdmenu.h"

const char *menuItems[] = {
    "Start",
    "Distance(mm)",
    "Interval(mm)",
    "ExposureTime",
    "Direction -->",
    "Jog Mode",
    "Contrast",
    "Reset"};

// immer synchron mit menuItems halten
typedef enum
{
  START,
  DISTANCE,
  INTERVAL,
  EXPOSURE,
  DIRECTION,
  JOGMODE,
  CONTRAST,
  RESET
} menuItems_e;

const uint8_t menuItemCount = sizeof(menuItems) / sizeof(menuItems[0]);

LCDMenu::LCDMenu(uint8_t dc, uint8_t cs, uint8_t rst) : display(dc, cs, rst)
{
  interval *= INTERVAL_DIV;
}

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
  distance = DEFAULT_DISTANCE;
  interval = DEFAULT_INTERVAL;
  setContrast(DEFAULT_CONTRAST);
  direction = 1;
  menuItems[4] = "Direction -->";
}

void LCDMenu::drawMenu()
{
  if (page == 1)
  {
    displayHeader("Macro Rail");

    for (uint8_t i = 0; i < 4; i++)
    {
      displayMenuItem(menuItems[window + i], 10 * (i + 1), pos - (window + i) == 0);
    }
    display.display();
  }
  else if (page == 2)
  {
    auto item = &menuItems[pos];
    switch (pos)
    {
    case menuItems_e::CONTRAST:
      displayIntMenuPage(*item, contrast);
      break;
    case menuItems_e::DISTANCE:
      displayIntMenuPage(*item, distance, "mm");
      break;
    case menuItems_e::INTERVAL:
      displayFractionalIntMenuPage(*item, interval, "mm");
      break;
    case menuItems_e::EXPOSURE:
      displayIntMenuPage(*item, exposureTime, "sec");
      break;
    }
  }
}


void LCDMenu::displayHeader(const char *text){
  display.setTextSize(1);
  display.clearDisplay();
  display.setTextColor(BLACK, WHITE);
  display.setCursor(5, 0);
  display.print(text);
  display.drawFastHLine(0, 8, 83, BLACK);
  display.setCursor(5, 15);
}

void LCDMenu::displayIntMenuPage(const char *menuItem, int value, const char *unit)
{
  displayHeader(menuItem);
  if (unit != nullptr)
  {
    display.print(unit);
  }
  display.setTextSize(2);
  display.setCursor(5, 25);
  display.print(value);
  display.setTextSize(2);
  display.display();
}

void LCDMenu::displayFractionalIntMenuPage(const char *menuItem, int value, const char *unit)
{
  displayHeader(menuItem);
  if (unit != nullptr)
  {
    display.print(unit);
  }
  display.setTextSize(2);
  display.setCursor(5, 25);
  char strBuf[16];
  uint8_t x = value;
  uint8_t y = value % INTERVAL_DIV;
  x = (x - y) / INTERVAL_DIV;
  snprintf(strBuf, sizeof(strBuf), "%02d.%02d", x, y * (100 / INTERVAL_DIV));
  display.print(strBuf);
  display.setTextSize(2);
  display.display();
}

void LCDMenu::displayStringMenuPage(const char *menuItem, const char *value)
{
  displayHeader(menuItem);

  display.print("Value");
  display.setTextSize(2);
  display.setCursor(5, 25);
  display.print(value);
  display.setTextSize(2);
  display.display();
}

void LCDMenu::displayMenuItem(const char *item, int position, boolean selected)
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
  if(dir == 0)
  {
    return;
  }
  else if (page == 1)
  {
    pos = constrain(pos + dir, 0, menuItemCount - 1);

    if (pos > window + 3)
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
    case menuItems_e::CONTRAST:
      contrast += dir;
      setContrast(contrast);
      break;
    case menuItems_e::DISTANCE:
      distance += dir;
      break;
    case menuItems_e::EXPOSURE:
      exposureTime = constrain(exposureTime + dir, 0, 255);
      break;
    case menuItems_e::INTERVAL:
      interval += dir;
      break;
    }
  }
}

void LCDMenu::select(bool longpress)
{
  if (page == 1)
  { // main menu
    switch (pos)
    {
    case menuItems_e::START:
      if (!longpress)
      {
        action = menuAction::START;
      }
      else
      {
        action = menuAction::DRYRUN;
      }
      break;
    case menuItems_e::DIRECTION:
      direction = -direction;
      menuItems[4] = direction == 1 ? "Direction -->" : "Direction <--";
      break;
    case menuItems_e::RESET:
      resetDefaults();
      break;
    case menuItems_e::JOGMODE:
      action = menuAction::JOGMODE;
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

void LCDMenu::drawText(const char *title, const char *text)
{
  display.setTextSize(1);
  display.clearDisplay();
  display.setTextColor(BLACK, WHITE);
  display.setCursor(5, 10);
  display.print(title);
  display.drawFastHLine(0, 20, 83, BLACK);
  display.setCursor(5, 25);
  if (text != nullptr)
  {
    display.print(text);
  }
  display.display();
}