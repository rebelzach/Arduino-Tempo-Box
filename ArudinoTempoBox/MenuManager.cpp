
#ifndef MENUMANAGER_cpp
#define MENUMANAGER_cpp
#include <EEPROM.h>
#include "Definitions.h"
#include "Arduino.h"
#include "MenuManager.h"

int outputAddresses[4] = {20,60,120,200};

int const MENU_TIMEOUT_DURATION = 10000;
int enterButtonPin = 4; // This Pin Should ride 

void selectPressed();
void pulseRateSelected(int selection);

void MenuManager::initialize()
{
  oldEncoderPosition = 0;
  currentMenuLevel = 0;
  currentMenuSelection = 0;
  parameterEditorActive = LOW;
  menuActive = LOW;
  buttonState = LOW;
}

const byte menuIDRoot = 0;
const byte menuIDOutput1 = 1;
const byte menuIDOutput2 = 2;
const byte menuIDOutput3 = 3;
const byte menuIDOutput4 = 4;
MenuItem menuRoot[4] = {{"Output 1", menuIDOutput1},
                        {"Output 2", menuIDOutput2},
                        {"Output 3", menuIDOutput3},
                        {"Output 4", menuIDOutput4}};
const byte menuIDRate = 7;
const byte menuIDPolarity = 8;
const byte menuIDPulseCount = 9;
const byte menuIDPulseLength = 10;
MenuItem menuOutputOptions[4] = {{"Rate", menuIDRate},
                                {"Polarity", menuIDPolarity},
                                {"Pulse Count", menuIDPulseCount},
                                {"Pulse Length", menuIDPulseLength}};
char *rates[11] = {"1:1",
                  "1:2",
                  "1:3",
                  "1:4",
                  "1:6",
                  "1:8",
                  "2:1",
                  "3:1",
                  "4:1",
                  "6:1",
                  "8:1"};
                  
char *polaritySettings[2] = {"Normally Open",
                             "Normally Closed"};
                                
void MenuManager::pushMenuWithID(byte menuID)
{
  debugPrint("MENU ID:");
  debugPrintln(menuID);
  debugPrint("MENU Level:");
  debugPrintln(currentMenuLevel);
  switch (menuID)
  {
    case menuIDRoot:
       displayMenu("Menu", menuRoot, sizeof(menuRoot)/sizeof(MenuItem));
       break;
    case menuIDOutput1:
       displayMenu("Output 1 Options", menuOutputOptions, sizeof(menuOutputOptions)/sizeof(MenuItem));
       break;
    case menuIDOutput2:
       displayMenu("Output 2 Options", menuOutputOptions, sizeof(menuOutputOptions)/sizeof(MenuItem));
       break;
    case menuIDOutput3:
       displayMenu("Output 3 Options", menuOutputOptions, sizeof(menuOutputOptions)/sizeof(MenuItem));
       break;
    case menuIDOutput4:
       displayMenu("Output 4 Options", menuOutputOptions, sizeof(menuOutputOptions)/sizeof(MenuItem));
       break;
    case menuIDRate:
       displayOptionPropertyEditor("Pulse Rate", rates, 11, 0, &pulseRateSelected);
       break;
    case menuIDPolarity:
       //displayOptionPropertyEditor("Pulse Rate", rates, 11, 0, pulseRateSelected);
       break;
    case menuIDPulseCount:
    
       break;
    case menuIDPulseLength:

       break;
  }
  
}

const int MAX_DEBOUNCE_TIMEOUT = 200;
unsigned long menuDebounceTimer = 0;

void MenuManager::processLoop()
{
  if (millis() > menuDebounceTimer + MAX_DEBOUNCE_TIMEOUT) {
    if (buttonState == LOW) {
      if (digitalRead(enterButtonPin) == HIGH) { // Pin 2 low button down
        buttonState = HIGH;
        menuDebounceTimer = millis();
        selectPressed();
      }
    }
    if (buttonState == HIGH) {
      if (digitalRead(enterButtonPin) == LOW) { // Pin 2 high button up
        buttonState = LOW;
        menuDebounceTimer = millis();
      }
    }
  }
  if (menuActive) { 
    // Read the encoder
    int newPosition = menuEncoder->read();
    if (abs(oldEncoderPosition - newPosition) > 3) { // check that the encoder has moved a notch at least
      resetMenuTimeout();
      int menuItemCount = currentMenuCount + 1; // Items + "Exit"
      if (parameterEditorActive) { 
        menuItemCount = currentMenuCount; 
      }
      debugPrint("Menu Count:");
      debugPrintln(menuItemCount);
      int selectionDelta = (newPosition - oldEncoderPosition)/3;
      debugPrint("Selection Delta:");
      debugPrintln(selectionDelta);
      int rawSelection = currentMenuSelection + selectionDelta;
      currentMenuSelection = rawSelection % menuItemCount;
      debugPrint("Moded Selection:");
      debugPrintln(currentMenuSelection);
      if (currentMenuSelection < 0) { // wrap properly for negative values
        currentMenuSelection = menuItemCount + currentMenuSelection;
      }
      
      debugPrint("Selection:");
      debugPrintln(currentMenuSelection);
      oldEncoderPosition = newPosition;
      
      if (parameterEditorActive) { 
        displayParameterOption(parameterOptions[currentMenuSelection]);
      } else { // We are in a menu and we should select accordingly
        if (currentMenuSelection == menuItemCount - 1) {
          MenuItem exitItem = {"Exit", 222};
          displayMenuItem(exitItem);
        } else {
          displayMenuItem(currentMenu[currentMenuSelection]);
        }
      }
      
    }
    if (millis() > menuTimeoutTimer + MENU_TIMEOUT_DURATION)
      exitMenu();
  }
}

