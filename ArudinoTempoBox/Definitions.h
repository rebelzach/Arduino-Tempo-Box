#ifndef DEFINITIONS_h
#define DEFINITIONS_h

#define ON HIGH
#define OFF LOW
#define YES HIGH
#define NO LOW

#define DEBUG 0 // Set this to zero when everything is good
#define debugBegin() do { if (1) Serial.begin(115200); } while (0)
#define debugPrint(x) do { if (DEBUG) Serial.print(x); } while (0)
#define debugPrintln(x) do { if (DEBUG) Serial.println(x); } while (0)
#define debugPrintMem() do { if (1) {Serial.print("MemoryFree"); Serial.println(freeMemory()); } } while (0)
#define BIG_LONG 4294967292
#define ALWAYS_PULSE 5000

#endif
