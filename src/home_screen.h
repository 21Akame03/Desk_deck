#pragma once
#include "screen.h"

// Stella home / landing screen. Shows the app name, the date, and a tile
// for the Daily Tracker (with today's progress). Tapping the tile opens
// the tracker. Room to add more app tiles later.
class HomeScreen : public Screen {
 public:
  void enter() override;
  void handleTouch(int x, int y) override;
  void tick() override;

 private:
  long _lastDay = -999;
  int  _lastMin = -1;
  void draw();
  void drawClock();
};
