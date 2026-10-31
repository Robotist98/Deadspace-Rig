#pragma once

#include <stdint.h>

enum class RigEffect : uint8_t { Startup, Idle, Damage, Healing, Malfunction, PowerDown, Off };
enum class RigCommand : uint8_t {
  PowerUp, Damage, Heal, Critical, Malfunction, PowerDown, DrainStasis, RechargeStasis
};

struct RigState {
  // Hundredths of a percent allow smooth fills between physical LEDs.
  uint16_t health;
  uint16_t stasis;
  uint8_t brightness;
  RigEffect effect;
  uint32_t elapsedMs;
  bool demo;
};

// Time and commands are supplied by the caller; no delays, networking or heap use.
class RigController {
 public:
  void begin(uint8_t health, uint8_t brightness, uint32_t now);
  void update(uint32_t now);
  void trigger(RigCommand command, uint32_t now);
  void setHealth(uint8_t percent, uint32_t now);
  void setStasis(uint8_t percent, uint32_t now);
  void setBrightness(uint8_t brightness);
  void setDemo(bool enabled, uint32_t now);
  RigState state(uint32_t now) const;

 private:
  void startCommand(RigCommand command, uint32_t now);
  void startDemoStep(uint32_t now);
  void changeStasis(uint16_t target, uint32_t now);

  uint16_t health_ = 10000;
  uint16_t stasis_ = 10000;
  uint16_t healingFrom_ = 10000;
  uint16_t stasisFrom_ = 10000;
  uint16_t stasisTarget_ = 10000;
  uint8_t brightness_ = 255;
  RigEffect effect_ = RigEffect::Startup;
  uint32_t effectStarted_ = 0;
  uint32_t stasisStarted_ = 0;
  uint32_t demoStepStarted_ = 0;
  uint8_t demoStep_ = 0;
  bool demo_ = false;
};

const char* rigEffectName(const RigState& state);
