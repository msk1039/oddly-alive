#pragma once

#include "config.hpp"
#include "math_utils.hpp"

#include <raylib.h>

#include <array>
#include <cstdint>

namespace koi {

enum class SwimState : std::uint8_t {
  Glide,
  Coast,
  Hover,
  Burst,
  Pivot,
};

struct Koi {
  Vector2 position{};
  Vector2 velocity{};
  std::array<Vector2, kSpineNodes> spine{};
  std::array<Vector2, kSpineNodes> renderSpine{};

  float heading = 0.0F;
  float angularVelocity = 0.0F;
  float speed = 0.0F;
  float cruiseSpeed = 20.0F;
  float maximumSpeed = 34.0F;
  float turnStrength = 5.0F;
  float bodyLength = 24.0F;
  float bodyWidth = 4.0F;
  float swimPhase = 0.0F;
  float phaseOffset = 0.0F;
  float wanderSeed = 0.0F;
  float stateAge = 0.0F;
  float stateDuration = 2.0F;
  float pivotHeading = 0.0F;
  float reactivity = 0.7F;
  float callDelay = 0.0F;
  float tailEffort = 0.6F;
  std::uint32_t behaviorRng = 1U;
  SwimState state = SwimState::Glide;

  void Reset(int index, Random &random);
};

}  // namespace koi
