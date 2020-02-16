#include <Arduino.h>
#include <WebSocketsClient.h>
#include <Servo.h>

#include "KapObjects.h"
#include "KapConfigStruct.h"

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
    Servo * _servo;
    bool connected = false;
  private:
    unsigned long _disconnectTime = 0;
    bool started = false;
    int _dep_any = 0;
    int _dep_cc = 0;
    int _dep_tp = 0;

    int _angle = 90;
    int _direction = 0;
    int _destination = 90;
    unsigned long _moveTime = 0;
    KapConfigParams* _configParams = NULL;
    
    
    void processServo(unsigned long now);
};

#endif
