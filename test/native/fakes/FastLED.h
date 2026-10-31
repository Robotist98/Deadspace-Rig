#pragma once
// Small host-only adapter for testing pixel placement, not LED wire timing.
#include <stdint.h>
#include <math.h>

inline uint8_t scale8(uint8_t value, uint8_t scale) { return value * (scale + 1U) >> 8; }
inline uint8_t scale8_video(uint8_t value, uint8_t scale) {
  return (value * scale >> 8) + (value && scale ? 1 : 0);
}
inline uint8_t sin8(uint8_t phase) {
  return static_cast<uint8_t>(127.5 + 127.5 * sin(phase * 6.283185307179586 / 256));
}
struct CRGB {
  uint8_t r, g, b;
  enum Colour : uint32_t { Black = 0x000000, Red = 0xff0000, Aqua = 0x00ffff };
  CRGB() : r(0), g(0), b(0) {}
  CRGB(uint8_t red, uint8_t green, uint8_t blue) : r(red), g(green), b(blue) {}
  CRGB(Colour colour) : r(colour >> 16), g(colour >> 8), b(colour) {}
  CRGB& nscale8_video(uint8_t scale) {
    r = scale8_video(r, scale); g = scale8_video(g, scale); b = scale8_video(b, scale);
    return *this;
  }
  bool operator==(const CRGB& other) const { return r == other.r && g == other.g && b == other.b; }
};
inline void fill_solid(CRGB* leds, uint16_t count, const CRGB& colour) {
  for (uint16_t i = 0; i < count; ++i) leds[i] = colour;
}
