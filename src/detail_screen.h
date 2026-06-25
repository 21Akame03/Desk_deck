#pragma once
#include "screen.h"

// Single-habit view: big counter with -/+ buttons, streak/best/goal line,
// a 7-day bar chart, and a reset-today button. Back returns to home.
class DetailScreen : public Screen {
 public:
  void enter() override;
  void handleTouch(int x, int y) override;

 private:
  void drawStatic();  // chrome that never changes while on this screen
  void drawCount();   // the number + streak line
  void drawChart();   // 7-day bars
};
