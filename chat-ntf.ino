#include "KapConfig.h"
#include "KapNetwork.h"
#include "KapChat.h"

KapObjects* kapObjects;

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
