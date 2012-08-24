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

int TAP_TEMPO_PIN = 7;
int output1pin = 8;
int output2pin = 9;
int output3pin = 10;
int output4pin = 12;
int pedalPin1 = A0;
int pedalPin2 = A1;
int pedalPin3 = A2;
int pedalPin4 = A3;

const int encRed = 5;
const int encGreen = 6;
const int encBlue = 11;

#define TAP_TIMEOUT 2000000
#define TAP_MINIMUM 8000
#define TIMER_QUANTUM 400 //100

volatile int shouldPulse;
volatile unsigned long averagePulseLength;
volatile unsigned long microsecondInterval[4] = {0,0,0,0};
volatile unsigned long microsecondCounter[4] = {0,0,0,0};
volatile unsigned long microsecondPulseLenSetting[4] = {4294967292,100000,4294967292,4294967292};
volatile unsigned long beatCounter;
float beatLevel = 255;
volatile int pedalPulseCountSetting[4] = {8,4000,4000,16};
volatile int pedalPulsePolaritySetting[4] = {1,1,1,1};
float pedalPulseRateSetting[4] = {.25,1,1,2};
volatile int pedalPulseCounter[4] = {0,0,0,0};

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
  pinMode(TAP_TEMPO_PIN, INPUT);
  Timer1.initialize(TIMER_QUANTUM);
  Timer1.attachInterrupt(pulseInterrupt); // attach the service routine here
  tempoChangeCallback = NULL;
  setTempo(120);
  shouldPulse = NO;
  midi.setTX(A4);
  midi.begin(31250);
}

float previousBeatLevel = 0;
unsigned long previousBeatCounter = 0;
float tempoColorVector[3] = {0,0,0};
float rangedTempo = .5;

void TempoBoss::processLoop()
{
  processPendingTempoTaps();
  if (shouldPulse == 1) {
    noteOn(176, 64, 127);
    shouldPulse = 0;
  }
  if (beatLevel != previousBeatLevel) {
    previousBeatLevel = beatLevel;
    analogWrite(encRed, 255 - (beatLevel * rangedTempo));
    analogWrite(encGreen, 255 - (beatLevel * (1 - rangedTempo) * .5));
    analogWrite(encBlue, 255 - (beatLevel * (1 - rangedTempo)));
  }
}

int pinStateBuffer[13] = {1,1,1,1,1,1,1,1,1,1,1,1,1};
int previousPinStateBuffer[13] = {0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned long pinPulseBeginTime[13] = {0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned long briefPulseDuration = 100000;

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
      Serial.println("Timed Out");
      return; // since we timed out
    }
    
    Serial.print("Tempo: ");
    Serial.println(calculateTempo());
    
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
  calculateAndSetIntervals(averagePulseLength); // 120BPM
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
  
  microsecondInterval[0] = (compensatedPulseLength / pedalPulseRateSetting[0]);
  microsecondInterval[1] = (compensatedPulseLength / pedalPulseRateSetting[1]);
  microsecondInterval[2] = (compensatedPulseLength / pedalPulseRateSetting[2]);
  microsecondInterval[3] = (compensatedPulseLength / pedalPulseRateSetting[3]);
}

void TempoBoss::rePollSettings()
{
  calculateAndSetIntervals(averagePulseLength);
}

void TempoBoss::resetPulseCounters() 
{
  debugPrintln("Reset");
  debugPrintln(microsecondInterval[1]);
  debugPrintln(microsecondCounter[1]);
  microsecondCounter[0] = 0;
  microsecondCounter[1] = 0;
  microsecondCounter[2] = 0;
  microsecondCounter[3] = 0;
  pinStateBuffer[output1pin] = 1;
  pinStateBuffer[output2pin] = 1;
  pinStateBuffer[output3pin] = 1;
  pinStateBuffer[output4pin] = 1;
  digitalWrite(output1pin, !pedalPulsePolaritySetting[0]);
  digitalWrite(output2pin, !pedalPulsePolaritySetting[1]);
  digitalWrite(output3pin, !pedalPulsePolaritySetting[2]);
  digitalWrite(output4pin, !pedalPulsePolaritySetting[3]);
  pedalPulseCounter[0] = 0;
  pedalPulseCounter[1] = 0;
  pedalPulseCounter[2] = 0;
  pedalPulseCounter[3] = 0;
  beatCounter = 255;
}

volatile unsigned long pwmUpdateCounter = 0;
inline void incrementMidiCounter(int pulseIndex, int pin, int pedalPin);

void pulseInterrupt() {
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
inline void updatePins(int pin, int pedalPin);

inline void incrementPulseCounter(int pulseIndex, int pin, int pedalPin)
{
  if (microsecondCounter[pulseIndex] > microsecondInterval[pulseIndex]) {
    microsecondCounter[pulseIndex] = microsecondCounter[pulseIndex] - microsecondInterval[pulseIndex];
    
    pinStateBuffer[pin] = !pinStateBuffer[pin];
    updatePins(pin, pedalPin);
  } else if (pinStateBuffer[pin] == 1 && microsecondCounter[pulseIndex] > microsecondPulseLenSetting[pulseIndex]) {
    pinStateBuffer[pin] = 0;
    updatePins(pin, pedalPin);
  }
  
}

inline void incrementMidiCounter(int pulseIndex, int pin, int pedalPin)
{
  if (microsecondCounter[pulseIndex] > microsecondInterval[pulseIndex]) {
    microsecondCounter[pulseIndex] = microsecondCounter[pulseIndex] - microsecondInterval[pulseIndex];
    
    pinStateBuffer[pin] = !pinStateBuffer[pin];
    shouldPulse = 1;
    digitalWrite(pin, pinStateBuffer[pin]);
  } else if (pinStateBuffer[pin] == 1 && microsecondCounter[pulseIndex] > microsecondPulseLenSetting[pulseIndex]) {
    pinStateBuffer[pin] = 0;
    digitalWrite(pin, pinStateBuffer[pin]);
  }
}

inline void updatePins(int pin, int pedalPin)
{
  digitalWrite(pin, pinStateBuffer[pin]);
  digitalWrite(pedalPin, (pinStateBuffer[pin]) ^ pedalPulsePolaritySetting[pulseIndex]);
//  if (pedalPulseCounter[pulseIndex] <= pedalPulseCountSetting[pulseIndex]) { // Should we pulse the pedal?
//    digitalWrite(pedalPin, (pinStateBuffer[pin]) ^ pedalPulsePolaritySetting[pulseIndex]); // sets the pin according to polarity
//    pedalPulseCounter[pulseIndex]++;
//  } else if (pedalPulseCounter[pulseIndex] == pedalPulseCountSetting[pulseIndex] + 1) { // After the last pulse
//    digitalWrite(pedalPin, pedalPulsePolaritySetting[pulseIndex]); //make sure we always end on the right polarity setting
//  }
}

void noteOn(int cmd, int pitch, int velocity) 
{
  midi.write(cmd);
  midi.write(pitch);
  midi.write(velocity);
}

#endif
