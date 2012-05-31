#include "Definitions.h"
#include "TimerTwo.h"
#include "TempoBoss.h"
#include "MenuManager.h"
#include <SoftwareSerial.h>
#include <serLCD.h>
#include <Encoder.h>

//#define ENCODER_DO_NOT_USE_INTERRUPTS

unsigned long loopCounter = 0;
unsigned long loopStartTime = 0;

int LCDpin = A5;
serLCD lcd(LCDpin);
Encoder myEnc(2, 3);

void setup() 
{ 
  debugBegin();
  delay(1000); //Wake Up
  lcd.clear();
  lcd.print("Beatmaster 5000"); 
  delay(500); // See the Awesome startup text

  loopStartTime = millis();
  MenuManager1.initialize();
  MenuManager1.menuLCD = &lcd;
  MenuManager1.menuEncoder = &myEnc;
  TempoBoss1.initialize();
  TempoBoss1.setTempoChangeCallback(tempoChanged);
  lcd.clear();
  lcd.print("   BPM: ");
  lcd.print(120);
}

void tempoChanged(float tempo)
{
  lcd.clear();
  lcd.print("   BPM: ");
  int tempoInt = tempo;
  lcd.print(tempoInt);
}

void loop() 
{ 
  loopCounter++;
  TempoBoss1.processLoop();
  MenuManager1.processLoop();
  readEncoder();

  if (millis() - loopStartTime > 10000) {
    debugPrint("Loop: ");
    debugPrintln(loopCounter/10);
    loopStartTime = millis();
    loopCounter = 0;
  }
}

long oldPosition = 0;

void readEncoder()
{
  long newPosition = myEnc.read();
  if (newPosition != oldPosition) {
    oldPosition = newPosition;
    TempoBoss1.setTempo((newPosition/4) + 120);
    tempoChanged((newPosition/4) + 120);
    //Serial.println(newPosition);
  }
}

