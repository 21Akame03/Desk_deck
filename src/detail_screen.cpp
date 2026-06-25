#include "detail_screen.h"
#include "app.h"
#include "theme.h"
#include "ui.h"

// Tap targets (also used for drawing). The back button is drawn large and
// has an even larger hit area for finger / wall-mounted use.
static const int BACK_X = 6,   BACK_Y = 2,  BACK_W = 100, BACK_H = 30;
static const int BACK_HIT_X = 0, BACK_HIT_Y = 0, BACK_HIT_W = 120, BACK_HIT_H = 44;
static const int BTN_W = 64,   BTN_H = 64,  BTN_Y = 86;
static const int MINUS_X = 24, PLUS_X = 232;
static const int RESET_X = 110, RESET_Y = 206, RESET_W = 100, RESET_H = 28;

void DetailScreen::drawStatic() {
  int i = g_selectedHabit;

  // Header + back button + title
  tft.fillRect(0, 0, 320, HEADER_H, COLOR_HEADER);
  tft.fillRoundRect(BACK_X, BACK_Y, BACK_W, BACK_H, 6, COLOR_BADGE);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_TEXT, COLOR_BADGE);
  tft.drawString("< Back", BACK_X + BACK_W / 2, BACK_Y + BACK_H / 2, 4);
  tft.setTextColor(COLOR_TEXT, COLOR_HEADER);
  tft.drawString(store.name(i), 214, HEADER_H / 2, 4);

  // Minus / plus buttons (font 4 so '+' renders; font 6 has no '+')
  tft.fillRoundRect(MINUS_X, BTN_Y, BTN_W, BTN_H, 12, COLOR_CARD);
  tft.fillRoundRect(PLUS_X,  BTN_Y, BTN_W, BTN_H, 12, COLOR_ACCENT);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_TEXT, COLOR_CARD);
  tft.drawString("-", MINUS_X + BTN_W / 2, BTN_Y + BTN_H / 2 - 2, 4);
  tft.setTextColor(COLOR_TEXT, COLOR_ACCENT);
  tft.drawString("+", PLUS_X + BTN_W / 2, BTN_Y + BTN_H / 2 - 2, 4);

  // Reset-today button
  tft.fillRoundRect(RESET_X, RESET_Y, RESET_W, RESET_H, 6, COLOR_CARD);
  tft.setTextColor(COLOR_MUTED, COLOR_CARD);
  tft.drawString("Reset today", RESET_X + RESET_W / 2, RESET_Y + RESET_H / 2, 2);
}

void DetailScreen::drawCount() {
  int i = g_selectedHabit;

  // Big 7-seg number between the buttons.
  tft.fillRect(96, 78, 136, 70, COLOR_BG);
  char buf[8];
  snprintf(buf, sizeof(buf), "%u", store.get(i).todayCount);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(store.metToday(i) ? COLOR_GREEN : COLOR_TEXT, COLOR_BG);
  tft.drawString(buf, 164, 112, 7);

  // Streak / best / goal line.
  tft.fillRect(0, 150, 320, 20, COLOR_BG);
  char line[48];
  snprintf(line, sizeof(line), "Streak %u    Best %u    Goal %u",
           store.displayStreak(i), store.get(i).best, store.goal(i));
  tft.setTextColor(COLOR_MUTED, COLOR_BG);
  tft.drawString(line, 160, 160, 2);
}

void DetailScreen::drawChart() {
  int i = g_selectedHabit;
  HabitData& h = store.get(i);

  // 7 values: 6 closed days + today (rightmost, highlighted).
  uint16_t vals[7];
  for (int k = 0; k < HISTORY_DAYS; k++) vals[k] = h.history[k];
  vals[6] = h.todayCount;

  uint16_t mx = 1;
  for (int k = 0; k < 7; k++)
    if (vals[k] > mx) mx = vals[k];

  const int baseY = 202, maxH = 26, slotW = 40, x0 = 20, bw = 26;
  tft.fillRect(0, 172, 320, 32, COLOR_BG);
  for (int k = 0; k < 7; k++) {
    int bx = x0 + k * slotW;
    int bh = (int)((long)vals[k] * maxH / mx);
    if (vals[k] > 0 && bh < 2) bh = 2;           // keep tiny bars visible
    uint16_t c = (k == 6) ? COLOR_ACCENT : COLOR_BAR;
    tft.fillRect(bx, baseY - bh, bw, bh, c);
  }
}

void DetailScreen::enter() {
  tft.fillScreen(COLOR_BG);
  drawStatic();
  drawCount();
  drawChart();
}

void DetailScreen::handleTouch(int tx, int ty) {
  int i = g_selectedHabit;
  if (ui::inRect(tx, ty, BACK_HIT_X, BACK_HIT_Y, BACK_HIT_W, BACK_HIT_H)) {
    screens.show(gTrackerScreen);
    return;
  }
  if (ui::inRect(tx, ty, PLUS_X, BTN_Y, BTN_W, BTN_H)) {
    store.increment(i);
    drawCount();
    drawChart();
    return;
  }
  if (ui::inRect(tx, ty, MINUS_X, BTN_Y, BTN_W, BTN_H)) {
    store.decrement(i);
    drawCount();
    drawChart();
    return;
  }
  if (ui::inRect(tx, ty, RESET_X, RESET_Y, RESET_W, RESET_H)) {
    store.resetToday(i);
    drawCount();
    drawChart();
    return;
  }
}
