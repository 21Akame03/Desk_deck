#pragma once
#include <Arduino.h>
#include "config.h"

#define HISTORY_DAYS 6  // previous closed days kept for the chart

// Per-habit dynamic state. Persisted to NVS as a raw blob, keyed by index.
struct HabitData {
  uint16_t todayCount;             // increments so far today
  uint16_t streak;                 // completed days strictly BEFORE today
  uint16_t best;                   // best streak ever (incl. a live today)
  uint16_t history[HISTORY_DAYS];  // last closed days; [N-1] = most recent
  long     lastDay;                // day index todayCount belongs to (0 = new)
  uint16_t version;
};

// Owns all habit state and its persistence. Names/goals come from the
// compile-time HABITS[] table; only counts/streaks/history are stored.
class HabitStore {
 public:
  void begin();
  void refresh();          // re-check day rollover for every habit
  void increment(int i);
  void decrement(int i);
  void resetToday(int i);

  HabitData&  get(int i)  { return _data[i]; }
  const char* name(int i) { return HABITS[i].name; }
  uint16_t    goal(int i) { return HABITS[i].goal; }
  bool        metToday(int i) { return _data[i].todayCount >= HABITS[i].goal; }
  uint16_t    displayStreak(int i);  // streak including today if done

 private:
  HabitData _data[HABIT_COUNT];
  void load(int i);
  void save(int i);
  void rollover(int i, long today);
  void pushHistory(HabitData& h, uint16_t c);
};
