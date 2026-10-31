#include <Arduino.h>
#include <FastLED.h>
#include <OtaService.h>
#include <RigLighting.h>

// Copy include/config.example.h to include/config.h for a new checkout.
// Include directly: older ESP8266 toolchains do not support __has_include.
#include "config.h"
#include "led_config.h"

namespace {
constexpr unsigned long LED_FRAME_INTERVAL_MS = 20;
static_assert(LED_COUNT > 0, "LED_COUNT must be positive");
static_assert(LED_BRIGHTNESS >= 0 && LED_BRIGHTNESS <= 255,
              "LED_BRIGHTNESS must be between 0 and 255");
static_assert(RIG_HEALTH_PERCENT >= 0 && RIG_HEALTH_PERCENT <= 100,
              "RIG_HEALTH_PERCENT must be between 0 and 100");
CRGB leds[LED_COUNT];
unsigned long lastLedFrame = 0;
unsigned long rigAnimationStartedAt = 0;
OtaService ota;
const OtaService::Config OTA_CONFIG = {
    WIFI_SSID,
    WIFI_PASSWORD,
    OTA_HOSTNAME,
    OTA_PASSWORD,
    WIFI_STATIC_IP,
    WIFI_GATEWAY,
    WIFI_SUBNET,
    WIFI_DNS,
};

void configureLEDs() {
  FastLED.addLeds<LED_TYPE, LED_DATA_PIN, LED_COLOR_ORDER>(leds, LED_COUNT)
      .setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(LED_BRIGHTNESS);
  rigAnimationStartedAt = millis();
  animateDeadSpaceRig(leds, LED_COUNT, 0, RIG_HEALTH_PERCENT);
  FastLED.show();
  Serial.printf("FastLED ready: %u WS2812B LEDs on GPIO%u\n",
                static_cast<unsigned int>(LED_COUNT),
                static_cast<unsigned int>(LED_DATA_PIN));
}

void updateLEDs() {
  const unsigned long now = millis();
  if (ota.isUpdating() || now - lastLedFrame < LED_FRAME_INTERVAL_MS) {
    return;
  }

  lastLedFrame = now;
  animateDeadSpaceRig(leds, LED_COUNT, now - rigAnimationStartedAt, RIG_HEALTH_PERCENT);
  FastLED.show();
}

}  // namespace

void setup() {
  Serial.begin(115200);
  Serial.println("\nDeadspace LEDs booting");
  configureLEDs();

  ota.begin(OTA_CONFIG, []() { FastLED.clear(true); });
}

void loop() {
  ota.handle();
  updateLEDs();
  delay(1);
}
