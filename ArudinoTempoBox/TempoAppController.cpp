
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
SettingsManager settingsManager;
int oldEncoderPosition;
void tempoChanged(float tempo);

void TempoAppController::initialize()
{
  debugBegin();
  delay(1000); //Wake Up
  encoderOffset = 0;
  oldEncoderPosition = 0;
  menuController.initialize();
  menuController.menuLCD = &lcd;
  menuController.menuEncoder = &myEnc;
  menuController.delegate = this;
  menuController.tempoController = &tempoController;
  menuController.settingsManager = &settingsManager;
  
  if (digitalRead(4) == HIGH) {
    delay(5000);
    if (digitalRead(4) == HIGH) {
      lcd.clear();
      lcd.print("Resetting"); 
      settingsManager.resetAllSettings();
      delay(1000);
    }
  }
  tempoController.initialize();
  tempoController.setTempoChangeCallback(tempoChanged);
  settingsManager.openPreset(0);
  lcd.clear();
  lcd.print("Tempo Box"); 
  delay(500); // See the Awesome startup text
  lcd.clear();
  lcd.print("   BPM: ");
  lcd.print(120);
}

void TempoAppController::processLoop()
{
  tempoController.processLoop();
  menuController.processLoop();
  if (menuController.menuActive) {
    //Do nothing for now
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
  int oldEncoderPosition = myEnc.read();
  tempoChanged(tempoController.getTempo());
}

void tempoChanged(float tempo)
{
  if (menuController.menuActive) {
    return;
  }
  debugPrintln("Updating BPM on LCD");
  debugPrintln(tempo);
  lcd.clear();
  lcd.print("   BPM: ");
  int tempoInt = tempo;
  lcd.print(tempoInt);
}

void TempoAppController::readEncoder()
{
    int newPosition = myEnc.read();
    if (abs(oldEncoderPosition - newPosition) > 3) { // check that the encoder has moved a notch at least
      int encoderDelta = (newPosition - oldEncoderPosition)/3;
      int tempo = tempoController.getTempo() + encoderDelta;
      tempoController.setTempo(tempo);
      tempoController.restartOutputPulses();
      tempoChanged(tempo);
      oldEncoderPosition = newPosition;
    }
//  long newPosition = myEnc.read() - encoderOffset;
//  if (newPosition != oldPosition) {
//    oldPosition = newPosition;
//    tempoController.setTempo((newPosition/4) + 120);
//    tempoChanged((newPosition/4) + 120);
//  }
}

#endif
