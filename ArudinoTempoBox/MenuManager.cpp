
#ifndef MENUMANAGER_cpp
#define MENUMANAGER_cpp
#include <EEPROM.h>
#include "Definitions.h"
#include "Arduino.h"
#include "MenuManager.h"
#include "TempoBoss.h"

int outputAddresses[4] = {20,60,120,200};

int const MENU_TIMEOUT_DURATION = 20000;
int enterButtonPin = 4; // This Pin Should ride 

void selectPressed();
void pulseRateSelected(MenuManager* menuMan, int selection, boolean persist);
void polaritySelected(MenuManager* menuMan, int selection, boolean persist);
void pulseCountSelected(MenuManager* menuMan, int selection, boolean persist);
void pulseLengthSelected(MenuManager *menuMan, int selection, boolean persist);
void inputSelected(MenuManager *menuMan, int selection, boolean persist);

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
const byte menuIDTapInput = 5;
MenuItem menuRoot[5] = {{"Output 1", menuIDOutput1},
                        {"Output 2", menuIDOutput2},
                        {"Output 3", menuIDOutput3},
                        {"Output 4", menuIDOutput4},
                        {"Tap Input Jack", menuIDTapInput}};
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
char *tapInputSettings[2] = {"Internal Switch",
                             "External Jack"};
                                
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
    case menuIDTapInput:
    {
       int currentInput = settingsManager->getTapInput();
       displayOptionPropertyEditor("Tap Input", tapInputSettings, 2, currentInput, &inputSelected, HIGH);
    }
       break;   
    case menuIDRate:
    {
       int currentRate = settingsManager->getRate(currentOutput());
       displayOptionPropertyEditor("Pulse Rate", rates, 11, currentRate, &pulseRateSelected, HIGH);
    }
       break;
    case menuIDPolarity:
    {
       int currentPolarity = settingsManager->getPolarity(currentOutput());
       displayOptionPropertyEditor("Polarity", polaritySettings, 2, currentPolarity, &polaritySelected, HIGH);
    }
       break;
    case menuIDPulseCount:
    {
      int currentPulseCount = settingsManager->getPulseCount(currentOutput());
      displayNumericPropertyEditor("Pulse Count", "Pulses", 1, 100, 2, currentPulseCount, &pulseCountSelected, "Always Pulse", HIGH);
    }
       break;
    case menuIDPulseLength:
       {
       int currentPulseLength = settingsManager->getPulseLength(currentOutput());
       displayNumericPropertyEditor("Pulse Length", "ms", 10, 1000, 10, currentPulseLength, &pulseLengthSelected, "Automatic", HIGH);
       }
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
      if (parameterEditorActive && parameterRoot.length() == 0) { 
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
        if (parameterUnit.length() == 0) {
          displayParameterOption(parameterOptions[currentMenuSelection]);
        } else {
          if (parameterRoot.length() > 0 && currentMenuSelection == menuItemCount - 1) {
            debugPrintln("Updating root param");
            displayParameterOption(parameterRoot);
          } else {
            debugPrintln("Updating variable parameter");
            String currentParameter = "";
            int value = ((currentMenuSelection * parameterIncrement) + parameterLow);
            currentParameter = currentParameter + value + " " + parameterUnit;
            displayParameterOption(currentParameter);
          }
        }
        if (parameterEditorUpdateOnChange) {
          parameterSelectedCallback(this, currentMenuSelection, NO);
        }
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
    parameterSelectedCallback(this, currentMenuSelection, YES);
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
                                     void (*selectCallback)(MenuManager*, int,boolean),
                                     boolean updateOnSettingChange)
{
  menuTitleString = title;
  parameterEditorActive = HIGH;
  parameterOptions = options;
  parameterSelectedCallback = selectCallback;
  currentMenuCount = optionsCount;
  currentMenuSelection = initialChoice;
  displayParameterOption(parameterOptions[initialChoice]);
  
  parameterUnit = "";
  parameterIncrement = 0;
  parameterHigh = 0;
  parameterLow = 0;
  parameterRoot = "";
  parameterEditorUpdateOnChange = updateOnSettingChange;
}

void MenuManager::displayNumericPropertyEditor(String title, 
                                     String unit,
                                     int incrementValue,
                                     int rangeHigh,
                                     int rangeLow,
                                     int initialValue,
                                     void (*selectCallback)(MenuManager*, int,boolean),
                                     String optionalRootValue,
                                     boolean updateOnSettingChange)
{
  menuTitleString = title;
  parameterEditorActive = HIGH;
  parameterSelectedCallback = selectCallback;
  currentMenuCount = (rangeHigh - rangeLow)/incrementValue;
  currentMenuSelection = (initialValue - rangeLow)/incrementValue;
  parameterUnit = unit;
  parameterIncrement = incrementValue;
  String initalParameter = "";
  if (initialValue == -1) {
    initalParameter = initalParameter + optionalRootValue;
  } else {
    initalParameter = initalParameter + initialValue + " " + unit;
  }
  
  displayParameterOption(initalParameter);
  parameterHigh = rangeHigh;
  parameterLow = rangeLow;
  parameterRoot = optionalRootValue;
  parameterEditorUpdateOnChange = updateOnSettingChange;
}

void pulseRateSelected(MenuManager *menuMan, int selection, boolean persist)
{
  debugPrintln("Pulse Rate selected");
  int currentOutput = menuMan->currentOutput();
  
  if (persist) {
    menuMan->settingsManager->setRate(selection, currentOutput);
  } else {
    debugPrintln("No Persist");
    menuMan->settingsManager->setControllerRate(selection, currentOutput);
    menuMan->settingsManager->refreshTempo();
  }
}

void pulseLengthSelected(MenuManager *menuMan, int selection, boolean persist)
{
  debugPrintln("Pulse Count selected");
  int currentOutput = menuMan->currentOutput();
  int length = -1;
  if (selection != menuMan->currentMenuCount) {
    length = ((selection * menuMan->parameterIncrement) + menuMan->parameterLow);
  }
  if (persist) {
    menuMan->settingsManager->setPulseLength(length, currentOutput);
  } else {
    debugPrintln("No Persist");
    menuMan->settingsManager->setControllerPulseLength(length, currentOutput);
    menuMan->settingsManager->refreshTempo();
  }
}

void pulseCountSelected(MenuManager *menuMan, int selection, boolean persist)
{
  debugPrintln("Pulse Count selected");
  int currentOutput = menuMan->currentOutput();
  int count = -1;
  if (selection != menuMan->currentMenuCount) {
    count = ((selection * menuMan->parameterIncrement) + menuMan->parameterLow);
  }
  if (persist) {
    menuMan->settingsManager->setPulseCount(count, currentOutput);
  } else {
    debugPrintln("No Persist");
    menuMan->settingsManager->setControllerPulseCount(count, currentOutput);
    menuMan->settingsManager->refreshTempo();
  }
}

void polaritySelected(MenuManager *menuMan, int selection, boolean persist)
{
  debugPrintln("Polarity selected");
  int currentOutput = menuMan->currentOutput();
  if (persist) {
    menuMan->settingsManager->setPolarity(selection, currentOutput);
  } else {
    debugPrintln("No Persist");
    menuMan->settingsManager->setControllerPolarity(selection, currentOutput);
    menuMan->settingsManager->refreshTempo();
  }
}

void inputSelected(MenuManager *menuMan, int selection, boolean persist)
{
  debugPrint("Polarity selected");
  debugPrintln(selection);
  menuMan->settingsManager->setTapInput(selection, persist);
}

int MenuManager::currentOutput()
{
  return menuHistory[1] - 1;
}

#endif
