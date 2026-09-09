#pragma once

#include "config.hpp"
#include "koi.hpp"
#include "math_utils.hpp"

#include <raylib.h>

#include <array>

namespace koi {

struct Ripple {
  Vector2 center{};
  float age = 0.0F;
  bool alive = false;
};

class School {
 public:
  School();

  void SetCount(int count);
  int Count() const;
  void Reset();
  void CallTo(Vector2 point);
  void Scatter();
  void Update(float dt, float time);

  bool TargetActive() const;
  void LogBehaviorSummary() const;
  Koi &Fish(int index);
  const Koi &Fish(int index) const;
  const std::array<Ripple, kMaxRipples> &Ripples() const;

 private:
  static float BehaviorUnit(Koi &koi);
  static float BehaviorRange(Koi &koi, float low, float high);
  static void EnterState(Koi &koi, SwimState next);
  static void UpdateNaturalState(Koi &koi, float dt);
  static void Integrate(Koi &koi, Vector2 desired, float desiredSpeed,
                        float dt);

  Vector2 SteeringFor(int index, float time) const;
  float DesiredSpeedFor(int index) const;
  void AddRipple(Vector2 point);

  std::array<Koi, kMaxFish> fish_{};
  std::array<Ripple, kMaxRipples> ripples_{};
  Random random_{};
  Vector2 target_{kCanvasWidth * 0.5F, kCanvasHeight * 0.5F};
  int count_ = kInitialFish;
  int nextRipple_ = 0;
  float targetAge_ = 0.0F;
  bool targetActive_ = false;
};

}  // namespace koi
