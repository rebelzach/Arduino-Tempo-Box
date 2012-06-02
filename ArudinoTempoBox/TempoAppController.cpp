
#ifndef APP_cpp
#define APP_cpp

#include "Definitions.h"
#include "Arduino.h"
#include "MenuManager.h"
#include "TempoAppController.h"

int LCDpin = A5;
serLCD lcd(LCDpin);
Encoder myEnc(2, 3);
MenuManager menuController;
TempoBoss tempoController;

void tempoChanged(float tempo);

void TempoAppController::initialize()
{
  debugBegin();
  delay(1000); //Wake Up
  lcd.clear();
  lcd.print("Beatmaster 5000"); 
  delay(500); // See the Awesome startup text
  encoderOffset = 0;
  menuController.initialize();
  menuController.menuLCD = &lcd;
  menuController.menuEncoder = &myEnc;
  menuController.delegate = this;
  tempoController.initialize();
  tempoController.setTempoChangeCallback(tempoChanged);
  lcd.clear();
  lcd.print("   BPM: ");
  lcd.print(120);
}

void TempoAppController::processLoop()
{
  tempoController.processLoop();
  menuController.processLoop();
  if (menuController.menuActive) {
    
  } else {
    readEncoder();
  }
}

void TempoAppController::menuBecameActive()
{
  // Do Nothing now, but you can't delete this function
}

long oldPosition = 0;

void TempoAppController::menuBecameInactive()
{
  encoderOffset = myEnc.read() - oldPosition;
  tempoChanged(tempoController.getTempo());
}


void tempoChanged(float tempo)
{
  debugPrintln("Updating BPM on LCD");
  lcd.clear();
  lcd.print("   BPM: ");
  int tempoInt = tempo;
  lcd.print(tempoInt);
}

void TempoAppController::readEncoder()
{
  long newPosition = myEnc.read() - encoderOffset;
  if (newPosition != oldPosition) {
    oldPosition = newPosition;
    tempoController.setTempo((newPosition/4) + 120);
    tempoChanged((newPosition/4) + 120);
  }
}

#endif
