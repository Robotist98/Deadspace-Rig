#include "PartyLighting.h"

#include <string.h>

namespace {
const char* const NAMES[] = {"rainbow", "wave", "chase", "sparkle", "breathe", "scanner", "comet", "solid", "strobe"};
constexpr uint8_t PATTERNS = sizeof(NAMES) / sizeof(NAMES[0]);
constexpr uint32_t CYCLE_MS = 10000;

CRGB wheel(uint8_t hue) {
  if (hue < 85) return CRGB(255 - hue * 3, hue * 3, 0);
  if (hue < 170) { hue -= 85; return CRGB(0, 255 - hue * 3, hue * 3); }
  hue -= 170;
  return CRGB(hue * 3, 0, 255 - hue * 3);
}
}

void PartyLighting::begin(uint32_t now) {
  active_ = cycle_ = false;
  pattern_ = PartyPattern::Rainbow;
  speed_ = 50;
  colour_ = 0xff35a1;
  started_ = cycleStarted_ = now;
}

void PartyLighting::update(uint32_t now) {
  if (!active_ || !cycle_) return;
  const uint32_t steps = (now - cycleStarted_) / CYCLE_MS;
  if (steps) {
    pattern_ = static_cast<PartyPattern>((static_cast<uint8_t>(pattern_) + steps % PATTERNS) % PATTERNS);
    cycleStarted_ += steps * CYCLE_MS;
    started_ = cycleStarted_;
  }
}

void PartyLighting::start(uint32_t now) {
  if (!active_) started_ = cycleStarted_ = now;
  active_ = true;
}

void PartyLighting::stop() { active_ = cycle_ = false; }

void PartyLighting::select(PartyPattern pattern, uint32_t now) {
  pattern_ = static_cast<uint8_t>(pattern) < PATTERNS ? pattern : PartyPattern::Rainbow;
  started_ = cycleStarted_ = now;
  active_ = true;
  cycle_ = false;
}

void PartyLighting::setSpeed(uint8_t speed) { speed_ = speed < 1 ? 1 : speed > 100 ? 100 : speed; }
void PartyLighting::setColour(uint32_t colour) { colour_ = colour & 0xffffffUL; }

void PartyLighting::setCycle(bool enabled, uint32_t now) {
  cycle_ = enabled;
  cycleStarted_ = now;
}

PartyState PartyLighting::state(uint32_t now) const {
  return {active_, cycle_, pattern_, speed_, colour_, now - started_};
}

const char* partyPatternName(PartyPattern pattern) {
  const uint8_t index = static_cast<uint8_t>(pattern);
  return NAMES[index < PATTERNS ? index : 0];
}

bool parsePartyPattern(const char* name, PartyPattern& pattern) {
  if (!name) return false;
  for (uint8_t i = 0; i < PATTERNS; ++i) {
    if (strcmp(name, NAMES[i]) == 0) { pattern = static_cast<PartyPattern>(i); return true; }
  }
  return false;
}

void renderParty(CRGB* leds, uint16_t count, const PartyState& state) {
  if (!leds || count == 0) return;
  fill_solid(leds, count, CRGB::Black);
  if (!state.active) return;
  const uint8_t speed = state.speed < 1 ? 1 : state.speed > 100 ? 100 : state.speed;
  const uint16_t stepMs = 230 - speed * 2;
  const uint32_t step = state.elapsedMs / stepMs;
  const uint16_t period = 12000 - speed * 100;
  const uint8_t phase = (state.elapsedMs % period) * 256UL / period;
  const CRGB selected(state.colour >> 16, state.colour >> 8, state.colour);
  for (uint16_t i = 0; i < count; ++i) {
    switch (state.pattern) {
      case PartyPattern::Rainbow:
      case PartyPattern::Wave:
        leds[i] = wheel(phase + static_cast<uint32_t>(i) * 256 / count);
        if (state.pattern == PartyPattern::Wave)
          leds[i].nscale8_video(40 + scale8(sin8(phase * 2 + i * 17), 215));
        break;
      case PartyPattern::Chase:
        if ((i + step) % 3 == 0) leds[i] = selected;
        break;
      case PartyPattern::Sparkle: {
        uint32_t noise = (step + 1) * 1664525UL + (i + 1) * 1013904223UL;
        noise ^= noise >> 13;
        leds[i] = selected;
        leds[i].nscale8_video(noise % 13 == 0 ? 255 : 12);
        if (noise % 29 == 0) leds[i] = CRGB(255, 255, 255);
        break;
      }
      case PartyPattern::Breathe:
        leds[i] = selected;
        leds[i].nscale8_video(30 + scale8(sin8(phase), 225));
        break;
      case PartyPattern::Scanner: {
        const uint32_t travel = count > 1 ? 2UL * (count - 1) : 1;
        uint32_t head = step % travel;
        if (head >= count) head = travel - head;
        const uint32_t distance = i > head ? i - head : head - i;
        if (distance < 5) { leds[i] = selected; leds[i].nscale8_video(255 / (distance + 1)); }
        break;
      }
      case PartyPattern::Comet: {
        const uint32_t head = step % count;
        const uint32_t distance = (head + count - i) % count;
        if (distance < 7) { leds[i] = selected; leds[i].nscale8_video(255 >> distance); }
        break;
      }
      case PartyPattern::Solid: leds[i] = selected; break;
      case PartyPattern::Strobe: {
        // Around 1-10 flashes per second, with a 25% on time. No blocking delay.
        const uint16_t flashPeriod = 1000 - speed * 9;
        if (state.elapsedMs % flashPeriod < flashPeriod / 4) leds[i] = selected;
        break;
      }
    }
  }
}
