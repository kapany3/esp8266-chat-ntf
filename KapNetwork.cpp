#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "KapNetwork.h"
#include "KapConfig.h"
#include "KapConfigStruct.h"
#include "KapServer.h"
#include "KapChat.h"
#include "KapObjects.h"

KapNetwork::KapNetwork(int port, KapObjects* kapObjects) {
  _kapObjects = kapObjects;
  _kapObjects->_server = new KapServer(kapObjects);

  KapConfigParams* conf = _kapObjects->_config->getConfig();

  if (conf->ssidName == "") {
    Serial.println("No WiFi connection information available.");
    startAP();
  } else {
    startStation(30);
  }

  if (!MDNS.begin("chat-ntf")) {
    Serial.println("Error setting up MDNS responder!");
  }  
}

void KapNetwork::toggleMode() {
  Serial.println("Toggle mode");
  if (_apMode == WIFI_STA) {
    startAP();
  } else {
    startStation(60);
  }
}


void KapNetwork::startAP() {
  _kapObjects->_server->serverEnd();
  Serial.println("Starting AP");
  _apMode = WIFI_AP;
  WiFi.mode(WIFI_AP);
  delay(10);
  IPAddress local_IP(192,168,10,1);
  IPAddress gateway(192,168,10,1);
  IPAddress subnet(255,255,255,0);
  if (WiFi.softAPConfig(local_IP, gateway, subnet)) {
    Serial.println("AP config OK");
  } else {
    Serial.println("AP config FAILED");
  }
  WiFi.softAP("CHATNTF", "ESP8266NTF");
  _kapObjects->_server->serverBegin(true);
  Serial.println("AP PARAMS:\nSSID: CHATNTF\nPASS: ESP8266NTF\nIP: 192.168.10.1");
}

void KapNetwork::startStation(int waitSecs) {
  if (_apMode != WIFI_STA && _kapObjects->_server->isStarted()) {
    _kapObjects->_server->serverEnd();
  }
  Serial.println("Starting station");
  _apMode = WIFI_STA;
  
  KapConfigParams* conf = _kapObjects->_config->getConfig();
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(conf->ssidName, conf->ssidPass);

  Serial.println("Trying to connect to ssid " + String(conf->ssidName));

  // Wait for connection
  int counter = 0;
  while (WiFi.status() != WL_CONNECTED && counter < waitSecs) {
    delay(1000);
    Serial.print(".");
    counter += 1;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to " + String(conf->ssidName));
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect");
    _disconnectTime = millis();
  }
}

bool KapNetwork::isConnected() {
  return _apMode == WIFI_STA && WiFi.status() == WL_CONNECTED;
}

void KapNetwork::checkConnection() {
  if (_apMode == WIFI_AP) {
    _kapObjects->_server->process();
  } else if (WiFi.status() == WL_CONNECTED) {
    if (_disconnectTime != 0) {
      Serial.println("WiFi connected");
      _disconnectTime = 0;
    }
    // Если в режиме точки доступа или подключен к роутеру
    if (_apMode == WIFI_STA) {
      _kapObjects->_chat->process();
    }
  } else if (_apMode == WIFI_STA) {
    // Информировать об отключении раз в 60 секунд
    unsigned long now = millis();
    if (now - _disconnectTime > 60000 || now < _disconnectTime) {
      Serial.println("WiFi disconnected");
      _disconnectTime = now;
    }
  }
}
