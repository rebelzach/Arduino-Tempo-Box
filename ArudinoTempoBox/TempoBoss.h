
#ifndef TEMPOHANDLER_h
#define TEMPOHANDLER_h

#include "Arduino.h"

extern volatile unsigned long microsecondPulseLenSetting[];
extern float pedalPulseRateSetting[];
extern volatile int pedalPulseCountSetting[];
extern volatile int pedalPulsePolaritySetting[];

void calculateAndSetIntervals(unsigned long quarterBeatPulseLength);
void processPendingTempoTaps();
void tempoTapped();
void resetPulseCounters();
float calculateTempo();
inline boolean shouldDebounce(unsigned long pulseLength);
inline void incrementPulseCounter(int pulseIndex, int pin, int pedalPin);
void pulseInterrupt();

class TempoBoss
{
  
  public:
    // properties
    void (*tempoChangeCallback)(float);

    // methods
    void initialize();
    void processLoop();
    void setTempoChangeCallback(void (*changeCallback)(float));
    void setTempo(float tempo);
    float getTempo();
    void rePollSettings();
    
  private:
    float tempo;
    void processPendingTempoTaps();
    void tempoTapped();
    float calculateTempo();
    void calculateAndSetIntervals(unsigned long quarterBeatPulseLength);
    void resetPulseCounters();
    
};

#endif
