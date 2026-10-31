#pragma once

#include <FastLED.h>

enum class PartyPattern : uint8_t { Rainbow, Wave, Chase, Sparkle, Breathe, Scanner, Comet, Solid, Strobe };

struct PartyState {
  bool active;
  bool cycle;
  PartyPattern pattern;
  uint8_t speed;
  uint32_t colour;
  uint32_t elapsedMs;
};

class PartyLighting {
 public:
  void begin(uint32_t now);
  void update(uint32_t now);
  void start(uint32_t now);
  void stop();
  void select(PartyPattern pattern, uint32_t now);
  void setSpeed(uint8_t speed);
  void setColour(uint32_t colour);
  void setCycle(bool enabled, uint32_t now);
  PartyState state(uint32_t now) const;

 private:
  bool active_ = false;
  bool cycle_ = false;
  PartyPattern pattern_ = PartyPattern::Rainbow;
  uint8_t speed_ = 50;
  uint32_t colour_ = 0xff35a1;
  uint32_t started_ = 0;
  uint32_t cycleStarted_ = 0;
};

const char* partyPatternName(PartyPattern pattern);
bool parsePartyPattern(const char* name, PartyPattern& pattern);
void renderParty(CRGB* leds, uint16_t count, const PartyState& state);
