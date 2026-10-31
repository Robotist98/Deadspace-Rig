#include "RigLighting.h"
#include "PartyLighting.h"
#include <cassert>
#include <cstring>
#include <iostream>

void transitions() {
  RigController rig;
  rig.begin(100, 64, 0);
  rig.update(1600);
  assert(rig.state(1600).effect == RigEffect::Idle);
  rig.trigger(RigCommand::Damage, 2000);
  assert(rig.state(2000).health == 7500);
  rig.update(2450);
  assert(rig.state(2450).effect == RigEffect::Idle);
  rig.trigger(RigCommand::Critical, 3000);
  assert(std::strcmp(rigEffectName(rig.state(3000)), "critical") == 0);
  rig.trigger(RigCommand::Heal, 3000);
  rig.update(4100);
  assert(rig.state(4100).health == 6000);
  rig.update(5200);
  assert(rig.state(5200).health == 10000);
  assert(rig.state(5200).effect == RigEffect::Idle);
  // Interrupt a heal; the next command starts at the current interpolated level.
  rig.setHealth(20, 6000);
  rig.trigger(RigCommand::Heal, 6000);
  rig.trigger(RigCommand::Damage, 7100);
  assert(rig.state(7100).health == 3500);
  rig.trigger(RigCommand::DrainStasis, 8000);
  rig.update(8900);
  assert(rig.state(8900).stasis == 5000);
  assert(rig.state(8900).health == 3500);
  rig.trigger(RigCommand::RechargeStasis, 8900);
  rig.update(10700);
  assert(rig.state(10700).stasis == 10000);
  rig.trigger(RigCommand::Malfunction, 11000);
  rig.update(13400);
  assert(rig.state(13400).health == 3500);
  assert(rig.state(13400).stasis == 10000);
  rig.trigger(RigCommand::PowerDown, 14000);
  rig.update(15800);
  rig.update(20000);
  assert(rig.state(20000).effect == RigEffect::Off);
  rig.trigger(RigCommand::PowerUp, 20000);
  rig.update(21600);
  assert(rig.state(21600).effect == RigEffect::Idle);
  assert(rig.state(21600).health == 3500);
  rig.setHealth(5, 22000);
  rig.trigger(RigCommand::Damage, 22000);
  assert(rig.state(22000).health == 0); // Saturation, never unsigned underflow.
}

void demoAndRollover() {
  RigController rig;
  rig.begin(100, 255, 0);
  rig.setDemo(true, 0);
  const uint16_t durations[] = {2600, 2100, 2100, 3600, 3000, 2400, 2400, 3000, 2400};
  uint32_t now = 0;
  for (unsigned cycle = 0; cycle < 2; ++cycle) {
    for (uint16_t duration : durations) {
      // Model real loop updates rather than just skipping directly to the next step.
      const uint32_t end = now + duration;
      while (now < end) { now += 20; rig.update(now); }
      assert(rig.state(now).demo);
    }
    assert(rig.state(now).effect == RigEffect::Startup);
    assert(rig.state(now).health == 10000 && rig.state(now).stasis == 10000);
  }
  rig.setBrightness(80);
  assert(rig.state(now).demo);
  rig.setHealth(42, now);
  assert(!rig.state(now).demo);
  rig.update(now + 10000);
  assert(rig.state(now + 10000).health == 4200);
  // Unsigned millis subtraction must survive the ESP timer rolling over.
  const uint32_t start = UINT32_MAX - 500;
  rig.begin(20, 64, start);
  rig.trigger(RigCommand::Heal, start);
  rig.trigger(RigCommand::DrainStasis, start);
  rig.update(start + 1100U);
  assert(rig.state(start + 1100U).health == 6000);
  rig.update(start + 2200U);
  assert(rig.state(start + 2200U).health == 10000);
  assert(rig.state(start + 2200U).stasis == 0);
}

