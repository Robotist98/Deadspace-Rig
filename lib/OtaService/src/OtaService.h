#pragma once

// One service per device: the implementation manages the global ArduinoOTA server.
class OtaService {
 public:
  struct Config {
    const char* ssid;
    const char* wifiPassword;
    const char* hostname;
    const char* otaPassword;
    const char* staticIP;
    const char* gateway;
    const char* subnet;
    const char* dns;
  };

  using StartCallback = void (*)();

  // Configuration strings must remain valid for the lifetime of the service.
  // Start Serial before calling begin() to receive connection/update messages.
  void begin(const Config& config, StartCallback onStart = nullptr);
  void handle();
  bool isUpdating() const;

 private:
  void configureNetwork();
  void configureOTA();

  Config config_{};
  StartCallback onStart_ = nullptr;
  unsigned long lastWifiAttempt_ = 0;
  bool configured_ = false;
  bool wasConnected_ = false;
  bool otaStarted_ = false;
  bool updating_ = false;
};
