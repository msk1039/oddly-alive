#pragma once

#include <raylib.h>

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace koi {

inline float Length(Vector2 v) { return std::sqrt(v.x * v.x + v.y * v.y); }
inline Vector2 Add(Vector2 a, Vector2 b) { return {a.x + b.x, a.y + b.y}; }
inline Vector2 Sub(Vector2 a, Vector2 b) { return {a.x - b.x, a.y - b.y}; }
inline Vector2 Mul(Vector2 v, float scalar) {
  return {v.x * scalar, v.y * scalar};
}
inline Vector2 Lerp(Vector2 a, Vector2 b, float amount) {
  return Add(a, Mul(Sub(b, a), amount));
}
inline Vector2 Normalize(Vector2 v, Vector2 fallback = {1.0F, 0.0F}) {
  const float length = Length(v);
  return length > 0.0001F ? Mul(v, 1.0F / length) : fallback;
}
inline Vector2 FromAngle(float angle) {
  return {std::cos(angle), std::sin(angle)};
}
inline Vector2 Perpendicular(Vector2 v) { return {-v.y, v.x}; }
inline float Clamp(float value, float low, float high) {
  return std::max(low, std::min(value, high));
}
inline float WrapAngle(float angle) {
  return std::atan2(std::sin(angle), std::cos(angle));
}

struct Random {
  std::uint32_t state = 0xC0FFEEU;

  std::uint32_t Next() {
    state ^= state << 13U;
    state ^= state >> 17U;
    state ^= state << 5U;
    return state;
  }

  float Unit() {
    return static_cast<float>(Next() & 0x00FFFFFFU) /
           static_cast<float>(0x01000000U);
  }

  float Range(float low, float high) { return low + (high - low) * Unit(); }
};

}  // namespace koi
