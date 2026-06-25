#pragma once
#include "screen.h"

// The "Daily Tracker" page: a 2x2 grid of habit cards. Tap a card to
// increment today's count; tap the "i" badge to open detail. A back
// button (top-left) returns to the Stella home screen.
class TrackerScreen : public Screen {
 public:
  void enter() override;
  void handleTouch(int x, int y) override;
  void tick() override;

 private:
  long _lastDay = -999;
  void drawHeader();
  void drawCard(int i);
  void cardRect(int i, int& x, int& y, int& w, int& h);
  int  shownCount();
};
