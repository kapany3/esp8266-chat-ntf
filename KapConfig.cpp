#include <Arduino.h>
#include <ArduinoJson.h>
#include "FS.h"
#include "KapConfigStruct.h"
#include "KapConfig.h"
#include "KapObjects.h"

KapConfig::KapConfig() {
  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }

  loadConfig();
  Serial.println("Config created");
  delay(1000);  
}

void KapConfig::loadConfig() {
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config.json");
  }
  Serial.println("Loading config");
  String content = configFile.readString();
  configFile.close();

  Serial.println(content);
  
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, content);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
  }

  Serial.println("Config parsed");
  
  strlcpy(_configParams.ssidName, doc["ssidName"] | "", sizeof(_configParams.ssidName));
  Serial.println("SSID NAME: " + String(_configParams.ssidName));
  strlcpy(_configParams.ssidPass, doc["ssidPass"] | "", sizeof(_configParams.ssidPass));
  Serial.println("SSID PASS: " + String(_configParams.ssidPass));
  strlcpy(_configParams.server, doc["server"] | "", sizeof(_configParams.server));
  Serial.println("Server: " + String(_configParams.server));
  _configParams.ccWorker = doc["cc_worker"];
  Serial.println("CC: " + String(_configParams.ccWorker ? "Y" : "N"));
  _configParams.tpWorker = doc["tp_worker"];
  Serial.println("TP: " + String(_configParams.tpWorker ? "Y" : "N"));
}

void KapConfig::setSsid(const char* ssid, const char* pass) {
  strlcpy(_configParams.ssidName, ssid, sizeof(_configParams.ssidName));
  strlcpy(_configParams.ssidPass, pass, sizeof(_configParams.ssidPass));
}
void KapConfig::setServer(const char* server) {
  strlcpy(_configParams.server, server, sizeof(_configParams.server));
}

bool KapConfig::saveConfig() {
  Serial.println("Save config");
  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config.json for writing");
    return false;
  }

  StaticJsonDocument<512> doc;

  doc["ssidName"] = _configParams.ssidName;
  doc["ssidPass"] = _configParams.ssidPass;
  doc["server"] = _configParams.server;
  doc["cc_worker"] = _configParams.ccWorker;
  doc["tp_worker"] = _configParams.tpWorker;

  if (serializeJson(doc, configFile) == 0) {
    Serial.println(F("Failed to write to file"));
  } else {
    Serial.println("Config saved");
  }
  
  configFile.close();
  return true;
}

KapConfigParams* KapConfig::getConfig() {
  return &_configParams;
}
