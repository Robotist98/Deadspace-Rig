#pragma once

#include <FastLED.h>
#include "RigController.h"

struct RigLayout {
  uint16_t statusOffset;
  uint16_t statusCount;
  uint16_t spineOffset;
  uint16_t spineCount;
};

// Renders both sections, leaving unused LEDs black. Offsets are zero based.
void renderRig(CRGB* leds, uint16_t count, const RigLayout& layout, const RigState& state);

// Reverse pixels only inside their original sections, retaining physical positions.
void reverseRigSections(CRGB* leds, uint16_t count, const RigLayout& layout,
                        bool reverseStatus, bool reverseSpine);

// Render one frame into the supplied LEDs without delays or FastLED.show().
// elapsedMs is time since the effect started. The first 1.6 seconds fill the bar.
// Health: 51-100 cyan, 26-50 amber, 1-25 red double heartbeat, 0 off.
void animateDeadSpaceRig(CRGB* leds, uint16_t count, uint32_t elapsedMs,
                        uint8_t healthPercent = 100);
