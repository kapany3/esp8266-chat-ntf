#include <Arduino.h>
#include <WebSocketsClient.h>

#include "KapObjects.h"

#ifndef KapChat_h
#define KapChat_h

class KapChat
{
  public:
    KapChat(KapObjects*);
    void process();
    void connect();
    void processMessage(uint8_t * payload);
    KapObjects* _kapObjects;
    WebSocketsClient* _webSocket;
    bool connected = false;
  private:
    unsigned long _disconnectTime = 0;
    bool started = false;
    int _dep_any = 0;
    int _dep_cc = 0;
    int _dep_tp = 0;
};

#endif
