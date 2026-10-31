#pragma once

// Actual GPIO number on the external ESP-12E/F, not a NodeMCU D-pin number.
#define LED_DATA_PIN 5
#define LED_COUNT 30
#define LED_TYPE WS2812B
#define LED_COLOR_ORDER GRB
// 0 = off, 255 = full brightness. Start at roughly 25% brightness.
#define LED_BRIGHTNESS 200

// RIG health bar: 100 = full cyan, 50 = amber, 25 = pulsing red, 0 = off.
#define RIG_HEALTH_PERCENT 50
