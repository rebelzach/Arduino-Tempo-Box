
#ifndef APP_cpp
#define APP_cpp

#include "Definitions.h"
#include "Arduino.h"
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

  menuController.initialize();
  menuController.menuLCD = &lcd;
  menuController.menuEncoder = &myEnc;
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

void tempoChanged(float tempo)
{
  lcd.clear();
  lcd.print("   BPM: ");
  int tempoInt = tempo;
  lcd.print(tempoInt);
}

long oldPosition = 0;

void TempoAppController::readEncoder()
{
  long newPosition = myEnc.read();
  if (newPosition != oldPosition) {
    oldPosition = newPosition;
    tempoController.setTempo((newPosition/4) + 120);
    tempoChanged((newPosition/4) + 120);
    //Serial.println(newPosition);
  }
}

#endif