void pixelBoundaries() {
  CRGB pixels[26];
  const CRGB guard(77, 88, 99);
  pixels[0] = pixels[25] = guard;
  const RigLayout layout = {0, 5, 5, 19};
  RigState state = {0, 10000, 255, RigEffect::Idle, 1000, false};
  renderRig(pixels + 1, 24, layout, state);
  for (unsigned i = 1; i <= 5; ++i) assert(pixels[i].b > 0);
  for (unsigned i = 6; i <= 24; ++i) assert(pixels[i] == CRGB::Black);
  state.health = 10000;
  state.stasis = 0;
  renderRig(pixels + 1, 24, layout, state);
  for (unsigned i = 1; i <= 5; ++i) assert(pixels[i] == CRGB::Black);
  for (unsigned i = 6; i <= 24; ++i) assert(pixels[i].g > 0 && pixels[i].b > 0);
  state.stasis = 10000;
  state.effect = RigEffect::Damage;
  state.elapsedMs = 0;
  renderRig(pixels + 1, 24, layout, state);
  assert(pixels[1].b > 0 && pixels[1].r == 0);
  for (unsigned i = 6; i <= 24; ++i) assert(pixels[i] == CRGB::Red);
  state.effect = RigEffect::Off;
  renderRig(pixels + 1, 24, layout, state);
  for (unsigned i = 1; i <= 24; ++i) assert(pixels[i] == CRGB::Black);
  state.effect = RigEffect::PowerDown;
  state.elapsedMs = 1800;
  renderRig(pixels + 1, 24, layout, state);
  for (unsigned i = 1; i <= 24; ++i) assert(pixels[i] == CRGB::Black);
  state.effect = RigEffect::Idle;
  state.health = 2000;
  state.elapsedMs = 120;
  renderRig(pixels + 1, 24, layout, state);
  const uint8_t firstBeat = pixels[6].r;
  state.elapsedMs = 380;
  renderRig(pixels + 1, 24, layout, state);
  assert(pixels[6].r == firstBeat);
  state.elapsedMs = 800;
  renderRig(pixels + 1, 24, layout, state);
  assert(pixels[6].r < firstBeat);
  // Truncated or absent sections still respect the supplied buffer.
  renderRig(pixels + 1, 24, {0, 5, 23, 19}, state);
  renderRig(pixels + 1, 24, {30, 5, 500, 19}, state);
  renderRig(nullptr, 24, layout, state);
  assert(pixels[0] == guard && pixels[25] == guard);
}

void reversedSections() {
  const RigLayout layout = {0, 5, 5, 19};
  const RigState state = {2000, 2000, 255, RigEffect::Idle, 120, false};
  const CRGB guard(77, 88, 99);
  CRGB original[24], pixels[26];
  pixels[0] = pixels[25] = guard;
  renderRig(original, 24, layout, state);
  renderRig(pixels + 1, 24, layout, state);
  reverseRigSections(pixels + 1, 24, layout, true, true);
  for (unsigned i = 0; i < 5; ++i) assert(pixels[i + 1] == original[4 - i]);
  for (unsigned i = 5; i < 24; ++i) assert(pixels[i + 1] == original[28 - i]);
  assert(pixels[1] == CRGB::Black && pixels[5].b > 0);
  assert(pixels[6] == CRGB::Black && pixels[24].r > 0);
  reverseRigSections(pixels + 1, 24, layout, true, false);
  for (unsigned i = 0; i < 5; ++i) assert(pixels[i + 1] == original[i]);
  renderRig(pixels + 1, 24, layout, state);
  reverseRigSections(pixels + 1, 24, layout, true, false);
  for (unsigned i = 0; i < 5; ++i) assert(pixels[i + 1] == original[4 - i]);
  for (unsigned i = 5; i < 24; ++i) assert(pixels[i + 1] == original[i]);
  reverseRigSections(pixels + 1, 24, {30, 5, 23, 19}, true, true);
  reverseRigSections(nullptr, 24, layout, true, true);
  assert(pixels[0] == guard && pixels[25] == guard);
}

