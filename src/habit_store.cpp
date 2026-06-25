#include "habit_store.h"
#include <Preferences.h>
#include "time_util.h"

static Preferences prefs;
static const char* NS = "tracker";

void HabitStore::begin() {
  prefs.begin(NS, false);
  for (int i = 0; i < HABIT_COUNT; i++) load(i);
  refresh();
}

void HabitStore::load(int i) {
  HabitData d{};
  char key[8];
  snprintf(key, sizeof(key), "h%d", i);
  size_t n = prefs.getBytes(key, &d, sizeof(d));
  if (n != sizeof(d)) {  // first run / changed layout -> fresh state
    d = HabitData{};
    d.version = 1;
  }
  _data[i] = d;
}

void HabitStore::save(int i) {
  char key[8];
  snprintf(key, sizeof(key), "h%d", i);
  prefs.putBytes(key, &_data[i], sizeof(HabitData));
}

void HabitStore::pushHistory(HabitData& h, uint16_t c) {
  for (int k = 0; k < HISTORY_DAYS - 1; k++) h.history[k] = h.history[k + 1];
  h.history[HISTORY_DAYS - 1] = c;
}

// Advance the habit from its stored day up to `today`, closing each day
// in between: archive it to history and extend or break the streak.
void HabitStore::rollover(int i, long today) {
  if (today < 0) return;  // time unknown -> don't touch anything
  HabitData& h = _data[i];
  if (h.lastDay == 0) {   // never seen a date before
    h.lastDay = today;
    return;
  }
  if (today == h.lastDay) return;

  uint16_t g = HABITS[i].goal;
  for (long d = h.lastDay; d < today; d++) {
    uint16_t c = (d == h.lastDay) ? h.todayCount : 0;  // skipped days = 0
    pushHistory(h, c);
    if (c >= g) h.streak++;
    else        h.streak = 0;
    if (h.streak > h.best) h.best = h.streak;
  }
  h.todayCount = 0;
  h.lastDay = today;
  save(i);
}

void HabitStore::refresh() {
  long t = todayIndex();
  for (int i = 0; i < HABIT_COUNT; i++) rollover(i, t);
}

void HabitStore::increment(int i) {
  rollover(i, todayIndex());
  if (_data[i].todayCount < 9999) _data[i].todayCount++;
  uint16_t ds = displayStreak(i);
  if (ds > _data[i].best) _data[i].best = ds;
  save(i);
}

void HabitStore::decrement(int i) {
  rollover(i, todayIndex());
  if (_data[i].todayCount > 0) _data[i].todayCount--;
  save(i);
}

void HabitStore::resetToday(int i) {
  _data[i].todayCount = 0;
  save(i);
}

uint16_t HabitStore::displayStreak(int i) {
  return _data[i].streak + (metToday(i) ? 1 : 0);
}
