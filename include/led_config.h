#pragma once

// Actual GPIO number on the external ESP-12E/F, not a NodeMCU D-pin number.
#define LED_DATA_PIN 5
#define LED_COUNT 24
#define LED_TYPE WS2812B
#define LED_COLOR_ORDER GRB
// 0 = off, 255 = full brightness.
#define LED_BRIGHTNESS 255

// Physical LED numbers start at 1 at the strip DIN end.
// LEDs 1-5 are the blue stasis gauge; LEDs 6-24 are the health spine.
#define RIG_STATUS_FIRST_LED 1
#define RIG_STATUS_LED_COUNT 5
#define RIG_SPINE_FIRST_LED 6
#define RIG_SPINE_LED_COUNT 19

// Reverse animation order inside each section without moving its physical LEDs.
#define RIG_STATUS_REVERSED 1
#define RIG_SPINE_REVERSED 0

// Boot health: 100 = full cyan, 50 = amber, 25 = red heartbeat, 0 = off.
// Health and brightness can be adjusted at runtime using the web controls.
#define RIG_HEALTH_PERCENT 100