const int menuAdjustRates = 1;

void MenuManager::selectPressed()
{
  debugPrintln("PRESS");
  resetMenuTimeout();
  if (menuActive == LOW) {
    menuActive = HIGH;
    oldEncoderPosition = menuEncoder->read();
    currentMenuLevel = 0;
    menuHistory[currentMenuLevel] = menuIDRoot;
    pushMenuWithID(menuIDRoot);
    delegate->menuBecameActive();
    return;     
  }
  if (parameterEditorActive) {
    parameterSelectedCallback(currentMenuSelection);
    popParameterEditor();
    return;
  }
  if (currentMenuSelection == currentMenuCount) { // Exit
    popCurrentMenu();
    return;
  }
  currentMenuLevel++;
  menuHistory[currentMenuLevel] = currentMenu[currentMenuSelection].ID;
  pushMenuWithID(currentMenu[currentMenuSelection].ID);
}

void MenuManager::resetMenuTimeout()
{
  menuTimeoutTimer = millis();
}

void MenuManager::displayMenu(String title, MenuItem menuItems[], int itemCount)
{
  currentMenuCount = itemCount;
  currentMenu = menuItems;
  currentMenuSelection = 0;
  menuTitleString = title;
  displayMenuItem(menuItems[0]);
}

void MenuManager::displayMenuItem(MenuItem item)
{
  menuLCD->clear();
  menuLCD->print(menuTitleString);
  menuLCD->selectLine(2);
  menuLCD->print(item.title);
}

void MenuManager::displayParameterOption(String option)
{
  menuLCD->clear();
  menuLCD->print(menuTitleString);
  menuLCD->selectLine(2);
  menuLCD->print(option);
  if (parameterUnit.length()>0) {
    menuLCD->print(" ");
    menuLCD->print(parameterUnit);
  }
}

void MenuManager::popCurrentMenu()
{
  if (currentMenuLevel == 0) {
    exitMenu();
    return;
  }
  
  currentMenuLevel--;
  
  debugPrint("Popping. Current Level is:");
  debugPrintln(currentMenuLevel);
  debugPrint("menu loading is:");
  debugPrintln(menuHistory[currentMenuLevel]);
  pushMenuWithID(menuHistory[currentMenuLevel]);
}

void MenuManager::exitMenu()
{
   menuActive = LOW;
   parameterEditorActive = LOW;
   delegate->menuBecameInactive();
}

void MenuManager::popParameterEditor()
{
  debugPrintln("Popping parameter editor");
  parameterEditorActive = LOW;
  currentMenuLevel--;
  pushMenuWithID(menuHistory[currentMenuLevel]);
}

void MenuManager::displayOptionPropertyEditor(String title, 
                                     char* options[],
                                     int optionsCount,
                                     int initialChoice,
                                     void (*selectCallback)(int))
{
  menuTitleString = title;
  parameterEditorActive = HIGH;
  parameterOptions = options;
  parameterSelectedCallback = selectCallback;
  currentMenuCount = optionsCount;
  currentMenuSelection = initialChoice;
  displayParameterOption(parameterOptions[initialChoice]);
}

void MenuManager::displayNumericPropertyEditor(String title, 
                                     String unit,
                                     String rootOption,
                                     int incrementValue,
                                     int rangeHigh,
                                     int rangeLow,
                                     int initialValue,
                                     void (*selectCallback)(int),
                                     String optionalRootValue)
{
  parameterSelectedCallback = selectCallback;
}

void pulseRateSelected(int selection)
{
  debugPrintln("Pulse Rate selected");
}

#endif
