#include "RigController.h"

namespace {
constexpr uint32_t STARTUP_MS = 1600;
constexpr uint32_t DAMAGE_MS = 450;
constexpr uint32_t HEALING_MS = 2200;
constexpr uint32_t MALFUNCTION_MS = 2400;
constexpr uint32_t POWER_DOWN_MS = 1800;
constexpr uint32_t STASIS_MS = 1800;
const uint16_t DEMO_DURATIONS[] = {2600, 2100, 2100, 3600, 3000, 2400, 2400, 3000, 2400};
constexpr uint8_t DEMO_STEPS = sizeof(DEMO_DURATIONS) / sizeof(DEMO_DURATIONS[0]);

uint16_t level(uint8_t percent) {
  return (percent > 100 ? 100 : percent) * 100U;
}

uint16_t interpolate(uint16_t from, uint16_t to, uint32_t elapsed, uint32_t duration) {
  if (elapsed >= duration) return to;
  return from + (static_cast<int32_t>(to) - from) * static_cast<int32_t>(elapsed) /
                    static_cast<int32_t>(duration);
}
}

void RigController::begin(uint8_t health, uint8_t brightness, uint32_t now) {
  health_ = level(health);
  stasis_ = stasisFrom_ = stasisTarget_ = 10000;
  brightness_ = brightness;
  effect_ = RigEffect::Startup;
  effectStarted_ = stasisStarted_ = now;
  demo_ = false;
}

void RigController::update(uint32_t now) {
  const uint32_t elapsed = now - effectStarted_;
  stasis_ = interpolate(stasisFrom_, stasisTarget_, now - stasisStarted_, STASIS_MS);
  if (now - stasisStarted_ >= STASIS_MS) stasisFrom_ = stasisTarget_;
  bool complete = false;
  switch (effect_) {
    case RigEffect::Startup: complete = elapsed >= STARTUP_MS; break;
    case RigEffect::Damage: complete = elapsed >= DAMAGE_MS; break;
    case RigEffect::Healing:
      health_ = interpolate(healingFrom_, 10000, elapsed, HEALING_MS);
      complete = elapsed >= HEALING_MS;
      break;
    case RigEffect::Malfunction: complete = elapsed >= MALFUNCTION_MS; break;
    case RigEffect::PowerDown:
      if (elapsed >= POWER_DOWN_MS) effect_ = RigEffect::Off;
      break;
    default: break;
  }
  if (complete) {
    effect_ = RigEffect::Idle;
    effectStarted_ = now;
  }
  if (demo_ && now - demoStepStarted_ >= DEMO_DURATIONS[demoStep_]) {
    demoStep_ = (demoStep_ + 1) % DEMO_STEPS;
    startDemoStep(now);
  }
}

void RigController::trigger(RigCommand command, uint32_t now) {
  update(now);
  demo_ = false;
  startCommand(command, now);
}

void RigController::startCommand(RigCommand command, uint32_t now) {
  if (command == RigCommand::DrainStasis || command == RigCommand::RechargeStasis) {
    changeStasis(command == RigCommand::DrainStasis ? 0 : 10000, now);
    return;
  }
  effectStarted_ = now;
  switch (command) {
    case RigCommand::PowerUp: effect_ = RigEffect::Startup; break;
    case RigCommand::Damage:
      health_ = health_ > 2500 ? health_ - 2500 : 0;
      effect_ = RigEffect::Damage;
      break;
    case RigCommand::Heal:
      healingFrom_ = health_;
      effect_ = RigEffect::Healing;
      break;
    case RigCommand::Critical:
      health_ = 2000;
      effect_ = RigEffect::Idle;
      break;
    case RigCommand::Malfunction: effect_ = RigEffect::Malfunction; break;
    case RigCommand::PowerDown: effect_ = RigEffect::PowerDown; break;
    default: break;
  }
}

void RigController::setHealth(uint8_t percent, uint32_t now) {
  update(now);
  demo_ = false;
  health_ = level(percent);
  effect_ = RigEffect::Idle;
  effectStarted_ = now;
}

void RigController::changeStasis(uint16_t target, uint32_t now) {
  stasisFrom_ = stasis_;
  stasisTarget_ = target;
  stasisStarted_ = now;
}

void RigController::setStasis(uint8_t percent, uint32_t now) {
  update(now);
  demo_ = false;
  changeStasis(level(percent), now);
}

void RigController::setBrightness(uint8_t brightness) {
  brightness_ = brightness;
}

void RigController::setDemo(bool enabled, uint32_t now) {
  update(now);
  demo_ = enabled;
  if (enabled) {
    demoStep_ = 0;
    startDemoStep(now);
  }
}

void RigController::startDemoStep(uint32_t now) {
  demoStepStarted_ = now;
  switch (demoStep_) {
    case 0:
      health_ = stasis_ = stasisFrom_ = stasisTarget_ = 10000;
      stasisStarted_ = now;
      startCommand(RigCommand::PowerUp, now);
      break;
    case 1:
    case 2: startCommand(RigCommand::Damage, now); break;
    case 3: startCommand(RigCommand::Critical, now); break;
    case 4: startCommand(RigCommand::Heal, now); break;
    case 5: startCommand(RigCommand::DrainStasis, now); break;
    case 6: startCommand(RigCommand::RechargeStasis, now); break;
    case 7: startCommand(RigCommand::Malfunction, now); break;
    case 8: startCommand(RigCommand::PowerDown, now); break;
  }
}

RigState RigController::state(uint32_t now) const {
  return {health_, stasis_, brightness_, effect_, now - effectStarted_, demo_};
}

const char* rigEffectName(const RigState& state) {
  switch (state.effect) {
    case RigEffect::Startup: return "startup";
    case RigEffect::Damage: return "damage";
    case RigEffect::Healing: return "healing";
    case RigEffect::Malfunction: return "malfunction";
    case RigEffect::PowerDown: return "power-down";
    case RigEffect::Off: return "off";
    default: return state.health > 0 && state.health <= 2500 ? "critical" : "idle";
  }
}
