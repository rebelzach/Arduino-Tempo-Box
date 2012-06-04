#include "Definitions.h"
#include "TempoAppController.h"
#include <SoftwareSerial.h>
#include <serLCD.h>
#include <Encoder.h>
#include <EEPROM.h>

unsigned long loopCounter = 0;
unsigned long loopStartTime = 0;

TempoAppController appController;

void setup() 
{ 
  appController.initialize();
#if DEBUG 
  loopStartTime = millis();
#endif
}

void loop() 
{
  appController.processLoop();
#if DEBUG 
  loopCounter++;
  if (millis() - loopStartTime > 10000) {
    debugPrint("Loop count per second: ");
    debugPrintln(loopCounter/10);
    loopStartTime = millis();
    loopCounter = 0;
  }
#endif
}
