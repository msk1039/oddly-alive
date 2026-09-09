#include "school.hpp"

#include "math_utils.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

namespace koi {

School::School() {
  for (int i = 0; i < kMaxFish; ++i) fish_[i].Reset(i, random_);
}

void School::SetCount(int count) {
  count_ = std::clamp(count, 1, kMaxFish);
}

int School::Count() const { return count_; }

void School::Reset() {
  random_.state = 0xC0FFEEU;
  for (int i = 0; i < kMaxFish; ++i) fish_[i].Reset(i, random_);
  targetActive_ = false;
}

void School::CallTo(Vector2 point) {
  target_ = point;
  targetActive_ = true;
  targetAge_ = 0.0F;
  for (int i = 0; i < count_; ++i) {
    fish_[i].callDelay =
        random_.Range(0.04F, 1.15F) * (1.22F - fish_[i].reactivity);
  }
  AddRipple(point);
}

void School::Scatter() {
  for (int i = 0; i < count_; ++i) {
    fish_[i].heading += random_.Range(-1.35F, 1.35F);
    fish_[i].speed = fish_[i].maximumSpeed;
    fish_[i].angularVelocity += random_.Range(-2.0F, 2.0F);
    EnterState(fish_[i], SwimState::Burst);
  }
  targetActive_ = false;
}

void School::Update(float dt, float time) {
  targetAge_ += dt;
  if (targetActive_ && targetAge_ > 7.5F) targetActive_ = false;

  std::array<Vector2, kMaxFish> desired{};
  std::array<float, kMaxFish> desiredSpeed{};

  for (int i = 0; i < count_; ++i) {
    Koi &koi = fish_[i];
    koi.callDelay = std::max(0.0F, koi.callDelay - dt);
    UpdateNaturalState(koi, dt);
    desired[i] = SteeringFor(i, time);
    desiredSpeed[i] = DesiredSpeedFor(i);
  }
  for (int i = 0; i < count_; ++i) {
    Integrate(fish_[i], desired[i], desiredSpeed[i], dt);
  }

  for (Ripple &ripple : ripples_) {
    if (!ripple.alive) continue;
    ripple.age += dt;
    if (ripple.age > 1.25F) ripple.alive = false;
  }
}

bool School::TargetActive() const { return targetActive_; }

void School::LogBehaviorSummary() const {
  std::array<int, 5> counts{};
  for (int i = 0; i < count_; ++i) {
    ++counts[static_cast<std::size_t>(fish_[i].state)];
  }
  TraceLog(LOG_INFO,
           "BEHAVIOR: glide=%i coast=%i hover=%i burst=%i pivot=%i",
           counts[0], counts[1], counts[2], counts[3], counts[4]);
}

Koi &School::Fish(int index) { return fish_[index]; }
const Koi &School::Fish(int index) const { return fish_[index]; }

const std::array<Ripple, kMaxRipples> &School::Ripples() const {
  return ripples_;
}

float School::BehaviorUnit(Koi &koi) {
  koi.behaviorRng ^= koi.behaviorRng << 13U;
  koi.behaviorRng ^= koi.behaviorRng >> 17U;
  koi.behaviorRng ^= koi.behaviorRng << 5U;
  return static_cast<float>(koi.behaviorRng & 0x00FFFFFFU) /
         static_cast<float>(0x01000000U);
}

float School::BehaviorRange(Koi &koi, float low, float high) {
  return low + (high - low) * BehaviorUnit(koi);
}

void School::EnterState(Koi &koi, SwimState next) {
  koi.state = next;
  koi.stateAge = 0.0F;
  switch (next) {
    case SwimState::Glide:
      koi.stateDuration = BehaviorRange(koi, 1.7F, 5.2F);
      break;
    case SwimState::Coast:
      koi.stateDuration = BehaviorRange(koi, 0.7F, 2.1F);
      break;
    case SwimState::Hover:
      koi.stateDuration = BehaviorRange(koi, 0.65F, 3.1F);
      break;
    case SwimState::Burst:
      koi.stateDuration = BehaviorRange(koi, 0.32F, 0.92F);
      break;
    case SwimState::Pivot: {
      koi.stateDuration = BehaviorRange(koi, 0.3F, 0.78F);
      const float direction = BehaviorUnit(koi) < 0.5F ? -1.0F : 1.0F;
      koi.pivotHeading = WrapAngle(
          koi.heading + direction * BehaviorRange(koi, 0.85F, 2.35F));
      break;
    }
  }
}

void School::UpdateNaturalState(Koi &koi, float dt) {
  koi.stateAge += dt;
  if (koi.stateAge < koi.stateDuration) return;

  const float roll = BehaviorUnit(koi);
  switch (koi.state) {
    case SwimState::Glide:
      if (roll < 0.25F)
        EnterState(koi, SwimState::Coast);
      else if (roll < 0.43F)
        EnterState(koi, SwimState::Hover);
      else if (roll < 0.61F)
        EnterState(koi, SwimState::Pivot);
      else if (roll < 0.72F)
        EnterState(koi, SwimState::Burst);
      else
        EnterState(koi, SwimState::Glide);
      break;
    case SwimState::Coast:
      if (roll < 0.38F)
        EnterState(koi, SwimState::Hover);
      else if (roll < 0.72F)
        EnterState(koi, SwimState::Glide);
      else if (roll < 0.9F)
        EnterState(koi, SwimState::Pivot);
      else
        EnterState(koi, SwimState::Burst);
      break;
    case SwimState::Hover:
      if (roll < 0.34F)
        EnterState(koi, SwimState::Pivot);
      else if (roll < 0.55F)
        EnterState(koi, SwimState::Burst);
      else
        EnterState(koi, SwimState::Glide);
      break;
    case SwimState::Burst:
      EnterState(koi, SwimState::Coast);
      break;
    case SwimState::Pivot:
      EnterState(koi,
                 roll < 0.38F ? SwimState::Burst : SwimState::Glide);
      break;
  }
}

Vector2 School::SteeringFor(int index, float time) const {
  const Koi &koi = fish_[index];
  const Vector2 forward = FromAngle(koi.heading);
  Vector2 steering = Mul(forward, 0.95F);

  if (koi.state == SwimState::Pivot) {
    steering = Mul(FromAngle(koi.pivotHeading), 4.7F);
  } else if (koi.state != SwimState::Hover) {
    const float wander =
        std::sin(time * 0.29F + koi.wanderSeed) * 0.7F +
        std::sin(time * 0.113F + koi.wanderSeed * 1.73F) * 0.45F;
    steering = Add(steering,
                   Mul(FromAngle(koi.heading + wander), 0.62F));
  }

  Vector2 separation{};
  Vector2 alignment{};
  Vector2 cohesion{};
  int neighbours = 0;

  for (int other = 0; other < count_; ++other) {
    if (other == index) continue;
    const Vector2 offset = Sub(koi.position, fish_[other].position);
    const float distance = Length(offset);
    if (distance > 0.001F && distance < 37.0F) {
      ++neighbours;
      cohesion = Add(cohesion, fish_[other].position);
      alignment = Add(alignment, Normalize(fish_[other].velocity));
      if (distance < 14.0F) {
        separation = Add(separation,
                         Mul(Normalize(offset), (14.0F - distance) / 14.0F));
      }
    }
  }

  if (neighbours > 0) {
    cohesion = Mul(cohesion, 1.0F / static_cast<float>(neighbours));
    cohesion = Normalize(Sub(cohesion, koi.position), forward);
    alignment = Normalize(alignment, forward);
    steering = Add(steering, Mul(cohesion, 0.25F));
    steering = Add(steering, Mul(alignment, 0.42F));
    steering = Add(steering, Mul(separation, 2.8F));
  }

  constexpr float margin = 32.0F;
  Vector2 edgeForce{};
  if (koi.position.x < margin)
    edgeForce.x += (margin - koi.position.x) / margin;
  if (koi.position.x > kCanvasWidth - margin)
    edgeForce.x -= (koi.position.x - (kCanvasWidth - margin)) / margin;
  if (koi.position.y < margin)
    edgeForce.y += (margin - koi.position.y) / margin;
  if (koi.position.y > kCanvasHeight - margin)
    edgeForce.y -= (koi.position.y - (kCanvasHeight - margin)) / margin;
  steering = Add(steering, Mul(edgeForce, 4.8F));

  if (targetActive_ && koi.callDelay <= 0.0F &&
      koi.state != SwimState::Hover) {
    const Vector2 toTarget = Sub(target_, koi.position);
    const float distance = Length(toTarget);
    if (distance > 13.0F) {
      steering = Add(steering, Mul(Normalize(toTarget), 2.45F));
    } else {
      const Vector2 tangent = Perpendicular(Normalize(toTarget, forward));
      steering = Add(steering, Mul(tangent, 2.2F));
      steering = Add(steering, Mul(Normalize(toTarget, forward), -0.5F));
    }
  }

  return Normalize(steering, forward);
}

float School::DesiredSpeedFor(int index) const {
  const Koi &koi = fish_[index];
  float intention = koi.cruiseSpeed;
  if (targetActive_ && koi.callDelay <= 0.0F) {
    const float distance = Length(Sub(target_, koi.position));
    const float urgency = Clamp(distance / 105.0F, 0.2F, 1.0F);
    intention = koi.cruiseSpeed +
                (koi.maximumSpeed - koi.cruiseSpeed) * urgency;
  }

  switch (koi.state) {
    case SwimState::Glide:
      return intention;
    case SwimState::Coast:
      return intention * 0.28F;
    case SwimState::Hover:
      return 0.0F;
    case SwimState::Burst:
      return koi.maximumSpeed * 1.08F;
    case SwimState::Pivot:
      return koi.cruiseSpeed * 0.16F;
  }
  return intention;
}

void School::Integrate(Koi &koi, Vector2 desired, float desiredSpeed,
                       float dt) {
  const float desiredHeading = std::atan2(desired.y, desired.x);
  const float headingError = WrapAngle(desiredHeading - koi.heading);
  const bool pivoting = koi.state == SwimState::Pivot;
  const float turnMultiplier = pivoting ? 2.65F : 1.0F;
  const float angularDamping = pivoting ? 2.15F : 3.8F;
  const float angularAcceleration =
      headingError * koi.turnStrength * turnMultiplier -
      koi.angularVelocity * angularDamping;
  koi.angularVelocity += angularAcceleration * dt;
  const float maximumTurnRate = pivoting ? 4.35F : 2.25F;
  koi.angularVelocity =
      Clamp(koi.angularVelocity, -maximumTurnRate, maximumTurnRate);
  koi.heading = WrapAngle(koi.heading + koi.angularVelocity * dt);

  float speedResponse = 1.65F;
  float desiredTailEffort = 0.62F;
  switch (koi.state) {
    case SwimState::Glide:
      break;
    case SwimState::Coast:
      speedResponse = 1.05F;
      desiredTailEffort = 0.16F;
      break;
    case SwimState::Hover:
      speedResponse = 3.6F;
      desiredTailEffort = 0.05F;
      break;
    case SwimState::Burst:
      speedResponse = 6.4F;
      desiredTailEffort = 1.22F;
      break;
    case SwimState::Pivot:
      speedResponse = 4.2F;
      desiredTailEffort = 1.0F;
      break;
  }

  koi.speed +=
      (desiredSpeed - koi.speed) * (1.0F - std::exp(-speedResponse * dt));
  koi.tailEffort += (desiredTailEffort - koi.tailEffort) *
                    (1.0F - std::exp(-4.5F * dt));
  koi.velocity = Mul(FromAngle(koi.heading), koi.speed);
  koi.position = Add(koi.position, Mul(koi.velocity, dt));

  const float beatRate = 0.45F + (koi.speed / koi.maximumSpeed) * 4.6F +
                         koi.tailEffort * 0.9F;
  koi.swimPhase += beatRate * dt;

  koi.spine[0] = koi.position;
  const float spacing = koi.bodyLength / static_cast<float>(kSpineNodes - 1);
  for (int node = 1; node < kSpineNodes; ++node) {
    const Vector2 fallback = Mul(FromAngle(koi.heading), -1.0F);
    const Vector2 direction =
        Normalize(Sub(koi.spine[node], koi.spine[node - 1]), fallback);
    const Vector2 constrained =
        Add(koi.spine[node - 1], Mul(direction, spacing));
    const float tailAmount =
        static_cast<float>(node) / static_cast<float>(kSpineNodes - 1);
    const float stiffness = 0.94F - tailAmount * 0.17F;
    koi.spine[node] = Lerp(koi.spine[node], constrained, stiffness);
  }
}

void School::AddRipple(Vector2 point) {
  Ripple &ripple = ripples_[nextRipple_];
  ripple.center = point;
  ripple.age = 0.0F;
  ripple.alive = true;
  nextRipple_ = (nextRipple_ + 1) % static_cast<int>(ripples_.size());
}

}  // namespace koi
