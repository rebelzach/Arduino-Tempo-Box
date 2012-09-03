#ifndef TEMPOBOSS_cpp
#define TEMPOBOSS_cpp

#include "TimerTwo.h"
#include "Definitions.h"
#include "TempoBoss.h"

#include "Arduino.h"
#include <SoftwareSerial.h>

const int MIDI_PIN = A4;
SoftwareSerial midi =  SoftwareSerial(MIDI_PIN, MIDI_PIN);
void noteOn(int cmd, int pitch, int velocity);

int TAP_TEMPO_PIN = 7; // This is controlled by the settings manager
const int output1pin = 12;
const int output2pin = 10;
const int output3pin = 9;
const int output4pin = 8;
const int pedalPin1 = A0;
const int pedalPin2 = A1;
const int pedalPin3 = A2;
const int pedalPin4 = A3;
//int pedalPin1 = 12;
//int pedalPin2 = 10;
//int pedalPin3 = 9;
//int pedalPin4 = 8;
//int output1pin = A0;
//int output2pin = A1;
//int output3pin = A2;
//int output4pin = A3;

const int encRed = 5;
const int encGreen = 6;
const int encBlue = 11;

#define TAP_TIMEOUT 2000000
#define TAP_MINIMUM 250000
#define TIMER_QUANTUM 400 //100 //subdivision of update timer in microseconds

volatile int shouldMIDIPulse;
volatile unsigned long averagePulseLength;
volatile unsigned long microsecondInterval[4] = {0,0,0,0};
volatile unsigned long microsecondCounter[4] = {0,0,0,0};
volatile unsigned long microsecondPulseLenSetting[4] = {4294967292,100000,4294967292,4294967292};
volatile unsigned long beatCounter;
float beatLevel = 255;
volatile int pedalPulseCountSetting[4] = {8,4000,4000,16};
volatile int pedalPulsePolaritySetting[4] = {1,1,1,1};
float pedalPulseRateSetting[4] = {.25,1,1,2};
int pedalPulseCounter[4] = {0,0,0,0};
int pinStateBuffer[4] =         {1,1,1,1};
int previousPinStateBuffer[4] =      {0,0,0,0};
int pulseStateBuffer[4] = {0,0,0,0};

void TempoBoss::initialize()
{
  pinMode(output1pin, OUTPUT);
  pinMode(output2pin, OUTPUT);
  pinMode(output3pin, OUTPUT);
  pinMode(output4pin, OUTPUT);
  pinMode(4, INPUT);
  pinMode(6, INPUT);
  pinMode(pedalPin1, OUTPUT);
  pinMode(pedalPin2, OUTPUT);
  pinMode(pedalPin3, OUTPUT);
  pinMode(pedalPin4, OUTPUT);
  pinMode(encRed, OUTPUT);
  pinMode(encGreen, OUTPUT);
  pinMode(encBlue, OUTPUT);
  digitalWrite(encRed, HIGH);
  digitalWrite(encGreen, HIGH);
  digitalWrite(encBlue, HIGH);
  digitalWrite(pedalPin1, pedalPulsePolaritySetting[0]);
  digitalWrite(pedalPin2, pedalPulsePolaritySetting[1]);
  digitalWrite(pedalPin3, pedalPulsePolaritySetting[2]);
  digitalWrite(pedalPin4, pedalPulsePolaritySetting[3]);
  Timer1.initialize(TIMER_QUANTUM);
  Timer1.attachInterrupt(pulseInterrupt); // attach the service routine here
  tempoChangeCallback = NULL;
  setTempo(120);
  shouldMIDIPulse = NO;
  midi.setTX(A4);
  midi.begin(31250);
}

float previousBeatLevel = 0;
float tempoColorVector[3] = {0,0,0};
float rangedTempo = .5;

void TempoBoss::processLoop()
{
  processPendingTempoTaps();
  if (shouldMIDIPulse == 1) {
    noteOn(176, 64, 127);
    shouldMIDIPulse = 0;
  }
  if (beatLevel != previousBeatLevel) {
    previousBeatLevel = beatLevel;
    analogWrite(encRed, 255 - (beatLevel * rangedTempo));
    analogWrite(encGreen, 255 - (beatLevel * (1 - rangedTempo) * .5));
    analogWrite(encBlue, 255 - (beatLevel * (1 - rangedTempo)));
  }
}

