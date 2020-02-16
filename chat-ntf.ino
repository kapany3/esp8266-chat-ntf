#include "KapConfig.h"
#include "KapNetwork.h"
#include "KapChat.h"

KapObjects* kapObjects;
/*
#ifdef DEBUG_ESP_PORT
#define DEBUG_MSG(...) DEBUG_ESP_PORT.printf( __VA_ARGS__ )
#define DEBUG_WEBSOCKETS(...) os_printf( __VA_ARGS__ )
#else
#define DEBUG_MSG(...)
#endif
*/

void setup() {
  Serial.begin(115200);
  delay(1000);  
  Serial.println("\n\n\nHello, All");
  kapObjects = new KapObjects();
  kapObjects->_config = new KapConfig();
  kapObjects->_chat = new KapChat(kapObjects);
  kapObjects->_network = new KapNetwork(80, kapObjects);

  Serial.println("\nInit finished");
}

void loop() {
  kapObjects->_network->checkConnection();  
  
  String readString;
  while (Serial.available()) {
    char c = Serial.read();
    readString += c;
    delay(2);
  }

  if (readString.length() > 0) {
    readString.trim();
    if (readString == "MODE") {
      kapObjects->_network->toggleMode();
    } else {
      Serial.println("STRING: '" + readString + "'");
    }
    readString="";
  }
}
