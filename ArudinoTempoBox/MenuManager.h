#ifndef MENUMANAGER_h
#define MENUMANAGER_h

#include "Arduino.h"
#include <serLCD.h>
#include <Encoder.h>

class MenuManager
{
  public:
    // properties
    int currentMenu;
    int currentMenuSelection;
    boolean buttonState;
    boolean menuActive;
    boolean parameterEditorActive;
    Encoder *menuEncoder;
    serLCD *menuLCD;
    
    // methods
    void initialize();
    void processLoop();
  private:
    void selectPressed();
    void popParameterEditor();
    void pushMenuItem(String title, String subtitle, int menuID);
};

class MenuManagerDelegate {
  public:
    virtual void menuBecameActive() {};
    virtual void menuBecameInactive() {};
};

#endif
