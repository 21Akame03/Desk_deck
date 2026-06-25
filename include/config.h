#pragma once
#include <Arduino.h>
#include "secrets.h"  // WIFI_SSID, WIFI_PASS, DIRIGERA_TOKEN (gitignored)

// =====================================================================
//  USER CONFIG  -  this is the only file you normally need to edit.
//  Credentials live in secrets.h (copy secrets.h.example to start).
// =====================================================================

// ---- WiFi -----------------------------------------------------------
// SSID/password come from secrets.h. WiFi is only used to fetch the
// date/time over NTP (daily rollover + streaks) and to reach the hub.
#define TZ_OFFSET_HOURS 2 // hours from UTC (no automatic DST)
#define NTP_SERVER "pool.ntp.org"

// ---- Your habits ----------------------------------------------------
// `goal` = how many increments count as "done" for the day (for streaks).
// Up to 4 habits are shown on the home grid. Reordering this list remaps
// saved data (data is stored by index), so add new habits at the end.
struct HabitDef {
  const char *name;
  uint16_t goal;
};
static const HabitDef HABITS[] = {
    {"Workout", 1},
    {"Read", 1},
    {"Water 2L", 1},
    {"Meditate", 1},
};
#define HABIT_COUNT ((int)(sizeof(HABITS) / sizeof(HABITS[0])))

// ---- Display --------------------------------------------------------
// CYD panels ship in two variants with opposite inversion. If the
// background shows up white/light instead of dark, flip this value.
#define DISPLAY_INVERT true

// ---- Touch controller pins (XPT2046 on the CYD) ---------------------
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

// ---- Touch calibration (raw ADC -> screen pixels) -------------------
// If taps land in the wrong place, uncomment the Serial.printf line in
// readTouch() (main.cpp), note the raw values at each corner, and tune
// these. Swapping/inverting may be needed depending on your panel batch.
#define TOUCH_X_MIN 200
#define TOUCH_X_MAX 3700
#define TOUCH_Y_MIN 240
#define TOUCH_Y_MAX 3800

// ---- Idle behaviour -------------------------------------------------
// Each IDLE_STEP_MS with no touch steps the UI "down" one level:
//   sub-screen -> Stella home -> screensaver (dimmed floating clock).
// A touch in the screensaver wakes back to a full-bright Stella home
// (that tap is swallowed); other taps interact normally.
#define BACKLIGHT_PIN  21          // TFT_BL on the CYD
#define IDLE_STEP_MS   20000UL     // idle time per step (20s)

// Screensaver brightness depends on time of day (PWM duty, 0-255).
// "Night" is NIGHT_START_HOUR..DAY_START_HOUR; daytime otherwise.
#define BL_DIM_DAY        26       // ~10% during the day
#define BL_DIM_NIGHT      13       // ~5% at night
#define NIGHT_START_HOUR  22       // 22:00
#define DAY_START_HOUR     7       // 07:00

// ---- IKEA Dirigera scenes ------------------------------------------
// Tapping a scene tile sends PUT /v1/scenes/{id}/trigger to the hub over
// the LAN (self-signed cert -> verification is skipped on the ESP32).
//
// DIRIGERA_IP : hub's LAN IP. Find it with:
//                 dns-sd -G v4 gw2-XXXXXXXXXXXX.local
//               (.local mDNS is unreliable from the ESP32, so use the IP.)
// DIRIGERA_TOKEN : long-lived bearer token, kept in secrets.h.
// SCENES[] : add/rename here, then re-flash (up to 4 shown on the grid).
//            Get ids with:  curl -k -H "Authorization: Bearer <token>" \
//                             https://<hub-ip>:8443/v1/scenes
#define DIRIGERA_IP    "192.168.0.234"   // hub LAN IP (gw2-0c4f5df140fa.local)

// `icon` picks the glyph + accent colour drawn on the tile (see
// scenes_screen.cpp). Pick whichever fits the scene.
enum SceneIcon { IC_WORK, IC_GAME, IC_HOME, IC_AWAY, IC_SUN, IC_MOON, IC_BULB };
struct SceneDef { const char* name; const char* id; uint8_t icon; };
static const SceneDef SCENES[] = {
    { "Work mode",   "4acb0e55-098c-401b-84e8-022cfc8d5f32", IC_WORK },
    { "Game mode",   "ccc1486d-ba3d-4e74-a5c6-669a20a4b10b", IC_GAME },
    { "Chill",       "7e92f342-dbf1-4fb1-a465-ce63d534b2a9", IC_BULB },
    { "Sleepy lights", "e6b5e502-8133-485e-ae5c-091d3f8cf472", IC_MOON },
};
#define SCENE_COUNT ((int)(sizeof(SCENES) / sizeof(SCENES[0])))
