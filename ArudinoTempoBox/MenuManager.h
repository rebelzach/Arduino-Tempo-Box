#ifndef MENUMANAGER_h
#define MENUMANAGER_h

#include "Arduino.h"
#include <serLCD.h>
#include <Encoder.h>

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
    boolean menuActive;
    Encoder *menuEncoder;
    serLCD *menuLCD;
    MenuManagerDelegate *delegate;
    
    // methods
    void initialize();
    void processLoop();
    
  private:
    int menuHistory[MAX_MENU_LEVELS];
    int currentMenuSelection;
    int currentMenuCount;
    boolean buttonState;
    boolean parameterEditorActive;
    byte currentMenuLevel;
    int oldEncoderPosition;
    String menuTitleString;
    
    // methods
    MenuItem *currentMenu;
    void selectPressed();
    
    void pushMenu(String title, MenuItem menuItems[], int itemCount);
    void pushMenuItem(MenuItem item);
    void pushMenuWithID(int menuID);
    void popParameterEditor();
    void popCurrentMenu();
};

#endif
