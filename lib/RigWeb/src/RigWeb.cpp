#include "RigWeb.h"

#include <ESP8266WiFi.h>
#include "RigPage.h"
#include "PartyPage.h"

void RigWeb::begin(bool reverseStatus, bool reverseSpine) {
  reverseStatus_ = reverseStatus;
  reverseSpine_ = reverseSpine;
  server_.on("/", HTTP_GET, [this]() {
    server_.sendHeader("Cache-Control", "no-store");
    server_.send_P(200, PSTR("text/html; charset=utf-8"), RIG_PAGE);
  });
  server_.on("/party", HTTP_GET, [this]() {
    server_.sendHeader("Cache-Control", "no-store");
    server_.send_P(200, PSTR("text/html; charset=utf-8"), PARTY_PAGE);
  });
  server_.on("/api/state", HTTP_GET, [this]() { sendState(); });
  server_.on("/api/settings", HTTP_POST, [this]() { settings(); });
  server_.on("/api/action", HTTP_POST, [this]() { action(); });
  server_.on("/api/party", HTTP_POST, [this]() { partySettings(); });
  server_.on("/api/mode", HTTP_POST, [this]() { mode(); });
  server_.onNotFound([this]() {
    server_.send(404, "application/json", "{\"error\":\"Not found\"}");
  });
}

void RigWeb::handle() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (!started_) {
    server_.begin();
    started_ = true;
    Serial.printf("RIG controls: http://%s/\n", WiFi.localIP().toString().c_str());
  }
  server_.handleClient();
}

void RigWeb::sendState() {
  const uint32_t now = millis();
  rig_.update(now);
  party_.update(now);
  const RigState state = rig_.state(now);
  const PartyState party = party_.state(now);
  char json[440];
  snprintf(json, sizeof(json),
           "{\"health\":%u,\"stasis\":%u,\"brightness\":%u,\"effect\":\"%s\",\"demo\":%s,\"powered\":%s,"
           "\"mode\":\"%s\",\"partyPattern\":\"%s\",\"partySpeed\":%u,\"partyColour\":\"#%06lx\","
           "\"partyCycle\":%s,\"statusReversed\":%s,\"spineReversed\":%s}",
           static_cast<unsigned>((state.health + 50) / 100),
           static_cast<unsigned>((state.stasis + 50) / 100),
           static_cast<unsigned>(state.brightness), rigEffectName(state),
           state.demo ? "true" : "false", party.active || state.effect != RigEffect::Off ? "true" : "false",
           party.active ? "party" : "rig", partyPatternName(party.pattern),
           static_cast<unsigned>(party.speed), static_cast<unsigned long>(party.colour),
           party.cycle ? "true" : "false", reverseStatus_ ? "true" : "false", reverseSpine_ ? "true" : "false");
  server_.sendHeader("Cache-Control", "no-store");
  server_.send(200, "application/json", json);
}

void RigWeb::badRequest(const char* message) {
  // Messages are fixed strings, never request values or credentials.
  char json[160];
  snprintf(json, sizeof(json), "{\"error\":\"%s\"}", message);
  server_.send(400, "application/json", json);
}

bool RigWeb::readNumber(const char* name, uint16_t maximum, uint8_t& result) {
  const String& value = server_.arg(name);
  if (value.length() == 0 || value.length() > 3) return false;
  uint16_t parsed = 0;
  for (unsigned i = 0; i < value.length(); ++i) {
    if (value[i] < '0' || value[i] > '9') return false;
    parsed = parsed * 10 + value[i] - '0';
  }
  if (parsed > maximum) return false;
  result = parsed;
  return true;
}