volatile unsigned long tap0Time = 0;
volatile unsigned long tap1Time = 0;
volatile unsigned long pwmUpdateStep = 0;
volatile int tapIndex = 1;
const int maxPulseCount = 3;
volatile long pulseLengthBuffer[maxPulseCount] = {0, 0, 0};
int pulseIndex = 0;
int pulseBufferCount = 0;
volatile long tapState = OFF;
volatile boolean tapsProcessed = YES;
boolean triggerState = LOW;
unsigned long outputStartTime;

void TempoBoss::processPendingTempoTaps()
{
  if (triggerState == LOW) {
    if (digitalRead(TAP_TEMPO_PIN) == HIGH) { // Pin 2 low button down
      //digitalWrite(monitorPin, HIGH);
      triggerState = HIGH;
      tempoTapped();
    }
  }
  if (triggerState == HIGH) {
    if (digitalRead(TAP_TEMPO_PIN) == LOW) { // Pin 2 high button up
      //digitalWrite(monitorPin, LOW);
      triggerState = LOW;
    }
  }
  if (!tapsProcessed) {
    long pulseLength = tap1Time - tap0Time;
    if (pulseLength < TAP_TIMEOUT && pulseLength > TAP_MINIMUM) { // we havent timed out and we haven't overflowed
      outputStartTime = tap0Time;
      resetPulseCounters();
      pulseLengthBuffer[pulseIndex] = pulseLength;
      tapsProcessed = YES;
    } else { // timed out or overflow (hopefully overflows are rare)
      resetPulseCounters();
      tapIndex = 1; // we will only detect a timeout when ther is a new tap
      tap0Time = tap1Time; // Move the latest tap into the zero slot
      tapsProcessed = YES;
      pulseIndex = 0;
      pulseBufferCount = 0;
      return; // since we timed out
    }
    tempo = calculateTempo();
    
    if (pulseBufferCount < maxPulseCount - 1) { // Once we max pulses out then just stay at the max
      pulseBufferCount++;
    }
    if (pulseIndex < maxPulseCount - 1) {
      pulseIndex++;
    } else {
      pulseIndex = 0;
    }
  }
}

void TempoBoss::restartOutputPulses()
{
  debugPrintln("Reset");
  debugPrintln(microsecondPulseLenSetting[0]);
  debugPrintln(microsecondInterval[1]);
  debugPrintln(microsecondCounter[1]);
  microsecondCounter[0] = beatCounter;
  microsecondCounter[1] = beatCounter;
  microsecondCounter[2] = beatCounter;
  microsecondCounter[3] = beatCounter;
  pinStateBuffer[0] = 1;
  pinStateBuffer[1] = 1;
  pinStateBuffer[2] = 1;
  pinStateBuffer[3] = 1;
  digitalWrite(output1pin, !pedalPulsePolaritySetting[0]);
  digitalWrite(output2pin, !pedalPulsePolaritySetting[1]);
  digitalWrite(output3pin, !pedalPulsePolaritySetting[2]);
  digitalWrite(output4pin, !pedalPulsePolaritySetting[3]);
  pedalPulseCounter[0] = 0;
  pedalPulseCounter[1] = 0;
  pedalPulseCounter[2] = 0;
  pedalPulseCounter[3] = 0;
  pulseStateBuffer[0] = 1;
  pulseStateBuffer[1] = 1;
  pulseStateBuffer[2] = 1;
  pulseStateBuffer[3] = 1;
  shouldMIDIPulse = 1;
}

void TempoBoss::tempoTapped()
{
  unsigned long currentTime = micros();
  if (!tapsProcessed) {
    return; // Don't even think about recording data if we haven't dealt with the current taps
  }
  if (tapIndex == 0) {
    tap0Time = currentTime;
    tapIndex = 1;
  } else if (tapIndex == 1) {
    if (shouldDebounce(currentTime - tap0Time)) {
      return;
    }
    tap1Time = currentTime;
    tapIndex = 2;
    tapsProcessed = NO; // Signal that until these are processed we shouldn't register more
  } else if (tapIndex == 2) { // we remain at index 2 until the there is a reset
    if (shouldDebounce(currentTime - tap1Time)) {
      return;
    }
    tap0Time = tap1Time;
    tap1Time = currentTime;
    tapsProcessed = NO; // Signal that until these are processed we shouldn't register more
  }
}

