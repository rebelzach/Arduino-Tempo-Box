#ifndef DEFINITIONS_h
#define DEFINITIONS_h

#define ON HIGH
#define OFF LOW
#define YES HIGH
#define NO LOW

#define DEBUG 1 // Set this to zero when everything is good
#define debugBegin() do { if (DEBUG) Serial.begin(115200); } while (0)
#define debugPrint(x) do { if (DEBUG) Serial.print(x); } while (0)
#define debugPrintln(x) do { if (DEBUG) Serial.println(x); } while (0)

#endif