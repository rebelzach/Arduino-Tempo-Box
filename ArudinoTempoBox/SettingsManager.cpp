#ifndef SETTINGS_cpp
#define SETTINGS_cpp

#include <EEPROM.h>
#include "Arduino.h"
#include "SettingsManager.h"
#include "TempoBoss.h"
#include "Definitions.h"

#define PRESET_COUNT 10
#define OUTPUT_COUNT 4
#define SETTINGS_ROOT_BYTE 50
#define OUTPUT_BYTE_SPACING 20
#define OUTPUT_SETTINGS_COUNT_SPACING 20

#define RATE_ADDRESS 0
#define PULSE_LENGTH_ADDRESS 4
#define PULSE_COUNT_ADDRESS 6
#define POLARITY_ADDRESS 9

void eepromWriteInt(int address, int value);
float eepromReadFloat(int address);
void eepromWriteFloat(int address, float value);
int addressForOutput(int output, int preset);
int eepromReadInt(int address);

void SettingsManager::resetAllSettings()
{
  for (byte preset = 0; preset < PRESET_COUNT; preset++) {
    for (byte output = 0; output < OUTPUT_COUNT; output++) {
      setRate(0, output, preset);
      setPulseLength(-1, output, preset);
      setPulseCount(6, output, preset);
      setPolarity(0, output, preset);
    }
  }
}

void SettingsManager::refreshTempo()
{
   tempoController->rePollSettings();
}

void SettingsManager::setControllerRate(int rateChoice, int output)
{
  float pulseRate = 0;
  switch (rateChoice) {
    case 0:
       pulseRate = 1;
       break;
    case 1:
       pulseRate = 2;
       break;
    case 2:
       pulseRate = 3;
       break;
    case 3:
       pulseRate = 4;
       break;
    case 4:
       pulseRate = 6;
       break;
    case 5:
       pulseRate = 8;
       break;
    case 6:
       pulseRate = .5;
       break;
    case 7:
       pulseRate = .333;
       break;
    case 8:
       pulseRate = .25;
       break;
    case 9:
       pulseRate = .1667;
       break;
    case 10:
       pulseRate = .125;
       break;
  }
  debugPrint("Updating Pulse Rate:");
  debugPrintln(pulseRate);
  pedalPulseRateSetting[output] = pulseRate;
}

void SettingsManager::setControllerPulseLength(int pulseLength, int output)
{
  debugPrint("Updating Pulse Len:");
  if (pulseLength == -1) {
    pulseLength = BIG_LONG;
  }
  debugPrintln(pulseLength);
  microsecondPulseLenSetting[output] = pulseLength * 1000L;
}

void SettingsManager::setControllerPulseCount(int pulseCount, int output)
{
  debugPrint("Updating Pulse Count:");
  if (pulseCount == -1) {
    pulseCount = ALWAYS_PULSE;
  }
  debugPrintln(pulseCount);
  pedalPulseCountSetting[output] = pulseCount;
}

void SettingsManager::setControllerPolarity(boolean polarity, int output)
{
  debugPrint("Updating Polarity:");
  debugPrintln(polarity);
  pedalPulsePolaritySetting[output] = polarity;
}

void SettingsManager::openPreset(int preset)
{
  currentPreset = preset;
  for (byte output = 0; output < OUTPUT_COUNT; output++) {
    setControllerRate(getRate(output, preset), output);
    setControllerPulseLength(getPulseLength(output, preset), output);
    setControllerPulseCount(getPulseCount(output, preset), output);
    setControllerPolarity(getPolarity(output, preset), output);
  }
  refreshTempo();
}

// PUBLIC
void SettingsManager::setRate(int rate, int outputID)
{ 
  setRate(rate, outputID, currentPreset);
  setControllerRate(rate, outputID);
  refreshTempo();
}

void SettingsManager::setRate(int rate, int outputID, int preset)
{
  int address = addressForOutput(outputID, preset) + RATE_ADDRESS;
  if (eepromReadInt(address) != rate) {
    eepromWriteInt(address, rate);
  }
}

int SettingsManager::getRate(int outputID)
{
  return getRate(outputID, currentPreset);
}

int SettingsManager::getRate(int outputID, int preset)
{
  int address = addressForOutput(outputID, preset) + RATE_ADDRESS;
  return eepromReadInt(address);
}
// PUBLIC
void SettingsManager::setPulseLength(int pulseLength, int outputID)
{
  setPulseLength(pulseLength, outputID, currentPreset);
  setControllerPulseLength(pulseLength, outputID);
  refreshTempo();
}

void SettingsManager::setPulseLength(int pulseLength, int outputID, int preset)
{
  int address = addressForOutput(outputID, preset) + PULSE_LENGTH_ADDRESS;
  if (eepromReadInt(address) != pulseLength) {
    eepromWriteInt(address, pulseLength);
  }
}

int SettingsManager::getPulseLength(int outputID)
{
  return getPulseLength(outputID, currentPreset);
}

int SettingsManager::getPulseLength(int outputID, int preset)
{
  int address = addressForOutput(outputID, preset) + PULSE_LENGTH_ADDRESS;
  return eepromReadInt(address);
}
// PUBLIC
void SettingsManager::setPulseCount(int pulseCount, int outputID)
{
  setPulseCount(pulseCount, outputID, currentPreset);
  setControllerPulseCount(pulseCount, outputID);
  refreshTempo();
}

void SettingsManager::setPulseCount(int pulseCount, int outputID, int preset)
{
  int address = addressForOutput(outputID, preset) + PULSE_COUNT_ADDRESS;
  if (eepromReadInt(address) != pulseCount) {
    eepromWriteInt(address, pulseCount);
  }
}

int SettingsManager::getPulseCount(int outputID)
{
  return getPulseCount(outputID, currentPreset);
}

int SettingsManager::getPulseCount(int outputID, int preset)
{
  int address = addressForOutput(outputID, preset) + PULSE_COUNT_ADDRESS;
  return eepromReadInt(address);
}
// PUBLIC
void SettingsManager::setPolarity(boolean polarity, int outputID)
{
  setPolarity(polarity, outputID, currentPreset);
  setControllerPolarity(polarity, outputID);
  refreshTempo();
}

void SettingsManager::setPolarity(boolean polarity, int outputID, int preset)
{
  int address = addressForOutput(outputID, preset) + POLARITY_ADDRESS;
  if (EEPROM.read(address) != polarity) {
    EEPROM.write(address, polarity);
  }
}

boolean SettingsManager::getPolarity(int outputID)
{
  return getPolarity(outputID, currentPreset);
}

boolean SettingsManager::getPolarity(int outputID, int preset)
{
  int address = addressForOutput(outputID, preset) + POLARITY_ADDRESS;
  return EEPROM.read(address);
}

int addressForOutput(int output, int preset)
{
  int presetAddress = SETTINGS_ROOT_BYTE + (preset * OUTPUT_BYTE_SPACING * OUTPUT_COUNT);
  int address = presetAddress + (output * SETTINGS_ROOT_BYTE);
  return address;
}

int eepromReadInt(int address)
{
   int value = 0x0000;
   value = value | (EEPROM.read(address) << 8);
   value = value | EEPROM.read(address+1);
   return value;
}
 
void eepromWriteInt(int address, int value)
{
   EEPROM.write(address, (value >> 8) & 0xFF );
   EEPROM.write(address+1, value & 0xFF);
}
 
float eepromReadFloat(int address)
{
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
 
void eepromWriteFloat(int address, float value)
{
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
