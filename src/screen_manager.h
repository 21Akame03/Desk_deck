#pragma once
#include "screen.h"

// Holds the active screen and routes input/ticks to it.
class ScreenManager {
 public:
  void show(Screen* s) {
    _cur = s;
    if (_cur) _cur->enter();
  }
  void touch(int x, int y) { if (_cur) _cur->handleTouch(x, y); }
  void tick()              { if (_cur) _cur->tick(); }
  Screen* current()        { return _cur; }

 private:
  Screen* _cur = nullptr;
};
