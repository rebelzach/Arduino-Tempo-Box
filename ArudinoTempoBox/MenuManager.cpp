
#ifndef MENUMANAGER_cpp
#define MENUMANAGER_cpp

#include "Arduino.h"
#include "MenuManager.h"

int enterButtonPin = 4; // This Pin Should ride 

void selectPressed();

struct MenuItem {
  String title;
  byte ID;
};

void MenuManager::initialize()
{
  currentMenu = 0;
  currentMenuSelection = 0;
  parameterEditorActive = LOW;
  menuActive = LOW;
  buttonState = LOW;
}

void MenuManager::processLoop()
{
  if (buttonState == LOW) {
    if (digitalRead(enterButtonPin) == HIGH) { // Pin 2 low button down
      buttonState = HIGH;
      selectPressed();
    }
  }
  if (buttonState == HIGH) {
    if (digitalRead(enterButtonPin) == LOW) { // Pin 2 high button up
      buttonState = LOW;
    }
  }
}

const byte menuRootID = 0;
MenuItem menuRoot = {"MainMenu", menuRootID};

const int menuAdjustRates = 1;

void MenuManager::selectPressed()
{
  if (menuActive = LOW) {
    menuActive = HIGH;
    return;
  }
  if (parameterEditorActive) {
    popParameterEditor();
    return;
  }
  switch (currentMenu)
  {
    case menuRootID:
       
       break;
    case menuAdjustRates:
       
       break;
  }
}

void MenuManager::pushMenuItem(String title, String subtitle, int menuID)
{
  
}

void MenuManager::popParameterEditor()
{
  
}

#endif
