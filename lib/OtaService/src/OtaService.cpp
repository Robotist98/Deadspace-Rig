#include "OtaService.h"

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>

namespace {
constexpr unsigned long WIFI_RETRY_INTERVAL_MS = 30000;
}

void OtaService::begin(const Config& config, StartCallback onStart) {
  if (configured_) {
    return;
  }

  if (!config.ssid || config.ssid[0] == '\0' ||
      !config.otaPassword || config.otaPassword[0] == '\0') {
    Serial.println("Set WIFI_SSID, WIFI_PASSWORD and OTA_PASSWORD in include/config.h, then flash over USB.");
    return;
  }

  config_ = config;
  onStart_ = onStart;
  configureOTA();

  // Keep credentials in the firmware instead of rewriting Wi-Fi flash settings.
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  if (config_.hostname && config_.hostname[0] != '\0') {
    WiFi.hostname(config_.hostname);
  }
  WiFi.setAutoReconnect(true);
  configureNetwork();
  WiFi.begin(config_.ssid, config_.wifiPassword ? config_.wifiPassword : "");
  lastWifiAttempt_ = millis();
  configured_ = true;
  Serial.println("Connecting to Wi-Fi...");
}

void OtaService::configureNetwork() {
  if (!config_.staticIP || config_.staticIP[0] == '\0') {
    Serial.println("Using automatic IP addressing (DHCP)");
    return;
  }

  IPAddress localIP;
  IPAddress gateway;
  IPAddress subnet;
  IPAddress dns;
  bool valid = config_.gateway && config_.subnet &&
               localIP.fromString(config_.staticIP) &&
               gateway.fromString(config_.gateway) &&
               subnet.fromString(config_.subnet);

  if (!config_.dns || config_.dns[0] == '\0') {
    dns = gateway;
  } else {
    valid = valid && dns.fromString(config_.dns);
  }

  const IPAddress unspecified(0, 0, 0, 0);
  if (!valid || localIP == unspecified || gateway == unspecified ||
      subnet == unspecified || localIP == gateway ||
      !WiFi.config(localIP, gateway, subnet, dns)) {
    Serial.println("Could not apply static IP settings; using DHCP instead");
    WiFi.config(unspecified, unspecified, unspecified);
    return;
  }

  Serial.printf("Using static IP: %s\n", config_.staticIP);
}

void OtaService::configureOTA() {
  if (config_.hostname && config_.hostname[0] != '\0') {
    ArduinoOTA.setHostname(config_.hostname);
  }
  ArduinoOTA.setPassword(config_.otaPassword);

  ArduinoOTA.onStart([this]() {
    updating_ = true;
    if (onStart_) {
      onStart_();
    }
    Serial.println("OTA update started");
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("OTA update complete. Restarting...");
  });

  ArduinoOTA.onError([this](ota_error_t error) {
    updating_ = false;
    Serial.printf("OTA error [%u]: ", static_cast<unsigned int>(error));
    switch (error) {
      case OTA_AUTH_ERROR: Serial.println("Authentication failed"); break;
      case OTA_BEGIN_ERROR: Serial.println("Could not begin update"); break;
      case OTA_CONNECT_ERROR: Serial.println("Connection failed"); break;
      case OTA_RECEIVE_ERROR: Serial.println("Receive failed"); break;
      case OTA_END_ERROR: Serial.println("Could not finish update"); break;
      default: Serial.println("Unknown error"); break;
    }
  });
}

void OtaService::handle() {
  if (!configured_) {
    return;
  }

  const bool connected = WiFi.status() == WL_CONNECTED;
  if (connected) {
    if (!otaStarted_) {
      ArduinoOTA.begin();
      otaStarted_ = true;
    }

    if (!wasConnected_) {
      Serial.printf("OTA ready: %s.local (%s), port 8266\n",
                    ArduinoOTA.getHostname().c_str(), WiFi.localIP().toString().c_str());
      Serial.printf("Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
      Serial.printf("Subnet mask: %s\n", WiFi.subnetMask().toString().c_str());
      Serial.printf("MAC address: %s\n", WiFi.macAddress().c_str());
    }

    ArduinoOTA.handle();
  } else {
    if (wasConnected_) {
      Serial.println("Wi-Fi disconnected. Reconnecting...");
    }

    if (millis() - lastWifiAttempt_ >= WIFI_RETRY_INTERVAL_MS) {
      lastWifiAttempt_ = millis();
      Serial.println("Retrying Wi-Fi connection...");
      WiFi.reconnect();
    }
  }

  wasConnected_ = connected;
}

bool OtaService::isUpdating() const {
  return updating_;
}
