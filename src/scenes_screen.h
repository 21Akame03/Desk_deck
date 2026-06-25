#pragma once
#include "screen.h"

// "Scenes" page: a 2x2 grid of IKEA Dirigera scene tiles. Tap a tile to
// trigger that scene on the hub (one HTTPS PUT). A large back button
// (top-left) returns to the Stella home screen.
class ScenesScreen : public Screen {
 public:
  void enter() override;
  void handleTouch(int x, int y) override;

 private:
  void drawHeader();
  void drawTile(int i, const char* status);  // status==nullptr -> idle look
  void tileRect(int i, int& x, int& y, int& w, int& h);
  int  shownCount();
};
