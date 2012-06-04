#ifndef SETTINGS_cpp
#define SETTINGS_cpp

#include <EEPROM.h>
#include "Arduino.h"
#include "SettingsManager.h"
#include "TempoBoss.h"

#define PRESET_COUNT 10
#define OUTPUT_COUNT 4
#define SETTINGS_ROOT_BYTE 50
#define OUTPUT_BYTE_SPACING 20
#define OUTPUT_SETTINGS_COUNT_SPACING 20

void SettingsManager::resetAllSettings()
{
  for (byte preset = 0; preset < PRESET_COUNT; preset++) {
    for (byte output = 0; output < OUTPUT_COUNT; output++) {
      setRate(1, output, preset);
      setPulseLength(-1, output, preset);
      setPulseCount(6, output, preset);
      setPolarity(0, output, preset);
    }
  }
}

void SettingsManager::openPreset(int preset)
{
  currentPreset = preset;
}

void SettingsManager::switchToPreset(int preset)
{
  
}

void SettingsManager::setRate(float rate, int outputID)
{ setRate(rate, outputID, currentPreset); }

void SettingsManager::setRate(float rate, int outputID, int preset)
{
  
}

void SettingsManager::setPulseLength(int pulseLength, int outputID)
{setPulseLength(pulseLength, outputID, currentPreset);}

void SettingsManager::setPulseLength(int pulseLength, int outputID, int preset)
{
  
}

void SettingsManager::setPulseCount(int pulseCount, int outputID)
{setPulseCount(pulseCount, outputID, currentPreset);}

void SettingsManager::setPulseCount(int pulseCount, int outputID, int preset)
{
  
}

void SettingsManager::setPolarity(boolean polarity, int outputID)
{setPolarity(polarity, outputID, currentPreset);}

void SettingsManager::setPolarity(boolean polarity, int outputID, int preset)
{
  
}

int eepromReadInt(int address){
   int value = 0x0000;
   value = value | (EEPROM.read(address) << 8);
   value = value | EEPROM.read(address+1);
   return value;
}
 
void eepromWriteInt(int address, int value){
   EEPROM.write(address, (value >> 8) & 0xFF );
   EEPROM.write(address+1, value & 0xFF);
}
 
float eepromReadFloat(int address){
   union u_tag {
     byte b[4];
     float fval;
   } u;   
   u.b[0] = EEPROM.read(address);
   u.b[1] = EEPROM.read(address+1);
   u.b[2] = EEPROM.read(address+2);
   u.b[3] = EEPROM.read(address+3);
   return u.fval;
}
 
void eepromWriteFloat(int address, float value){
   union u_tag {
     byte b[4];
     float fval;
   } u;
   u.fval=value;
 
   EEPROM.write(address  , u.b[0]);
   EEPROM.write(address+1, u.b[1]);
   EEPROM.write(address+2, u.b[2]);
   EEPROM.write(address+3, u.b[3]);
}

#endif
