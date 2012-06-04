#ifndef MENUMANAGER_h
#define MENUMANAGER_h

#include "Arduino.h"
#include <serLCD.h>
#include <Encoder.h>
#include "TempoBoss.h"

class MenuManagerDelegate {
  public:
    virtual void menuBecameActive() {};
    virtual void menuBecameInactive() {};
};

struct MenuItem {
  char title[16];
  byte ID;
};

static const int MAX_MENU_LEVELS = 7;

class MenuManager
{
  public:
    // properties
    byte menuHistory[MAX_MENU_LEVELS];
    boolean menuActive;
    Encoder *menuEncoder;
    serLCD *menuLCD;
    MenuManagerDelegate *delegate;
    TempoBoss *tempoController;
    long menuTimeoutTimer;
    
    // methods
    void initialize();
    void processLoop();
    void exitMenu();
    
  private:
    
    int currentMenuSelection;
    int currentMenuCount;
    boolean buttonState;
    
    boolean parameterEditorActive;
    boolean parameterEditorUpdateOnChange;
    void (*parameterSelectedCallback)(MenuManager*,int);
    char **parameterOptions;
    String parameterUnit; 
    String parameterRoot;
    int parameterHigh;
    int parameterLow;
    int parameterIncrement;
    
    byte currentMenuLevel;
    int oldEncoderPosition;
    String menuTitleString;
    
    // methods
    MenuItem *currentMenu;
    void selectPressed();
    void resetMenuTimeout();
    void displayMenu(String title, MenuItem menuItems[], int itemCount);
    void displayMenuItem(MenuItem item);
    void displayParameterOption(String parameter);
    void displayOptionPropertyEditor(String title, 
                                     char* options[],
                                     int optionsCount,
                                     int initialChoice,
                                     void (*selectCallback)(MenuManager*, int),
                                     boolean updateOnSettingChange);
    void displayNumericPropertyEditor(String title, 
                                     String unit,
                                     int incrementValue,
                                     int rangeHigh,
                                     int rangeLow,
                                     int initialValue,
                                     void (*selectCallback)(MenuManager*, int),
                                     String optionalRootValue,
                                     boolean updateOnSettingChange);
    void pushMenuWithID(byte menuID);
    void popParameterEditor();
    void popCurrentMenu();
};

extern int outputAddresses[];

enum settingBytes {
  rate = 0,
  polarity,
  pulseCount,
  pulseLengthHighByte,
  pulseLengthLowByte,
};

#endif
