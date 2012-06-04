#ifndef SETTINGS_h
#define SETTINGS_h

#include "Arduino.h"
#include "TempoBoss.h"

class SettingsManager
{
  public:
    TempoBoss *tempoController;
    void resetAllSettings();
    void openPreset(int preset);
    void switchToPreset(int preset);
    void setRate(float rate, int outputID);
    void setPulseLength(int pulseLength, int outputID);
    void setPulseCount(int pulseCount, int outputID);
    void setPolarity(boolean polarity, int outputID);
    
  private:
    int currentPreset;
    void setRate(float rate, int outputID, int preset);
    void setPulseLength(int pulseLength, int outputID, int preset);
    void setPulseCount(int pulseCount, int outputID, int preset);
    void setPolarity(boolean polarity, int outputID, int preset);
    
};



#endif
