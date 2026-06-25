#pragma once

// Base class for every screen. A screen owns a full region of the display:
//   enter()       - draw itself from scratch (called when shown)
//   handleTouch() - react to a tap at screen coords (x,y)
//   tick()        - periodic work (~1Hz), e.g. clock / day rollover
class Screen {
 public:
  virtual void enter() = 0;
  virtual void handleTouch(int x, int y) {}
  virtual void tick() {}
  virtual ~Screen() {}
};
