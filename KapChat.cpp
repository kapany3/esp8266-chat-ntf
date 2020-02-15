#include <Arduino.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>

#include "KapChat.h"
#include "KapObjects.h"
#include "KapConfig.h"
#include "KapNetwork.h"

KapChat* kapChat;

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      kapChat->connected = false;
      Serial.printf("[WSc] Disconnected!\n");
      break;
    case WStype_CONNECTED:
      kapChat->connected = true;
      Serial.printf("[WSc] Connected to url: %s\n", payload);
      kapChat->_webSocket->sendTXT("{\"id\":1,\"type\":\"init-watcher\"}");
      break;
    case WStype_TEXT: {
      Serial.printf("[WSc] get text: %s\n", payload);
      kapChat->processMessage(payload);
      break;
    }
    case WStype_BIN:
      Serial.printf("[WSc] get binary length: %u\n", length);
      break;
  }
}


KapChat::KapChat(KapObjects* kapObjects)
{
  _kapObjects = kapObjects;
  _webSocket = new WebSocketsClient();

  kapChat = this;
  Serial.println("Chat created");
}

void KapChat::connect() {
  if (_kapObjects->_network 
    && _kapObjects->_network->isConnected()) {
    Serial.println("Connecting websocket");
    KapConfigParams* conf = _kapObjects->_config->getConfig();

    _webSocket->begin(conf->server, 9341, "/", "worker");
    _webSocket->onEvent(webSocketEvent);
    _webSocket->setReconnectInterval(5000);
    _webSocket->enableHeartbeat(15000, 3000, 2);
    started = true;
  }
}

void KapChat::process(){
  if (started) {
    _webSocket->loop();
  } else {
    unsigned long now = millis();
    if (now - _disconnectTime > 5000 || now < _disconnectTime) {
      Serial.println("No websocket connection");
      _disconnectTime = now;
      connect();
    }
  }
}

void KapChat::processMessage(uint8_t * payload) {
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    return;
  }

  String type = doc["type"].as<char*>();
  String dep;
  if (type == "new-chat") {
    Serial.print("New chat: ");
    dep = doc["content"]["department"].as<char*>();
    Serial.println(dep);

    if (dep == "any") {
      _dep_any += 1;
    } else if (dep == "cc") {
      _dep_cc += 1;
    } else if (dep == "tp") {
      _dep_tp += 1;
    }
  } else if (type == "picked-up") {
    Serial.println("Chat picked up: ");
    dep = doc["content"]["department_p"].as<char*>();
    Serial.println(dep);

    if (dep == "any") {
      _dep_any -= 1;
    } else if (dep == "cc") {
      _dep_cc -= 1;
    } else if (dep == "tp") {
      _dep_tp -= 1;
    }
  } else if (type == "watch-ok") {
    // {"id":1581799638068,"type":"watch-ok","content":{"sessions":[{"department":"cc","count":1},{"department":"tp","count":1}]}}
    JsonArray deps = doc["content"]["sessions"].as<JsonArray>();
    for (int i = 0, l = deps.size(); i < l; i += 1) {
      JsonObject obj = deps[i].as<JsonObject>();
      dep = obj["department"].as<char*>();
      int count = obj["count"].as<int>();
      if (dep == "any") {
        _dep_any = count;
      } else if (dep == "cc") {
        _dep_cc = count;
      } else if (dep == "tp") {
        _dep_tp = count;
      }
    }
  }

  Serial.print("CC: "); Serial.println(_dep_cc);
  Serial.print("TP: "); Serial.println(_dep_tp);
  Serial.print("ANY: "); Serial.println(_dep_any);
}
