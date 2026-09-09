#include "renderer.hpp"

#include "config.hpp"
#include "koi.hpp"
#include "math_utils.hpp"
#include "school.hpp"

#include <raylib.h>

#include <algorithm>
#include <array>
#include <cmath>

namespace koi {
namespace {

void BuildRenderSpine(Koi &koi) {
  koi.renderSpine[0] = koi.spine[0];
  for (int node = 1; node < kSpineNodes; ++node) {
    const float t =
        static_cast<float>(node) / static_cast<float>(kSpineNodes - 1);
    const int previous = std::max(0, node - 1);
    const int next = std::min(kSpineNodes - 1, node + 1);
    const Vector2 tangent = Normalize(
        Sub(koi.spine[previous], koi.spine[next]), FromAngle(koi.heading));
    const Vector2 normal = Perpendicular(tangent);
    const float waveEnvelope = std::pow(t, 1.72F);
    const float wave = std::sin(koi.swimPhase - t * 6.1F) *
                       koi.bodyWidth * 1.15F * waveEnvelope *
                       (0.08F + koi.tailEffort * 0.92F);
    koi.renderSpine[node] = Add(koi.spine[node], Mul(normal, wave));
  }
}

float WidthAt(const Koi &koi, int node) {
  const float t =
      static_cast<float>(node) / static_cast<float>(kSpineNodes - 1);
  float profile = 0.0F;
  if (t < 0.18F) {
    profile = 0.73F + (t / 0.18F) * 0.27F;
  } else {
    const float tail = (t - 0.18F) / 0.82F;
    profile = std::pow(std::max(0.0F, 1.0F - tail), 0.72F);
  }
  return std::max(0.7F, koi.bodyWidth * profile);
}

void DrawKoi(const Koi &koi) {
  std::array<Vector2, kSpineNodes> left{};
  std::array<Vector2, kSpineNodes> right{};

  for (int node = 0; node < kSpineNodes; ++node) {
    const int previous = std::max(0, node - 1);
    const int next = std::min(kSpineNodes - 1, node + 1);
    const Vector2 tangent = Normalize(
        Sub(koi.renderSpine[previous], koi.renderSpine[next]),
        FromAngle(koi.heading));
    const Vector2 normal = Perpendicular(tangent);
    const float halfWidth = WidthAt(koi, node);
    left[node] = Add(koi.renderSpine[node], Mul(normal, halfWidth));
    right[node] = Add(koi.renderSpine[node], Mul(normal, -halfWidth));
  }

  constexpr int pectoralFront = 3;
  constexpr int pectoralCenter = 4;
  constexpr int pectoralBack = 6;
  const Vector2 pectoralTangent = Normalize(
      Sub(koi.renderSpine[pectoralCenter - 1],
          koi.renderSpine[pectoralCenter + 1]),
      FromAngle(koi.heading));
  const Vector2 pectoralNormal = Perpendicular(pectoralTangent);
  const float paddleActivity =
      koi.state == SwimState::Hover
          ? 1.0F
          : (koi.state == SwimState::Pivot ? 0.85F : 0.45F);
  const float finPulse =
      0.82F + paddleActivity * 0.25F *
                  std::sin(koi.swimPhase * 0.64F + koi.phaseOffset);
  const float pectoralReach =
      koi.bodyWidth * (0.55F + paddleActivity * 0.25F) * finPulse;
  const Vector2 leftPectoral =
      Add(Add(left[pectoralCenter], Mul(pectoralNormal, pectoralReach)),
          Mul(pectoralTangent, -koi.bodyWidth * 0.22F));
  const Vector2 rightPectoral =
      Add(Add(right[pectoralCenter], Mul(pectoralNormal, -pectoralReach)),
          Mul(pectoralTangent, -koi.bodyWidth * 0.22F));
  DrawTriangle(left[pectoralFront], leftPectoral, left[pectoralBack], WHITE);
  DrawTriangle(right[pectoralFront], right[pectoralBack], rightPectoral,
               WHITE);

  constexpr int pelvicFront = 7;
  constexpr int pelvicCenter = 8;
  constexpr int pelvicBack = 9;
  const Vector2 pelvicTangent = Normalize(
      Sub(koi.renderSpine[pelvicCenter - 1],
          koi.renderSpine[pelvicCenter + 1]),
      FromAngle(koi.heading));
  const Vector2 pelvicNormal = Perpendicular(pelvicTangent);
  const float pelvicReach = koi.bodyWidth * (0.28F + 0.05F * finPulse);
  const Vector2 leftPelvic =
      Add(left[pelvicCenter], Mul(pelvicNormal, pelvicReach));
  const Vector2 rightPelvic =
      Add(right[pelvicCenter], Mul(pelvicNormal, -pelvicReach));
  DrawTriangle(left[pelvicFront], leftPelvic, left[pelvicBack], WHITE);
  DrawTriangle(right[pelvicFront], right[pelvicBack], rightPelvic, WHITE);

  for (int node = kSpineNodes - 2; node >= 0; --node) {
    DrawTriangle(left[node], right[node], right[node + 1], WHITE);
    DrawTriangle(left[node], right[node + 1], left[node + 1], WHITE);
  }

  const Vector2 headForward = Normalize(
      Sub(koi.renderSpine[0], koi.renderSpine[1]), FromAngle(koi.heading));
  const Vector2 headNormal = Perpendicular(headForward);
  const Vector2 noseCenter =
      Add(koi.renderSpine[0], Mul(headForward, koi.bodyWidth * 0.43F));
  const float noseHalfWidth = WidthAt(koi, 0) * 0.72F;
  const Vector2 noseLeft = Add(noseCenter, Mul(headNormal, noseHalfWidth));
  const Vector2 noseRight =
      Add(noseCenter, Mul(headNormal, -noseHalfWidth));
  DrawTriangle(left[0], noseLeft, noseRight, WHITE);
  DrawTriangle(left[0], noseRight, right[0], WHITE);
  DrawCircleV(noseCenter, std::max(1.0F, noseHalfWidth * 0.72F), WHITE);

  constexpr int tailNode = kSpineNodes - 1;
  const Vector2 tailForward = Normalize(
      Sub(koi.renderSpine[tailNode - 1], koi.renderSpine[tailNode]),
      FromAngle(koi.heading));
  const Vector2 tailNormal = Perpendicular(tailForward);
  const float fan =
      koi.bodyWidth * (1.22F + 0.2F * std::sin(koi.swimPhase - 0.8F));
  const Vector2 finCenter = Add(
      koi.renderSpine[tailNode], Mul(tailForward, -koi.bodyWidth * 1.05F));
  const Vector2 tailTip = Add(
      koi.renderSpine[tailNode], Mul(tailForward, -koi.bodyWidth * 2.05F));
  const Vector2 upperFin = Add(finCenter, Mul(tailNormal, fan));
  const Vector2 lowerFin = Add(finCenter, Mul(tailNormal, -fan));
  DrawTriangle(koi.renderSpine[tailNode], upperFin, tailTip, WHITE);
  DrawTriangle(koi.renderSpine[tailNode], tailTip, lowerFin, WHITE);

  std::array<Vector2, kSpineNodes> innerLeft{};
  std::array<Vector2, kSpineNodes> innerRight{};
  for (int node = 0; node < kSpineNodes; ++node) {
    const int previous = std::max(0, node - 1);
    const int next = std::min(kSpineNodes - 1, node + 1);
    const Vector2 tangent = Normalize(
        Sub(koi.renderSpine[previous], koi.renderSpine[next]),
        FromAngle(koi.heading));
    const Vector2 normal = Perpendicular(tangent);
    const float insetWidth = std::max(0.0F, WidthAt(koi, node) - 1.65F);
    innerLeft[node] = Add(koi.renderSpine[node], Mul(normal, insetWidth));
    innerRight[node] = Add(koi.renderSpine[node], Mul(normal, -insetWidth));
  }
  for (int node = kSpineNodes - 2; node >= 0; --node) {
    DrawTriangle(innerLeft[node], innerRight[node], innerRight[node + 1],
                 BLACK);
    DrawTriangle(innerLeft[node], innerRight[node + 1], innerLeft[node + 1],
                 BLACK);
  }

  const float innerNoseWidth = std::max(0.0F, noseHalfWidth - 1.65F);
  const Vector2 innerNoseLeft =
      Add(noseCenter, Mul(headNormal, innerNoseWidth));
  const Vector2 innerNoseRight =
      Add(noseCenter, Mul(headNormal, -innerNoseWidth));
  DrawTriangle(innerLeft[0], innerNoseLeft, innerNoseRight, BLACK);
  DrawTriangle(innerLeft[0], innerNoseRight, innerRight[0], BLACK);
  if (innerNoseWidth > 0.6F) {
    DrawCircleV(noseCenter, innerNoseWidth * 0.67F, BLACK);
  }

  DrawLineV(left[pectoralFront], leftPectoral, WHITE);
  DrawLineV(leftPectoral, left[pectoralBack], WHITE);
  DrawLineV(right[pectoralFront], rightPectoral, WHITE);
  DrawLineV(rightPectoral, right[pectoralBack], WHITE);
  DrawLineV(left[pelvicFront], leftPelvic, WHITE);
  DrawLineV(leftPelvic, left[pelvicBack], WHITE);
  DrawLineV(right[pelvicFront], rightPelvic, WHITE);
  DrawLineV(rightPelvic, right[pelvicBack], WHITE);
}

void DrawDebug(const School &school) {
  for (int i = 0; i < school.Count(); ++i) {
    const Koi &fish = school.Fish(i);
    for (int node = 0; node < kSpineNodes - 1; ++node) {
      DrawLineV(fish.renderSpine[node], fish.renderSpine[node + 1], WHITE);
    }
    for (const Vector2 point : fish.renderSpine) {
      DrawRectangle(static_cast<int>(point.x), static_cast<int>(point.y), 1, 1,
                    BLACK);
    }
  }
}

}  // namespace

void DrawSchool(School &school, float time, bool showDebug) {
  for (int i = 0; i < school.Count(); ++i) {
    Koi &fish = school.Fish(i);
    BuildRenderSpine(fish);
    DrawKoi(fish);
  }

  for (const Ripple &ripple : school.Ripples()) {
    if (!ripple.alive) continue;
    const float progress = ripple.age / 1.25F;
    const int radius = static_cast<int>(4.0F + progress * 19.0F);
    if ((static_cast<int>(time * 12.0F) & 1) == 0) {
      DrawCircleLines(static_cast<int>(ripple.center.x),
                      static_cast<int>(ripple.center.y),
                      static_cast<float>(radius), WHITE);
    }
  }

  if (showDebug) DrawDebug(school);
}

Vector2 WindowToCanvas(Vector2 mouse, Rectangle destination) {
  return {(mouse.x - destination.x) * kCanvasWidth / destination.width,
          (mouse.y - destination.y) * kCanvasHeight / destination.height};
}

Rectangle CanvasDestination() {
  const float scale =
      std::min(static_cast<float>(GetScreenWidth()) / kCanvasWidth,
               static_cast<float>(GetScreenHeight()) / kCanvasHeight);
  const float width = kCanvasWidth * scale;
  const float height = kCanvasHeight * scale;
  return {(GetScreenWidth() - width) * 0.5F,
          (GetScreenHeight() - height) * 0.5F, width, height};
}

void DrawInterface(const School &school, bool debug) {
  DrawText("PROCEDURAL KOI / ONE-BIT SPINE STUDY", 8, 7, 7, WHITE);
  const char *state =
      school.TargetActive() ? "STATE: CALLED" : "STATE: WANDERING";
  DrawText(state, 8, kCanvasHeight - 14, 7, WHITE);
  DrawText(TextFormat("FISH: %02i  FPS: %03i", school.Count(), GetFPS()),
           kCanvasWidth - 102, 7, 7, WHITE);
  DrawText("CLICK CALLS  SPACE SCATTERS  [ ] COUNT  D SPINE  F11 FULLSCREEN",
           kCanvasWidth - 300, kCanvasHeight - 14, 7, WHITE);
  if (debug) DrawText("SPINE VIEW", 8, 18, 7, WHITE);
}

}  // namespace koi
