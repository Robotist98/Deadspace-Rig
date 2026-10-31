#include "RigLighting.h"

namespace {
constexpr uint32_t STARTUP_DURATION_MS = 1600;
constexpr uint16_t IDLE_PULSE_PERIOD_MS = 3200;
constexpr uint16_t CRITICAL_PULSE_PERIOD_MS = 1200;
}

void animateDeadSpaceRig(CRGB* leds, uint16_t count, uint32_t elapsedMs,
                        uint8_t healthPercent) {
  if (!leds || count == 0) {
    return;
  }
  if (healthPercent > 100) {
    healthPercent = 100;
  }
  if (healthPercent == 0) {
    fill_solid(leds, count, CRGB::Black);
    return;
  }

  const bool critical = healthPercent <= 25;
  const CRGB colour = critical ? CRGB::Red :
                      healthPercent <= 50 ? CRGB(255, 150, 0) : CRGB::Aqua;
  const uint16_t pulsePeriod = critical ? CRITICAL_PULSE_PERIOD_MS : IDLE_PULSE_PERIOD_MS;
  const uint8_t phase = static_cast<uint8_t>((elapsedMs % pulsePeriod) * 256UL / pulsePeriod);
  // Keep the healthy glow above 80% of its configured brightness.
  const uint8_t minimumGlow = critical ? 100 : 205;
  const uint8_t glow = minimumGlow + scale8(sin8(phase), 255 - minimumGlow);

  // Position in hundredths of an LED gives a soft edge when charging or damaged.
  const uint32_t targetLevel = static_cast<uint32_t>(count) * healthPercent;
  const uint32_t displayedLevel = elapsedMs < STARTUP_DURATION_MS
      ? static_cast<uint32_t>(static_cast<uint64_t>(targetLevel) * elapsedMs / STARTUP_DURATION_MS)
      : targetLevel;

  for (uint16_t index = 0; index < count; ++index) {
    const uint32_t pixelStart = static_cast<uint32_t>(index) * 100;
    if (displayedLevel <= pixelStart) {
      leds[index] = CRGB::Black;
      continue;
    }

    const uint32_t remaining = displayedLevel - pixelStart;
    const uint8_t coverage = remaining >= 100 ? 255 : remaining * 255 / 100;
    leds[index] = colour;
    leds[index].nscale8_video(scale8_video(glow, coverage));
  }
}