void RigWeb::settings() {
  const bool healthSet = server_.hasArg("health");
  const bool stasisSet = server_.hasArg("stasis");
  const bool brightnessSet = server_.hasArg("brightness");
  if (!healthSet && !stasisSet && !brightnessSet) {
    badRequest("Supply health, stasis or brightness");
    return;
  }
  uint8_t health = 0, stasis = 0, brightness = 0;
  // Validate the entire request before applying any changes.
  if (healthSet && !readNumber("health", 100, health)) {
    badRequest("Health must be an integer from 0 to 100"); return;
  }
  if (stasisSet && !readNumber("stasis", 100, stasis)) {
    badRequest("Stasis must be an integer from 0 to 100"); return;
  }
  if (brightnessSet && !readNumber("brightness", 255, brightness)) {
    badRequest("Brightness must be an integer from 0 to 255"); return;
  }
  const uint32_t now = millis();
  if (healthSet || stasisSet) party_.stop();
  if (healthSet) rig_.setHealth(health, now);
  if (stasisSet) rig_.setStasis(stasis, now);
  if (brightnessSet) rig_.setBrightness(brightness);
  sendState();
}

void RigWeb::action() {
  const String& action = server_.arg("action");
  const uint32_t now = millis();
  if (action == "demo-start" || action == "demo-stop") {
    party_.stop();
    rig_.setDemo(action == "demo-start", now);
    sendState();
    return;
  }
  struct Action { const char* name; RigCommand command; };
  static const Action actions[] = {
      {"power-up", RigCommand::PowerUp}, {"damage", RigCommand::Damage},
      {"heal", RigCommand::Heal}, {"critical", RigCommand::Critical},
      {"malfunction", RigCommand::Malfunction}, {"power-down", RigCommand::PowerDown},
      {"stasis-drain", RigCommand::DrainStasis}, {"stasis-recharge", RigCommand::RechargeStasis}};
  for (const Action& item : actions) {
    if (action == item.name) {
      party_.stop();
      rig_.trigger(item.command, now);
      sendState();
      return;
    }
  }
  badRequest("Unknown action");
}

void RigWeb::mode() {
  const String& mode = server_.arg("mode");
  const uint32_t now = millis();
  if (mode == "rig") {
    party_.stop();
    rig_.trigger(RigCommand::PowerUp, now);
  } else if (mode == "party") {
    rig_.setDemo(false, now);
    party_.start(now);
  } else {
    badRequest("Mode must be rig or party"); return;
  }
  sendState();
}

void RigWeb::partySettings() {
  const bool patternSet = server_.hasArg("pattern");
  const bool speedSet = server_.hasArg("speed");
  const bool colourSet = server_.hasArg("colour");
  const bool cycleSet = server_.hasArg("cycle");
  if (!patternSet && !speedSet && !colourSet && !cycleSet) {
    badRequest("Supply pattern, speed, colour or cycle"); return;
  }
  PartyPattern pattern = PartyPattern::Rainbow;
  uint8_t speed = 0;
  uint32_t colour = 0;
  if (patternSet && !parsePartyPattern(server_.arg("pattern").c_str(), pattern)) {
    badRequest("Unknown party pattern"); return;
  }
  if (speedSet && (!readNumber("speed", 100, speed) || speed == 0)) {
    badRequest("Speed must be an integer from 1 to 100"); return;
  }
  if (colourSet) {
    const String& text = server_.arg("colour");
    if (text.length() != 7 || text[0] != '#') {
      badRequest("Colour must use the format #RRGGBB"); return;
    }
    for (unsigned i = 1; i < 7; ++i) {
      const char c = text[i];
      uint8_t digit;
      if (c >= '0' && c <= '9') digit = c - '0';
      else if (c >= 'a' && c <= 'f') digit = c - 'a' + 10;
      else if (c >= 'A' && c <= 'F') digit = c - 'A' + 10;
      else { badRequest("Colour must use the format #RRGGBB"); return; }
      colour = (colour << 4) | digit;
    }
  }
  if (cycleSet && server_.arg("cycle") != "0" && server_.arg("cycle") != "1") {
    badRequest("Cycle must be 0 or 1"); return;
  }
  const uint32_t now = millis();
  rig_.setDemo(false, now);
  party_.start(now);
  if (patternSet) party_.select(pattern, now);
  if (speedSet) party_.setSpeed(speed);
  if (colourSet) party_.setColour(colour);
  if (cycleSet) party_.setCycle(server_.arg("cycle") == "1", now);
  sendState();
}
