#pragma once
#include "screen.h"

// Dimmed screensaver: a large clock (HH:MM) plus date that slowly drifts
// and bounces around the screen, so no pixels are lit constantly.
// Waking is handled by main (a touch returns to the Stella home).
class ScreensaverScreen : public Screen {
 public:
  void enter() override;
  void tick() override;

 private:
  int _x, _y, _vx, _vy, _w, _h;
  void render();                    // draw current time into the sprite
  void formatTime(char* t, char* d);
};
