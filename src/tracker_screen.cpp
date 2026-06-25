#include "tracker_screen.h"
#include "app.h"
#include "theme.h"
#include "time_util.h"
#include "ui.h"

static const int BADGE_R = 14;
// Back button (top-left). Drawn size is generous; the tappable area is
// larger still, so a finger anywhere in the top-left corner registers.
static const int BACK_X = 6, BACK_Y = 2, BACK_W = 100, BACK_H = 30;
static const int BACK_HIT_X = 0, BACK_HIT_Y = 0, BACK_HIT_W = 120, BACK_HIT_H = 40;

int TrackerScreen::shownCount() { return HABIT_COUNT < 4 ? HABIT_COUNT : 4; }

void TrackerScreen::cardRect(int i, int& x, int& y, int& w, int& h) {
  int col = i % 2, row = i / 2;
  w = 148;
  h = 91;
  x = 8 + col * 156;        // 148 card + 8 gap
  y = 42 + row * 99;        // below 34px header + 8 margin, 91 card + 8 gap
}

void TrackerScreen::drawHeader() {
  tft.fillRect(0, 0, 320, HEADER_H, COLOR_HEADER);
  tft.fillRoundRect(BACK_X, BACK_Y, BACK_W, BACK_H, 6, COLOR_BADGE);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_TEXT, COLOR_BADGE);
  tft.drawString("< Back", BACK_X + BACK_W / 2, BACK_Y + BACK_H / 2, 4);
  // Page title stays "Daily Tracker".
  tft.setTextColor(COLOR_TEXT, COLOR_HEADER);
  tft.drawString("Daily Tracker", 218, HEADER_H / 2, 4);
}

void TrackerScreen::drawCard(int i) {
  int x, y, w, h;
  cardRect(i, x, y, w, h);
  bool met = store.metToday(i);
  uint16_t bg = met ? COLOR_CARD_DONE : COLOR_CARD;

  tft.fillRoundRect(x, y, w, h, 8, bg);

  // Habit name
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(COLOR_TEXT, bg);
  tft.drawString(store.name(i), x + 10, y + 8, 2);

  // Big count
  char buf[8];
  snprintf(buf, sizeof(buf), "%u", store.get(i).todayCount);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(met ? COLOR_GREEN : COLOR_ACCENT, bg);
  tft.drawString(buf, x + w / 2, y + h / 2 + 8, 6);

  // Goal + streak footer
  char foot[28];
  snprintf(foot, sizeof(foot), "goal %u  streak %u",
           store.goal(i), store.displayStreak(i));
  tft.setTextDatum(BL_DATUM);
  tft.setTextColor(COLOR_MUTED, bg);
  tft.drawString(foot, x + 10, y + h - 6, 1);

  // "i" detail badge (top-right)
  int bx = x + w - 16, by = y + 16;
  tft.fillCircle(bx, by, BADGE_R, COLOR_BADGE);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_TEXT, COLOR_BADGE);
  tft.drawString("i", bx, by - 1, 2);
}

void TrackerScreen::enter() {
  tft.fillScreen(COLOR_BG);
  drawHeader();
  for (int i = 0; i < shownCount(); i++) drawCard(i);
  _lastDay = todayIndex();
}

void TrackerScreen::handleTouch(int tx, int ty) {
  // Back to Stella home (large hit area for the top-left corner).
  if (ui::inRect(tx, ty, BACK_HIT_X, BACK_HIT_Y, BACK_HIT_W, BACK_HIT_H)) {
    screens.show(gHomeScreen);
    return;
  }

  for (int i = 0; i < shownCount(); i++) {
    int x, y, w, h;
    cardRect(i, x, y, w, h);
    if (!ui::inRect(tx, ty, x, y, w, h)) continue;

    // Badge region (top-right corner) -> open detail.
    if (ui::inRect(tx, ty, x + w - 38, y, 38, 38)) {
      g_selectedHabit = i;
      screens.show(gDetailScreen);
      return;
    }
    // Anywhere else on the card -> increment.
    store.increment(i);
    drawCard(i);
    return;
  }
}

void TrackerScreen::tick() {
  long t = todayIndex();
  if (t != _lastDay) {   // midnight crossed (or first time sync) -> redraw
    store.refresh();
    enter();
  }
}
