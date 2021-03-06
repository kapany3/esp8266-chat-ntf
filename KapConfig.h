#include <Arduino.h>
#include <ArduinoJson.h>
#include "KapConfigStruct.h"
#include "KapObjects.h"

#ifndef KapConfig_h
#define KapConfig_h

class KapConfig {
  public:
    KapConfig();
    KapConfigParams *getConfig();
    bool saveConfig();
    void setSsid(const char* ssid, const char* pass);
    void setServer(const char* server);
  private:
    KapObjects* _kapObjects = NULL;
    KapConfigParams _configParams;
    void loadConfig();
};


#endif
