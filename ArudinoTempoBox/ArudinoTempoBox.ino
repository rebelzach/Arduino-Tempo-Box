#include "Definitions.h"
#include "TempoAppController.h"
#include <SoftwareSerial.h>
#include <serLCD.h>
#include <Encoder.h>
#include <EEPROM.h>
#include "MemoryFree.h"
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
    Serial.print("Loop count per second: ");
    Serial.println(loopCounter/10);
    loopStartTime = millis();
    loopCounter = 0;
  }
#endif
}
