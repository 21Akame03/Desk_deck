#pragma once
#include <Arduino.h>
#include <time.h>
#include "config.h"

// True once NTP has given us a real wall-clock time.
inline bool timeReady() { return time(nullptr) > 1600000000UL; }

// A monotonically increasing "day number" in local time, used to detect
// day rollovers. Returns -1 if the time isn't known yet.
inline long todayIndex() {
  if (!timeReady()) return -1;
  return (long)((time(nullptr) + (long)TZ_OFFSET_HOURS * 3600L) / 86400L);
}

// e.g. "Mon 15 Jun"
inline String dateString() {
  time_t t = time(nullptr);
  struct tm tm;
  localtime_r(&t, &tm);
  char buf[24];
  strftime(buf, sizeof(buf), "%a %d %b", &tm);
  return String(buf);
}
