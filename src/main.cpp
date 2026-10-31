#include <Arduino.h>
#include <FastLED.h>
#include <OtaService.h>
#include <RigLighting.h>
#include <RigWeb.h>

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
static_assert(RIG_STATUS_FIRST_LED >= 1 && RIG_STATUS_LED_COUNT > 0 &&
                  RIG_STATUS_FIRST_LED + RIG_STATUS_LED_COUNT - 1 <= LED_COUNT,
              "The status LEDs must fit within LED_COUNT");
static_assert(RIG_SPINE_FIRST_LED >= 1 && RIG_SPINE_LED_COUNT > 0 &&
                  RIG_SPINE_FIRST_LED + RIG_SPINE_LED_COUNT - 1 <= LED_COUNT,
              "The spine LEDs must fit within LED_COUNT");
static_assert(RIG_STATUS_FIRST_LED + RIG_STATUS_LED_COUNT <= RIG_SPINE_FIRST_LED ||
                  RIG_SPINE_FIRST_LED + RIG_SPINE_LED_COUNT <= RIG_STATUS_FIRST_LED,
              "The status and spine LEDs must not overlap");
constexpr uint16_t RIG_STATUS_LED_OFFSET = RIG_STATUS_FIRST_LED - 1;
constexpr uint16_t RIG_SPINE_LED_OFFSET = RIG_SPINE_FIRST_LED - 1;
CRGB leds[LED_COUNT];
unsigned long lastLedFrame = 0;
RigController rig;
PartyLighting party;
RigWeb web(rig, party);
const RigLayout RIG_LAYOUT = {RIG_STATUS_LED_OFFSET, RIG_STATUS_LED_COUNT,
                              RIG_SPINE_LED_OFFSET, RIG_SPINE_LED_COUNT};
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
  const unsigned long now = millis();
  rig.begin(RIG_HEALTH_PERCENT, LED_BRIGHTNESS, now);
  party.begin(now);
  renderRig(leds, LED_COUNT, RIG_LAYOUT, rig.state(now));
  reverseRigSections(leds, LED_COUNT, RIG_LAYOUT, RIG_STATUS_REVERSED, RIG_SPINE_REVERSED);
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
  const RigState state = rig.state(now);
  FastLED.setBrightness(state.brightness);
  const PartyState partyState = party.state(now);
  if (partyState.active) {
    renderParty(leds, LED_COUNT, partyState);
  } else {
    renderRig(leds, LED_COUNT, RIG_LAYOUT, state);
    reverseRigSections(leds, LED_COUNT, RIG_LAYOUT, RIG_STATUS_REVERSED, RIG_SPINE_REVERSED);
  }
  FastLED.show();
}

}  // namespace

void setup() {
  Serial.begin(115200);
  Serial.println("\nDeadspace LEDs booting");
  configureLEDs();

  ota.begin(OTA_CONFIG, []() { FastLED.clear(true); });
  web.begin(RIG_STATUS_REVERSED, RIG_SPINE_REVERSED);
}

void loop() {
  ota.handle();
  if (!ota.isUpdating()) {
    rig.update(millis());
    party.update(millis());
    web.handle();
    updateLEDs();
  }
  delay(1);
}
