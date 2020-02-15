#include <Arduino.h>
#include <ESP8266WebServer.h>
#include "KapServer.h"
#include "KapConfig.h"
#include "KapNetwork.h"
#include "KapObjects.h"

KapServer::KapServer(KapObjects* kapObjects) {
  _kapObjects = kapObjects;
  _server = new ESP8266WebServer(80);
  Serial.println("Server created");
}

void KapServer::process() {
  _server->handleClient();
}

bool KapServer::isStarted() {
  return _isStarted;
}

void KapServer::serverBegin(bool isAP) {
  Serial.println("Start server");
  _isAP = isAP;
  _server->on("/", std::bind(&KapServer::handleRoot, this));
  _server->onNotFound(std::bind(&KapServer::handleNotFound, this));
  if (isAP) {
    _server->on("/getConfig", std::bind(&KapServer::handleGetConfig, this));
    _server->on("/setConfig", std::bind(&KapServer::handleSetConfig, this));
  }
  _server->begin();
  _isStarted = true;
}

void KapServer::serverEnd() {
  if (_isStarted) {
    Serial.println("Stop server");
    _server->stop();
    _isStarted = false;
  }
}

void KapServer::handleRoot() {
  String message = "";
  uint8_t argsCount = _server->args();

  if (_isAP) {
    handleFileRead("/config.html");
  } else {
    _server->send(200, "text/plain", "Hello");
  }
}

void KapServer::handleGetConfig() {
  String message = "";
  StaticJsonDocument<512> doc;

  KapConfigParams* conf = _kapObjects->_config->getConfig();

  doc["ssidName"] = conf->ssidName;
  doc["ssidPass"] = conf->ssidPass;
  doc["server"] = conf->server;
  doc["cc_worker"] = conf->ccWorker;
  doc["tp_worker"] = conf->tpWorker;

  if (serializeJson(doc, message) == 0) {
    Serial.println(F("Failed to write to string"));
  }

  _server->send(200, "text/plain", message);
}

void KapServer::handleSetConfig() {
  String message = "OK";
  KapConfigParams* conf = _kapObjects->_config->getConfig();
  
  uint8_t argsCount = _server->args();
  char buf[32];
  for (int i = 0; i < argsCount; i += 1) {
    String argName = _server->argName(i);
    Serial.println("Argument: " + argName);
    _server->arg(i).toCharArray(buf, 32);
    Serial.println("Value: " + String(buf));
    
    if (argName == "ssid_name") {
      strlcpy(conf->ssidName, buf, sizeof(conf->ssidName));
    } else if (argName == "ssid_pass") {
      strlcpy(conf->ssidPass, buf, sizeof(conf->ssidPass));
    } else if (argName == "server") {
      strlcpy(conf->server, buf, sizeof(conf->server));
    } else if (argName == "cc_worker") {
      conf->ccWorker = buf[0] == '1';
    } else if (argName == "tp_worker") {
      conf->tpWorker = buf[0] == '1';
    }
  }
  _kapObjects->_config->saveConfig();
  // _kapObjects->_network->toggleMode();
  _server->send(200, "text/plain", message);
}

void KapServer::handleNotFound() {
  if (!handleFileRead(_server->uri())) {
      _server->send(404, "text/plain", "404: Not Found");
  }
}

String KapServer::getContentType(String filename) {
  if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool KapServer::handleFileRead(String path) {  // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  if(path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz)) {
      path += ".gz";
    }
    File file = SPIFFS.open(path, "r");
    size_t sent = _server->streamFile(file, contentType);
    file.close();
    Serial.println(String("\tSent file: ") + path);
    return true;
  }
  Serial.println(String("\tFile Not Found: ") + path);
  return false;
}
