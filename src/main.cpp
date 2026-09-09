#include "config.hpp"
#include "renderer.hpp"
#include "school.hpp"

#include <raylib.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>

int main(int argc, char **argv) {
  using namespace koi;

  const bool captureMode = argc > 1 && std::strcmp(argv[1], "--capture") == 0;
  const char *capturePath = argc > 2 ? argv[2] : "procedural-koi-preview.png";
  const int captureFrame =
      argc > 3 ? std::max(1, std::atoi(argv[3])) : 240;

  SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
  InitWindow(1280, 720, "Procedural Koi — one-bit spine study");
  SetWindowMinSize(800, 450);
  SetExitKey(KEY_ESCAPE);
  SetTargetFPS(120);

  RenderTexture2D canvas = LoadRenderTexture(kCanvasWidth, kCanvasHeight);
  SetTextureFilter(canvas.texture, TEXTURE_FILTER_POINT);

  School school;
  bool showInterface = true;
  bool showDebug = false;
  float accumulator = 0.0F;
  float simulationTime = 0.0F;
  int renderedFrames = 0;
  bool captureComplete = false;

  while (!WindowShouldClose() && !captureComplete) {
    const Rectangle destination = CanvasDestination();

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(GetMousePosition(), destination)) {
      school.CallTo(WindowToCanvas(GetMousePosition(), destination));
    }
    if (IsKeyPressed(KEY_SPACE)) school.Scatter();
    if (IsKeyPressed(KEY_R)) school.Reset();
    if (IsKeyPressed(KEY_LEFT_BRACKET)) school.SetCount(school.Count() - 1);
    if (IsKeyPressed(KEY_RIGHT_BRACKET)) school.SetCount(school.Count() + 1);
    if (IsKeyPressed(KEY_H)) showInterface = !showInterface;
    if (IsKeyPressed(KEY_D)) showDebug = !showDebug;
    if (IsKeyPressed(KEY_F11)) ToggleFullscreen();
    if (captureMode && renderedFrames == 45) {
      school.CallTo({kCanvasWidth * 0.68F, kCanvasHeight * 0.52F});
    }

    accumulator += std::min(GetFrameTime(), 0.1F);
    while (accumulator >= kFixedStep) {
      simulationTime += kFixedStep;
      school.Update(kFixedStep, simulationTime);
      accumulator -= kFixedStep;
    }

    BeginTextureMode(canvas);
    ClearBackground(BLACK);
    DrawSchool(school, simulationTime, showDebug);
    if (showInterface && !captureMode) DrawInterface(school, showDebug);
    EndTextureMode();

    if (captureMode && renderedFrames == captureFrame) {
      school.LogBehaviorSummary();
      Image preview = LoadImageFromTexture(canvas.texture);
      ImageFlipVertical(&preview);
      ExportImage(preview, capturePath);
      UnloadImage(preview);
      captureComplete = true;
    }

    BeginDrawing();
    ClearBackground(BLACK);
    const Rectangle source{0.0F, 0.0F,
                           static_cast<float>(canvas.texture.width),
                           -static_cast<float>(canvas.texture.height)};
    DrawTexturePro(canvas.texture, source, destination, {0.0F, 0.0F}, 0.0F,
                   WHITE);
    EndDrawing();
    ++renderedFrames;
  }

  UnloadRenderTexture(canvas);
  CloseWindow();
  return 0;
}
