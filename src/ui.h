#pragma once
#include <Arduino.h>

namespace ui {
// Simple axis-aligned hit test.
inline bool inRect(int px, int py, int x, int y, int w, int h) {
  return px >= x && px < x + w && py >= y && py < y + h;
}
}  // namespace ui
