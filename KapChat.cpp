#include <Arduino.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <Servo.h>

#include "KapChat.h"
#include "KapObjects.h"
#include "KapConfig.h"
#include "KapNetwork.h"
#include "KapConfigStruct.h"

KapChat* kapChat;

const int minAngle = 45;
const int maxAngle = 135;
const int servoSpeed = 3;

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
  _servo = new Servo();

  _servo->attach(2);
  _servo->write(90);
  delay(1000);
  _servo->detach();

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
  unsigned long now = millis();
  if (started) {
    _webSocket->loop();
  } else {
    if (now - _disconnectTime > 5000 || now < _disconnectTime) {
      Serial.println("No websocket connection");
      _disconnectTime = now;
      connect();
    }
  }
  processServo(now);
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

void KapChat::processServo(unsigned long now) {
  if (now - _moveTime < 30 && !(now < _moveTime) ) {
    return;
  }
  _moveTime = now;
  if (_configParams == NULL) {
    _configParams = _kapObjects->_config->getConfig();
  }

  if ( (_dep_cc == 0 || !_configParams->ccWorker)
    && (_dep_tp == 0 || !_configParams->tpWorker)
    && _angle < 90  && _direction < 0) {
    // Влево не надо двигаться больше, меняю направление
    Serial.println("1 SET DIRECTION TO 1, DEST TO 90");
    _direction = servoSpeed;
    _destination = 90;
  } else if (_dep_any == 0 && _angle > 90 && _direction > 0) {
    // Вправо не надо двигаться больше, меняю направление
    _direction = -servoSpeed;
    _destination = 90;
    Serial.println("1 SET DIRECTION TO -1, DEST TO 90");
  } else if (_dep_any == 0
          && (_dep_cc == 0 || !_configParams->ccWorker)
          && (_dep_tp == 0 || !_configParams->tpWorker)
          && (_angle >= 90 && _direction > 0 || _angle <= 90 && _direction < 0)
          && (_destination == 90)) {
    _direction = 0;
    _destination = 90;
    Serial.println("1 SET DIRECTION TO 0, DEST TO 90");
    return;
  }

  // Должен двигаться
  if (_direction == 0) {
    // Начало движения
    // cc и tp влево, any вправо
    if (_dep_cc > 0 && _configParams->ccWorker
     || _dep_tp > 0 && _configParams->tpWorker) {
      _direction = -servoSpeed;
      _destination = minAngle;
      Serial.println("SET DIRECTION TO -1, DESTINATION TO 0");
    } else if (_dep_any > 0) {
      _direction = servoSpeed;
      _destination = maxAngle;
      Serial.println("SET DIRECTION TO 1, DESTINATION TO 180");
    } else {
      if (_servo->attached()) {
        _servo->detach();
      }
      return;
    }
    Serial.println("ATTACH");
    _servo->attach(2); // d4
  }

  Serial.printf("ANGLE: %d, DIRECTION: %d, DESTINATION: %d, cc: %d, tp: %d, any: %d\n", _angle, _direction, _destination, _dep_cc, _dep_tp, _dep_any);
  if (_angle == _destination // Точное совпадение
   || _angle < _destination && _direction < 0 // Проскочил точку двигаясь справа
   || _angle > _destination && _direction > 0 // Проскочил точку двигаясь слева
    ) {
    Serial.println("ANGLE EQUAL TO DESTINATION");
    if (_destination <= minAngle && _direction < 0
     || _destination >= maxAngle && _direction > 0) {
      // Если двигались к краям, то движемся к центру
      _destination = 90;
      _direction = -_direction;
      Serial.println("SET DIRECTION TO -DIRECTION, DESTINATION TO 90");
    } else if (_destination == 90 && _direction > 0) {
      // Двигался к центру слева
      if (_dep_any > 0) {
        _destination = maxAngle;
        Serial.println("SET DESTINATION TO 180");
      } else if (_dep_cc > 0 && _configParams->ccWorker
              || _dep_tp > 0 && _configParams->tpWorker) {
        _destination = minAngle;
        _direction = -servoSpeed;
        Serial.println("SET DIRECTION TO -1, DESTINATION TO 0");
      } else {
        _direction = 0;
        _servo->detach();
        Serial.println("STOP");
      }
    } else if (_destination == 90 && _direction < 0) {
      // Двигался к центру справа
      if (_dep_cc > 0 && _configParams->ccWorker
       || _dep_tp > 0 && _configParams->tpWorker) {
        _destination = minAngle;
        Serial.println("SET DESTINATION TO 0");
      } else if (_dep_any > 0) {
        _destination = minAngle;
        _direction = -servoSpeed;
        Serial.println("SET DIRECTION TO -1, DESTINATION TO 0");
      } else {
        _direction = 0;
        _servo->detach();
        Serial.println("STOP");
      }
    }
  }
  if (_direction != 0) {
    _angle += _direction;
    Serial.printf("MOVE TO %d\n", _angle);
    _servo->write(_angle);
  }
}