inline boolean shouldDebounce(unsigned long pulseLength)
{
  return (pulseLength < TAP_MINIMUM);
}

float TempoBoss::calculateTempo() 
{
  long totalPulseLength = 0;
  for (int i = 0; i <= pulseBufferCount ; i++) {
    totalPulseLength += pulseLengthBuffer[i];
  }
  debugPrint("Buffercount: ");
  debugPrintln(pulseBufferCount);
  averagePulseLength = totalPulseLength / (pulseBufferCount + 1.0f);
  if (0) { //Quantization
    averagePulseLength = 1000000 * (60.0/(60000000L / averagePulseLength));
  }
  calculateAndSetIntervals(averagePulseLength);
  float bpm = 60000000.0f / averagePulseLength; // Microseconds to BPM
  if (tempoChangeCallback) {
    tempoChangeCallback(bpm);
  }
  return bpm;
}

void TempoBoss::setTempoChangeCallback(void (*changeCallback)(float))
{
  tempoChangeCallback = changeCallback;
}

void TempoBoss::setTempo(float newTempo)
{
  tempo = newTempo;
  averagePulseLength = 1000000 * (60.0/tempo);
  calculateAndSetIntervals(averagePulseLength);
}

float TempoBoss::getTempo()
{
  return tempo;
}

void TempoBoss::calculateAndSetIntervals(unsigned long quarterBeatPulseLength) 
{  
  unsigned long compensatedPulseLength = quarterBeatPulseLength;
  bitClear(quarterBeatPulseLength, 0); // Make |quarterBeatPulseLength| even
  pwmUpdateStep = quarterBeatPulseLength/10;
  
  microsecondInterval[0] = (compensatedPulseLength / pedalPulseRateSetting[0] / 2);
  microsecondInterval[1] = (compensatedPulseLength / pedalPulseRateSetting[1] / 2);
  microsecondInterval[2] = (compensatedPulseLength / pedalPulseRateSetting[2] / 2);
  microsecondInterval[3] = (compensatedPulseLength / pedalPulseRateSetting[3] / 2);
}

void TempoBoss::rePollSettings()
{
  calculateAndSetIntervals(averagePulseLength);
  restartOutputPulses();
}

void TempoBoss::resetPulseCounters() 
{
  debugPrintln("Reset");
  debugPrintln(microsecondPulseLenSetting[0]);
  debugPrintln(microsecondInterval[1]);
  debugPrintln(microsecondCounter[1]);
  microsecondCounter[0] = 0;
  microsecondCounter[1] = 0;
  microsecondCounter[2] = 0;
  microsecondCounter[3] = 0;
  pinStateBuffer[0] = 1;
  pinStateBuffer[1] = 1;
  pinStateBuffer[2] = 1;
  pinStateBuffer[3] = 1;
  digitalWrite(output1pin, !pedalPulsePolaritySetting[0]);
  digitalWrite(output2pin, !pedalPulsePolaritySetting[1]);
  digitalWrite(output3pin, !pedalPulsePolaritySetting[2]);
  digitalWrite(output4pin, !pedalPulsePolaritySetting[3]);
  pedalPulseCounter[0] = 0;
  pedalPulseCounter[1] = 0;
  pedalPulseCounter[2] = 0;
  pedalPulseCounter[3] = 0;
  pulseStateBuffer[0] = 1;
  pulseStateBuffer[1] = 1;
  pulseStateBuffer[2] = 1;
  pulseStateBuffer[3] = 1;
  beatCounter = 0;
  shouldMIDIPulse = 1;
}

volatile unsigned long pwmUpdateCounter = 0;
inline void incrementMidiCounter(int pulseIndex, int pin, int pedalPin);

