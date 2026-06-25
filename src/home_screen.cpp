#include "home_screen.h"
#include "app.h"
#include "theme.h"
#include "time_util.h"
#include "ui.h"
#include <time.h>

// Two full-width app tiles: Daily Tracker on top, Scenes below.
static const int TILE_X = 20, TILE_W = 280, TILE_H = 58;
static const int TRACKER_Y = 104;
static const int SCENES_Y  = 170;

static int shownHabits() { return HABIT_COUNT < 4 ? HABIT_COUNT : 4; }

static int doneToday() {
  int n = shownHabits(), d = 0;
  for (int i = 0; i < n; i++)
    if (store.metToday(i)) d++;
  return d;
}

static void drawTile(int x, int y, int w, const char* title, const char* sub,
                     uint16_t accent) {
  tft.fillRoundRect(x, y, w, TILE_H, 10, COLOR_CARD);
  tft.fillRoundRect(x, y, 6, TILE_H, 3, accent);  // coloured edge strip
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(accent, COLOR_CARD);
  tft.drawString(title, x + 16, y + 8, 4);
  tft.setTextColor(COLOR_MUTED, COLOR_CARD);
  tft.drawString(sub, x + 16, y + 38, 2);
  tft.setTextDatum(MR_DATUM);
  tft.setTextColor(accent, COLOR_CARD);
  tft.drawString(">", x + w - 16, y + TILE_H / 2, 4);
}

void HomeScreen::drawClock() {
  tft.fillRect(0, 24, 320, 74, COLOR_BG);   // clear the clock area

  char t[8], d[24];
  if (timeReady()) {
    time_t now = time(nullptr);
    struct tm tm;
    localtime_r(&now, &tm);
    strftime(t, sizeof(t), "%H:%M", &tm);
    strftime(d, sizeof(d), "%a %d %b", &tm);
    _lastMin = tm.tm_min;
  } else {
    strcpy(t, "--:--");
    strcpy(d, "syncing time...");
    _lastMin = -1;
  }

  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(COLOR_TEXT, COLOR_BG);
  tft.drawString(t, 160, 26, 7);            // big 7-segment clock
  tft.setTextColor(COLOR_MUTED, COLOR_BG);
  tft.drawString(d, 160, 80, 2);
}

void HomeScreen::draw() {
  tft.fillScreen(COLOR_BG);

  // Brand mark (top-left, small)
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(COLOR_ACCENT, COLOR_BG);
  tft.drawString("Stella", 12, 8, 2);

  drawClock();

  char sub[32];
  snprintf(sub, sizeof(sub), "%d / %d done today", doneToday(), shownHabits());
  drawTile(TILE_X, TRACKER_Y, TILE_W, "Daily Tracker", sub, COLOR_GREEN);

  snprintf(sub, sizeof(sub), "%d scenes", SCENE_COUNT);
  drawTile(TILE_X, SCENES_Y, TILE_W, "Scenes", sub, COLOR_MAUVE);

  _lastDay = todayIndex();
}

void HomeScreen::enter() { draw(); }

void HomeScreen::handleTouch(int tx, int ty) {
  if (ui::inRect(tx, ty, TILE_X, TRACKER_Y, TILE_W, TILE_H)) {
    screens.show(gTrackerScreen);
    return;
  }
  if (ui::inRect(tx, ty, TILE_X, SCENES_Y, TILE_W, TILE_H)) {
    screens.show(gScenesScreen);
    return;
  }
}

void HomeScreen::tick() {
  long day = todayIndex();
  if (day != _lastDay) {          // day changed / time first synced -> redraw
    store.refresh();
    draw();
    return;
  }
  // Keep the clock current (update when the minute changes).
  if (timeReady()) {
    time_t now = time(nullptr);
    struct tm tm;
    localtime_r(&now, &tm);
    if (tm.tm_min != _lastMin) drawClock();
  }
}
