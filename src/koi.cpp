#include "koi.hpp"

namespace koi {

void Koi::Reset(int index, Random &random) {
  position = {random.Range(45.0F, kCanvasWidth - 45.0F),
              random.Range(32.0F, kCanvasHeight - 32.0F)};
  heading = random.Range(-3.14159F, 3.14159F);
  cruiseSpeed = random.Range(13.0F, 21.0F);
  maximumSpeed = cruiseSpeed * random.Range(1.55F, 1.9F);
  speed = cruiseSpeed * random.Range(0.72F, 1.05F);
  turnStrength = random.Range(4.4F, 6.8F);
  bodyLength = random.Range(27.0F, 38.0F);
  bodyWidth = bodyLength * random.Range(0.17F, 0.2F);
  phaseOffset = random.Range(0.0F, kTau);
  swimPhase = phaseOffset;
  wanderSeed = random.Range(0.0F, 100.0F);
  reactivity = random.Range(0.35F, 1.0F);
  callDelay = 0.0F;
  behaviorRng = 0x9E3779B9U ^
                (static_cast<std::uint32_t>(index) + 1U) * 0x85EBCA6BU;
  state = static_cast<SwimState>(index % 5);

  switch (state) {
    case SwimState::Glide:
      stateDuration = random.Range(1.7F, 4.8F);
      break;
    case SwimState::Coast:
      stateDuration = random.Range(0.8F, 2.0F);
      break;
    case SwimState::Hover:
      stateDuration = random.Range(0.7F, 2.9F);
      break;
    case SwimState::Burst:
      stateDuration = random.Range(0.35F, 0.9F);
      break;
    case SwimState::Pivot:
      stateDuration = random.Range(0.35F, 0.8F);
      break;
  }

  stateAge = random.Range(0.0F, stateDuration * 0.8F);
  pivotHeading = heading;
  tailEffort = 0.6F;
  angularVelocity = 0.0F;
  velocity = Mul(FromAngle(heading), speed);

  const Vector2 backward =
      Mul(FromAngle(heading), -bodyLength / (kSpineNodes - 1));
  for (int node = 0; node < kSpineNodes; ++node) {
    spine[node] = Add(position, Mul(backward, static_cast<float>(node)));
    renderSpine[node] = spine[node];
  }
}

}  // namespace koi
