#include <Arduino.h>
#include <ArduinoJson.h>

#ifndef KapConfigStruct_h
#define KapConfigStruct_h

typedef struct {
  char ssidName[32];
  char ssidPass[32];
  char server[32];
  bool ccWorker;
  bool tpWorker;
}  KapConfigParams;

#endif
