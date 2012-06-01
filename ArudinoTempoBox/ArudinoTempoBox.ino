#include "Definitions.h"
#include "TempoAppController.h"
#include <SoftwareSerial.h>
#include <serLCD.h>
#include <Encoder.h>

unsigned long loopCounter = 0;
unsigned long loopStartTime = 0;

TempoAppController appController;

void setup() 
{ 
  appController.initialize();
  loopStartTime = millis();
}

void loop() 
{
  appController.processLoop();
  loopCounter++;
  if (millis() - loopStartTime > 10000) {
    debugPrint("Loop: ");
    debugPrintln(loopCounter/10);
    loopStartTime = millis();
    loopCounter = 0;
  }
}
