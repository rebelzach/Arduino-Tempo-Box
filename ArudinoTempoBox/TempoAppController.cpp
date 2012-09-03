
#ifndef APP_cpp
#define APP_cpp

#include "Definitions.h"
#include "Arduino.h"
#include "MenuManager.h"
#include "TempoAppController.h"
#include "MemoryFree.h"

const int LCDpin = A5;
serLCD lcd(LCDpin);
Encoder myEnc(2, 3);
MenuManager menuController;
TempoBoss tempoController;
SettingsManager settingsManager;
int oldEncoderPosition;
void tempoChanged(float tempo);
unsigned long tempoUpdatedTimer = 0;
const int UPDATE_WAIT_DURATION = 2000; // This is the amount of time (millis) that must pass before the tempo is updated in the EEPROM, cuts down on EEPROM writes
byte tempoUpdatedFlag = 0;

void TempoAppController::initialize()
{
  debugBegin();
  delay(1000); //Wake Up
  debugPrintMem();
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
  settingsManager.tempoController = &tempoController;
  settingsManager.initialize();
  lcd.clear();
  lcd.print("Tempo Box"); 
  delay(500); // See the Awesome start up text
  settingsManager.openPreset(0);
  tempoChanged(tempoController.getTempo());
  tempoUpdatedFlag = 0;
}

void TempoAppController::processLoop()
{
  tempoController.processLoop();
  menuController.processLoop();
  settingsManager.processLoop();
  if (menuController.menuActive) {
    //Do nothing for now
  } else {
    readEncoder();
  }
  if (tempoUpdatedFlag) {
    if (millis() - tempoUpdatedTimer > UPDATE_WAIT_DURATION) {
      tempoUpdatedFlag = 0;
      settingsManager.setTempo(tempoController.getTempo());
    }
  }
}

void TempoAppController::menuBecameActive()
{
  settingsManager.shouldListenForPresetChange = 0; 
}

void TempoAppController::menuBecameInactive()
{
  oldEncoderPosition = myEnc.read();
  tempoChanged(tempoController.getTempo());
   settingsManager.shouldListenForPresetChange = 1; 
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
  lcd.selectLine(2);
  lcd.print("   Preset: ");
  lcd.print(settingsManager.getPreset() + 1); // +1 because of zero indexing
  tempoUpdatedFlag = 1;
  tempoUpdatedTimer = millis();
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
}

#endif
