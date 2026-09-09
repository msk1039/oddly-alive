#pragma once

#include <raylib.h>

namespace koi {

class School;

void DrawSchool(School &school, float time, bool showDebug);
void DrawInterface(const School &school, bool debug);
Vector2 WindowToCanvas(Vector2 mouse, Rectangle destination);
Rectangle CanvasDestination();

}  // namespace koi
