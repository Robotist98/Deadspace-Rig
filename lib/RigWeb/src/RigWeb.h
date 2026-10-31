#pragma once

#include <ESP8266WebServer.h>
#include <RigController.h>
#include <PartyLighting.h>

class RigWeb {
 public:
  RigWeb(RigController& rig, PartyLighting& party) : rig_(rig), party_(party) {}
  void begin(bool reverseStatus, bool reverseSpine);
  void handle();

 private:
  void sendState();
  void settings();
  void action();
  void partySettings();
  void mode();
  void badRequest(const char* message);
  bool readNumber(const char* name, uint16_t maximum, uint8_t& result);

  RigController& rig_;
  PartyLighting& party_;
  bool reverseStatus_ = false;
  bool reverseSpine_ = false;
  ESP8266WebServer server_{80};
  bool started_ = false;
};
