#include "RigLighting.h"

namespace {
constexpr uint32_t STARTUP_MS = 1600;
constexpr uint32_t POWER_DOWN_MS = 1800;

uint8_t bump(uint16_t time, uint16_t centre, uint16_t width) {
  const uint16_t distance = time > centre ? time - centre : centre - time;
  return distance >= width ? 0 : 255UL * (width - distance) / width;
}

uint8_t heartbeat(uint32_t elapsedMs) {
  const uint16_t time = elapsedMs % 1400;
  const uint8_t first = bump(time, 120, 120);
  const uint8_t second = bump(time, 380, 120);
  return 30 + scale8(first > second ? first : second, 225);
}

uint8_t breathing(uint32_t elapsedMs) {
  const uint8_t phase = (elapsedMs % 3200) * 256UL / 3200;
  return 205 + scale8(sin8(phase), 50);
}

CRGB healthColour(uint16_t health) {
  return health <= 2500 ? CRGB::Red : health <= 5000 ? CRGB(255, 150, 0) : CRGB::Aqua;
}

uint16_t sectionCount(uint16_t count, uint16_t offset, uint16_t requested) {
  if (offset >= count) return 0;
  return requested < count - offset ? requested : count - offset;
}

void bar(CRGB* leds, uint16_t count, uint16_t level, const CRGB& colour, uint8_t glow) {
  if (level > 10000) level = 10000;
  const uint32_t total = static_cast<uint32_t>(count) * level;
  for (uint16_t i = 0; i < count; ++i) {
    const uint32_t start = static_cast<uint32_t>(i) * 10000;
    if (total <= start) {
      leds[i] = CRGB::Black;
      continue;
    }
    const uint32_t remaining = total - start;
    const uint8_t coverage = remaining >= 10000 ? 255 : remaining * 255 / 10000;
    leds[i] = colour;
    leds[i].nscale8_video(scale8_video(glow, coverage));
  }
}
}

void renderRig(CRGB* leds, uint16_t count, const RigLayout& layout, const RigState& state) {
  if (!leds || count == 0) return;
  fill_solid(leds, count, CRGB::Black);
  if (state.effect == RigEffect::Off) return;

  const uint16_t statusCount = sectionCount(count, layout.statusOffset, layout.statusCount);
  const uint16_t spineCount = sectionCount(count, layout.spineOffset, layout.spineCount);
  uint16_t health = state.health;
  uint8_t statusGlow = 255;
  const bool healing = state.effect == RigEffect::Healing;
  const CRGB colour = healing ? CRGB::Aqua : healthColour(health);
  const uint8_t glow = !healing && health <= 2500 ? heartbeat(state.elapsedMs) : breathing(state.elapsedMs);

  if (state.effect == RigEffect::Startup && state.elapsedMs < STARTUP_MS) {
    health = static_cast<uint32_t>(health) * state.elapsedMs / STARTUP_MS;
    statusGlow = state.elapsedMs >= 400 ? 255 : state.elapsedMs * 255 / 400;
  } else if (state.effect == RigEffect::PowerDown) {
    const uint32_t remaining = state.elapsedMs >= POWER_DOWN_MS ? 0 : POWER_DOWN_MS - state.elapsedMs;
    health = static_cast<uint32_t>(health) * remaining / POWER_DOWN_MS;
    statusGlow = remaining >= 450 ? 255 : remaining * 255 / 450;
  }
  // Avoid even forming an out-of-range pointer when a section is absent.
  if (statusCount) bar(leds + layout.statusOffset, statusCount, state.stasis, CRGB(0, 110, 255), statusGlow);
  if (spineCount) bar(leds + layout.spineOffset, spineCount, health, colour, glow);

  if (state.effect == RigEffect::Damage &&
      (state.elapsedMs < 90 || (state.elapsedMs >= 180 && state.elapsedMs < 270)) && spineCount) {
    fill_solid(leds + layout.spineOffset, spineCount, CRGB::Red);
  }
  if (state.effect == RigEffect::Malfunction) {
    // Deterministic noise gives broken sections without blocking or changing health.
    const uint32_t frame = state.elapsedMs / 65;
    for (uint16_t i = 0; i < count; ++i) {
      if (leds[i] == CRGB::Black) continue;
      uint32_t noise = (frame + 1) * 1664525UL + (i + 1) * 1013904223UL;
      noise ^= noise >> 13;
      if (noise % 7 < 3) leds[i] = CRGB::Black;
      else if (noise % 11 == 0) leds[i] = CRGB(180, 220, 255);
      else leds[i].nscale8_video(100 + noise % 156);
    }
  }
}

void reverseRigSections(CRGB* leds, uint16_t count, const RigLayout& layout,
                        bool reverseStatus, bool reverseSpine) {
  if (!leds || count == 0) return;
  const uint16_t offsets[] = {layout.statusOffset, layout.spineOffset};
  const uint16_t lengths[] = {layout.statusCount, layout.spineCount};
  const bool reverse[] = {reverseStatus, reverseSpine};
  for (uint8_t section = 0; section < 2; ++section) {
    if (!reverse[section]) continue;
    const uint16_t length = sectionCount(count, offsets[section], lengths[section]);
    for (uint16_t i = 0; i < length / 2; ++i) {
      const uint16_t first = offsets[section] + i;
      const uint16_t last = offsets[section] + length - 1 - i;
      const CRGB saved = leds[first];
      leds[first] = leds[last];
      leds[last] = saved;
    }
  }
}

void animateDeadSpaceRig(CRGB* leds, uint16_t count, uint32_t elapsedMs, uint8_t healthPercent) {
  if (!leds || count == 0) return;
  uint16_t health = (healthPercent > 100 ? 100 : healthPercent) * 100U;
  const CRGB colour = healthColour(health);
  if (elapsedMs < STARTUP_MS) health = static_cast<uint32_t>(health) * elapsedMs / STARTUP_MS;
  bar(leds, count, health, colour, healthPercent <= 25 ? heartbeat(elapsedMs) : breathing(elapsedMs));
}
