
#ifndef MENUMANAGER_cpp
#define MENUMANAGER_cpp

#include "Definitions.h"
#include "Arduino.h"
#include "MenuManager.h"

int enterButtonPin = 4; // This Pin Should ride 

void selectPressed();

void MenuManager::initialize()
{
  oldEncoderPosition = 0;
  currentMenuLevel = 0;
  currentMenuSelection = 0;
  parameterEditorActive = LOW;
  menuActive = LOW;
  buttonState = LOW;
}

const byte menuRootID = 0;
MenuItem menuRoot[4] = {{"Output 1", 0},{"Output 2", 1},{"Output 3", 2},{"Output 4", 3}};

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
  if (menuActive) { 
    // Read the encoder
    int newPosition = menuEncoder->read();
    if (abs(oldEncoderPosition - newPosition) > 3) { // check that the encoder has moved a notch at least
      if (parameterEditorActive) { 
        
      } else { // We are in a menu and we should select accordingly
        int menuItemCount = currentMenuCount + 1; // Items + "Exit"
        int selectionDelta = (newPosition - oldEncoderPosition)/3;
        currentMenuSelection = (currentMenuSelection + selectionDelta) % menuItemCount;
        if (currentMenuSelection < 0) { // wrap properly for negative values
          currentMenuSelection = menuItemCount + currentMenuSelection;
        }
        if (currentMenuSelection == menuItemCount-1) {
          MenuItem exitItem = {"Exit", 222};
          pushMenuItem(exitItem);
        } else {
          pushMenuItem(currentMenu[currentMenuSelection]);
        }
      }
      oldEncoderPosition = newPosition;
    }
  }
}

const int menuAdjustRates = 1;

void MenuManager::selectPressed()
{
  if (menuActive == LOW) {
    menuActive = HIGH;
    oldEncoderPosition = menuEncoder->read();
    currentMenuLevel = 0;
    pushMenuWithID(menuRootID);
    delegate->menuBecameActive();
  }
  if (parameterEditorActive) {
    popParameterEditor();
    return;
  }
}

void MenuManager::pushMenuWithID(int menuID)
{
  switch (menuID)
  {
    case menuRootID:
       pushMenu("Menu", menuRoot, sizeof(menuRoot)/sizeof(MenuItem));
       break;
    case menuAdjustRates:
       
       break;
  }
}

void MenuManager::pushMenu(String title, MenuItem menuItems[], int itemCount)
{
  currentMenuCount = itemCount;
  currentMenu = menuItems;
  menuTitleString = title;
  pushMenuItem(menuItems[0]);
}

void MenuManager::pushMenuItem(MenuItem item)
{
  menuLCD->clear();
  menuLCD->print(menuTitleString);
  menuLCD->selectLine(2);
  menuLCD->print(item.title);
}

void MenuManager::popParameterEditor()
{
  
}

#endif
