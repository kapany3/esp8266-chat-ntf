#include <Arduino.h>

#ifndef KapObjects_h
#define KapObjects_h

class KapConfig;
class KapServer;
class KapNetwork;
class KapChat;

class KapObjects
{
  public:
    KapObjects();
    
    KapConfig* _config = NULL;
    KapServer* _server = NULL;
    KapNetwork* _network = NULL;
    KapChat* _chat = NULL;
};

#endif