void partyEffects() {
  PartyLighting party;
  party.begin(0);
  assert(!party.state(0).active);
  PartyPattern parsed = PartyPattern::Solid;
  assert(!parsePartyPattern("invalid", parsed) && parsed == PartyPattern::Solid);
  assert(!parsePartyPattern(nullptr, parsed));
  assert(parsePartyPattern("comet", parsed) && parsed == PartyPattern::Comet);
  party.setSpeed(0); assert(party.state(0).speed == 1);
  party.setSpeed(255); assert(party.state(0).speed == 100);
  party.setColour(0xff123456); assert(party.state(0).colour == 0x123456);
  party.select(PartyPattern::Rainbow, 0);
  party.setCycle(true, 0);
  party.update(9999); assert(party.state(9999).pattern == PartyPattern::Rainbow);
  party.update(10000); assert(party.state(10000).pattern == PartyPattern::Wave);
  party.update(80000); assert(party.state(80000).pattern == PartyPattern::Strobe);
  party.update(90000); assert(party.state(90000).pattern == PartyPattern::Rainbow);
  party.select(PartyPattern::Solid, 91000);
  assert(!party.state(91000).cycle);
  party.stop(); assert(!party.state(91000).active && !party.state(91000).cycle);
  const uint32_t start = UINT32_MAX - 1000;
  party.begin(start); party.start(start); party.setCycle(true, start);
  party.update(start + 10000U);
  assert(party.state(start + 10000U).pattern == PartyPattern::Wave);

  CRGB pixels[26];
  const CRGB guard(77, 88, 99);
  pixels[0] = pixels[25] = guard;
  const char* names[] = {"rainbow","wave","chase","sparkle","breathe","scanner","comet","solid","strobe"};
  for (const char* name : names) {
    assert(parsePartyPattern(name, parsed));
    party.select(parsed, 0);
    party.setColour(0x123456);
    for (uint32_t time : {0U, 500U, 10000U, UINT32_MAX}) {
      renderParty(pixels + 1, 24, party.state(time));
      bool lit = false;
      for (unsigned i = 1; i <= 24; ++i) lit |= !(pixels[i] == CRGB::Black);
      assert(lit || parsed == PartyPattern::Strobe);
      assert(pixels[0] == guard && pixels[25] == guard);
    }
    renderParty(pixels + 1, 1, party.state(1234));
    renderParty(nullptr, 24, party.state(0));
    renderParty(pixels + 1, 0, party.state(0));
  }
  party.select(PartyPattern::Solid, 0);
  renderParty(pixels + 1, 24, party.state(0));
  for (unsigned i = 1; i <= 24; ++i) assert(pixels[i] == CRGB(0x12, 0x34, 0x56));
  party.select(PartyPattern::Strobe, 0);
  party.setSpeed(100);
  renderParty(pixels + 1, 24, party.state(0));
  for (unsigned i = 1; i <= 24; ++i) assert(pixels[i] == CRGB(0x12, 0x34, 0x56));
  renderParty(pixels + 1, 24, party.state(50));
  for (unsigned i = 1; i <= 24; ++i) assert(pixels[i] == CRGB::Black);
  renderParty(pixels + 1, 24, party.state(100));
  for (unsigned i = 1; i <= 24; ++i) assert(pixels[i] == CRGB(0x12, 0x34, 0x56));
  party.setSpeed(1);
  renderParty(pixels + 1, 24, party.state(100));
  for (unsigned i = 1; i <= 24; ++i) assert(pixels[i] == CRGB(0x12, 0x34, 0x56));
  party.stop();
  renderParty(pixels + 1, 24, party.state(0));
  for (unsigned i = 1; i <= 24; ++i) assert(pixels[i] == CRGB::Black);
}

int main() {
  transitions(); demoAndRollover(); pixelBoundaries(); reversedSections(); partyEffects();
  std::cout << "PASS: RIG transitions, reversed sections, party effects, playlist and LED boundaries\n";
}