void pulseInterrupt() 
{
  microsecondCounter[0] += TIMER_QUANTUM;
  microsecondCounter[1] += TIMER_QUANTUM;
  microsecondCounter[2] += TIMER_QUANTUM;
  microsecondCounter[3] += TIMER_QUANTUM;
  beatCounter += TIMER_QUANTUM;
  pwmUpdateCounter += TIMER_QUANTUM;
  if (beatCounter > averagePulseLength) {
    beatCounter = beatCounter - averagePulseLength;
    pwmUpdateCounter = beatCounter;
    beatLevel = 255;
  }
  if (pwmUpdateCounter > pwmUpdateStep) {
    pwmUpdateCounter = 0;
    if (beatLevel > 0) {
      beatLevel -= 20;
    }
  }
  incrementPulseCounter(0, output1pin, pedalPin1);
  incrementPulseCounter(1, output2pin, pedalPin2);
  incrementPulseCounter(2, output3pin, pedalPin3);
  incrementMidiCounter(3, output4pin, pedalPin4);
}
inline void updatePins(int pin, int pedalPin, int outputIndex, int state);
inline void updateMidi(int outputIndex);
inline void incrementMidiCounter(int outputIndex, int pin, int pedalPin)
{
  // This must be updated to match changes in incrementPulseCounter
  if (pulseStateBuffer[outputIndex] == 1 && microsecondCounter[outputIndex] > microsecondPulseLenSetting[outputIndex]) {
    pulseStateBuffer[outputIndex] = 0;
    updatePins(pin, pedalPin, outputIndex, pulseStateBuffer[outputIndex]);
  }
  if (microsecondCounter[outputIndex] > microsecondInterval[outputIndex]) {
    pedalPulseCounter[outputIndex]++;
    microsecondCounter[outputIndex] = microsecondCounter[outputIndex] - microsecondInterval[outputIndex];
    pinStateBuffer[outputIndex] = !pinStateBuffer[outputIndex];
    pulseStateBuffer[outputIndex] = pinStateBuffer[outputIndex];
    updatePins(pin, pedalPin, outputIndex, pinStateBuffer[outputIndex]);
    updateMidi(outputIndex);
  }
}

inline void incrementPulseCounter(int outputIndex, int pin, int pedalPin)
{
  if (pulseStateBuffer[outputIndex] == 1 && microsecondCounter[outputIndex] > microsecondPulseLenSetting[outputIndex]) {
    pulseStateBuffer[outputIndex] = 0;
    updatePins(pin, pedalPin, outputIndex, pulseStateBuffer[outputIndex]);
  }
  if (microsecondCounter[outputIndex] > microsecondInterval[outputIndex]) {
    pedalPulseCounter[outputIndex]++;
    microsecondCounter[outputIndex] = microsecondCounter[outputIndex] - microsecondInterval[outputIndex];
    pinStateBuffer[outputIndex] = !pinStateBuffer[outputIndex];
    pulseStateBuffer[outputIndex] = pinStateBuffer[outputIndex];
    updatePins(pin, pedalPin, outputIndex, pinStateBuffer[outputIndex]);
  }
}

inline void updateMidi(int outputIndex)
{
  if (pedalPulseCounter[outputIndex] <= pedalPulseCountSetting[outputIndex]) { // Should we pulse the pedal?
    shouldMIDIPulse = pinStateBuffer[outputIndex];
  }
}

inline void updatePins(int pin, int pedalPin, int outputIndex, int state)
{
  digitalWrite(pin, state);
  if (pedalPulseCounter[outputIndex] <= pedalPulseCountSetting[outputIndex] * 2) { // Should we pulse the pedal?
    digitalWrite(pedalPin, state ^ pedalPulsePolaritySetting[outputIndex]); // sets the pin according to polarity
  } else if (pedalPulseCounter[outputIndex] == (pedalPulseCountSetting[outputIndex] * 2) + 1) { // After the last pulse
    digitalWrite(pedalPin, pedalPulsePolaritySetting[outputIndex]); //make sure we always end on the right polarity setting
  }
}

void noteOn(int cmd, int pitch, int velocity) 
{
  midi.write(cmd);
  midi.write(pitch);
  midi.write(velocity);
}

#endif

