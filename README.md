# Desk_deck — CYD habit counter

A simple daily habit tracker for the **CYD (Cheap Yellow Display / ESP32-2432S028R)**.
Tap a habit each time you do it; the count, streak, and 7-day history are saved
to flash so they survive reboots and roll over at midnight.

Built with PlatformIO, structured around **screens**.

## Screens

```
        ┌─────────────── HOME ───────────────┐
        │  Daily Tracker            Mon 15 Jun│
        │  ┌──────────┐(i)  ┌──────────┐(i)   │
        │  │ Workout  │     │ Read     │       │   tap card  -> +1 today
        │  │    2     │     │    0     │       │   tap (i)   -> detail
        │  │goal1 st 3│     │goal1 st 0│       │
        │  └──────────┘     └──────────┘       │
        │  ┌──────────┐(i)  ┌──────────┐(i)   │
        │  │ Water 2L │     │ Meditate │       │
        │  └──────────┘     └──────────┘       │
        └─────────────────┬───────────────────┘
                          │ tap (i)
                          ▼
        ┌────────────── DETAIL ──────────────┐
        │ < Back        Workout              │
        │                                    │
        │   [ - ]      2        [ + ]        │
        │                                    │
        │   Streak 3    Best 7    Goal 1     │
        │   ▁ ▃ █ ▂ ▅ ▁ █   (last 7 days)   │
        │            [ Reset today ]         │
        └────────────────────────────────────┘
```

- **Home** — 2×2 grid of habit cards. Tapping a card increments today's count
  (the main daily action). A card turns green once it hits its goal. The small
  **(i)** badge opens the detail screen.
- **Detail** — big counter with **−/＋**, current streak / best / goal, a 7-day
  bar chart (rightmost bar = today), and **Reset today**.

## Project layout

| File | Role |
|------|------|
| `include/config.h`      | **Edit this** — WiFi, timezone, habit list, touch pins/calibration |
| `src/main.cpp`          | Boot (WiFi+NTP), touch reading, main loop |
| `src/habit_store.*`     | Counts/streaks/history + NVS persistence + day rollover |
| `src/screen*.h`         | `Screen` base class + `ScreenManager` |
| `src/home_screen.*`     | Home grid |
| `src/detail_screen.*`   | Per-habit detail |
| `src/theme.h`, `ui.h`, `time_util.h` | Colours/layout, hit-testing, NTP date helpers |

Want more screens? Subclass `Screen`, add a global pointer in `app.h`/`main.cpp`,
and `screens.show(...)` to navigate.

## Build & flash

```bash
pio run                 # compile
pio run -t upload       # flash over USB
pio device monitor      # serial @ 115200
```

1. Edit `include/config.h`: set `WIFI_SSID` / `WIFI_PASS`, `TZ_OFFSET_HOURS`,
   and your `HABITS[]` (up to 4 shown).
2. Flash. On first boot it connects to WiFi, syncs time via NTP, then shows Home.

> WiFi/NTP is only used for the date. Without it the tracker still counts, but
> streaks and the midnight rollover won't advance until it knows the date
> (the ESP32 has no battery-backed clock).

## Tuning for your board

- **Blank screen / inverted colours:** in `platformio.ini` swap
  `-DILI9341_2_DRIVER=1` for `-DILI9341_DRIVER=1`.
- **Touches land in the wrong spot:** uncomment the `Serial.printf` in
  `readTouch()` (`src/main.cpp`), tap each corner, and adjust
  `TOUCH_X_MIN/MAX` and `TOUCH_Y_MIN/MAX` in `config.h`. Some panel batches
  also need axes swapped/inverted.

## Notes / possible extensions

- Data is keyed by habit **index**, so reordering `HABITS[]` remaps saved
  counts — add new habits at the end.
- More than 4 habits: only the first 4 render today; add paging in `HomeScreen`.
- No DST handling (fixed UTC offset).
