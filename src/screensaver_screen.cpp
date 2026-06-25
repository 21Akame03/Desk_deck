#include "screensaver_screen.h"
#include "app.h"
#include "theme.h"
#include "time_util.h"
#include <time.h>

// Off-screen buffer for the clock. Only stores &tft (safe before tft is
// constructed); the pixels are allocated in createSprite() at enter().
static TFT_eSprite clk = TFT_eSprite(&tft);

void ScreensaverScreen::formatTime(char* t, char* d) {
  if (!timeReady()) {
    strcpy(t, "--:--");
    strcpy(d, "");
    return;
  }
  time_t now = time(nullptr);
  struct tm tm;
  localtime_r(&now, &tm);
  strftime(t, 8, "%H:%M", &tm);
  strftime(d, 24, "%a %d %b", &tm);
}

void ScreensaverScreen::render() {
  char t[8], d[24];
  formatTime(t, d);
  clk.fillSprite(COLOR_BG);
  clk.setTextDatum(TC_DATUM);
  clk.setTextColor(COLOR_TEXT, COLOR_BG);
  clk.drawString(t, _w / 2, 0, 7);          // 7-segment, ~48px
  clk.setTextColor(COLOR_MUTED, COLOR_BG);
  clk.drawString(d, _w / 2, 54, 2);
}

void ScreensaverScreen::enter() {
  tft.fillScreen(COLOR_BG);

  char t[8], d[24];
  formatTime(t, d);
  int tw = tft.textWidth(t, 7);
  int dw = tft.textWidth(d, 2);
  _w = (tw > dw ? tw : dw) + 8;
  _h = 54 + 18;                              // time + date
  if (_w > 318) _w = 318;

  clk.deleteSprite();
  clk.createSprite(_w, _h);

  _x = (320 - _w) / 2;
  _y = (240 - _h) / 2;
  _vx = (millis() & 1) ? 2 : -2;             // pseudo-random direction
  _vy = (millis() & 2) ? 1 : -1;

  render();
  clk.pushSprite(_x, _y);
}

void ScreensaverScreen::tick() {
  int oldX = _x, oldY = _y;
  _x += _vx;
  _y += _vy;

  // Bounce off the edges.
  if (_x < 0)            { _x = 0;            _vx = -_vx; }
  if (_x > 320 - _w)     { _x = 320 - _w;     _vx = -_vx; }
  if (_y < 0)            { _y = 0;            _vy = -_vy; }
  if (_y > 240 - _h)     { _y = 240 - _h;     _vy = -_vy; }

  // Erase only the trailing strips uncovered by the move (no flicker).
  int dx = _x - oldX, dy = _y - oldY;
  if (dx > 0)      tft.fillRect(oldX, oldY, dx, _h, COLOR_BG);
  else if (dx < 0) tft.fillRect(_x + _w, oldY, -dx, _h, COLOR_BG);
  if (dy > 0)      tft.fillRect(oldX, oldY, _w, dy, COLOR_BG);
  else if (dy < 0) tft.fillRect(oldX, _y + _h, _w, -dy, COLOR_BG);

  render();
  clk.pushSprite(_x, _y);
}
