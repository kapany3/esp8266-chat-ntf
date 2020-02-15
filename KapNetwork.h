#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "KapObjects.h"

#ifndef KapNetwork_h
#define KapNetwork_h

class KapNetwork
{
  public:
    KapNetwork(int, KapObjects* kapObjects);
    void toggleMode();
    void startAP();
    void startStation(int waitSecs);
    void checkConnection();
    bool isConnected();
    void onDisconnect();
    void onConnect();
    
  private:
    unsigned long _disconnectTime = 0;
    int _apMode = WIFI_STA;
    KapObjects* _kapObjects = NULL;
    void handleRoot();
    void handleNotFound();
    void handleGetConfig();
    void handleSetConfig();
    String getContentType(String filename);
    bool handleFileRead(String path);
};

#endif
