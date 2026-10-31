#pragma once

#include <FastLED.h>

// Render one frame into the supplied LEDs without delays or FastLED.show().
// elapsedMs is time since the effect started. The first 1.6 seconds fill the bar.
// Health: 51-100 cyan, 26-50 amber, 1-25 pulsing red, 0 off.
void animateDeadSpaceRig(CRGB* leds, uint16_t count, uint32_t elapsedMs,
                        uint8_t healthPercent = 100);
