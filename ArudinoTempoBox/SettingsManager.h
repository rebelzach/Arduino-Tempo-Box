#ifndef SETTINGS_h
#define SETTINGS_h

#include "Arduino.h"
#include "TempoBoss.h"

class SettingsManager
{
  public:
    void initialize();
    TempoBoss *tempoController;
    void resetAllSettings();
    void openPreset(int preset);
    int getPreset();
    int shouldListenForPresetChange;
    void setRate(int rate, int outputID);
    void setPulseLength(int pulseLength, int outputID);
    void setPulseCount(int pulseCount, int outputID);
    void setPolarity(int polarity, int outputID);
    void setTempo(float tempo);
    float getTempo();
    int getRate(int outputID);
    int getPulseLength(int outputID);
    int getPulseCount(int outputID);
    int getPolarity(int outputID);
    int getTapInput();
    void setTapInput(int tapInput, boolean persist);
    void setControllerRate(int rateChoice, int output);
    void setControllerPulseLength(int pulseLength, int output);
    void setControllerPulseCount(int pulseCount, int output);
    void setControllerPolarity(int polarity, int output);
    void setControllerTempo(float tempo);
    void refreshTempo();
    void processLoop();
  private:
    int currentPreset;
    void setRate(int rate, int outputID, int preset);
    void setPulseLength(int pulseLength, int outputID, int preset);
    void setPulseCount(int pulseCount, int outputID, int preset);
    void setPolarity(int polarity, int outputID, int preset);
    void setTempo(float tempo, int preset);
    float getTempo(int preset);
    int getRate(int outputID, int preset);
    int getPulseLength(int outputID, int preset);
    int getPulseCount(int outputID, int preset);
    int getPolarity(int outputID, int preset);
};

#endif

