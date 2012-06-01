#ifndef APP_h
#define APP_h

#include "Arduino.h"
#include "TimerTwo.h"
#include "TempoBoss.h"
#include "MenuManager.h"
#include <serLCD.h>
#include <Encoder.h>

class TempoAppController: public MenuManagerDelegate
{
  public:
    // properties
    
    // methods
    void initialize();
    void processLoop();
    void menuBecameActive();
    void menuBecameInactive();
  private:
    int encoderOffset;
    void readEncoder();
};

#endif
