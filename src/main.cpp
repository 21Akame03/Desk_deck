#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#include "config.h"
#include "theme.h"
#include "time_util.h"
#include "habit_store.h"
#include "screen_manager.h"
#include "home_screen.h"
#include "tracker_screen.h"
#include "detail_screen.h"
#include "scenes_screen.h"
#include "screensaver_screen.h"
#include "app.h"

// --- Global app context (declared extern in app.h) -------------------
TFT_eSPI      tft = TFT_eSPI();
HabitStore    store;
ScreenManager screens;
int           g_selectedHabit = 0;

static HomeScreen        homeScreen;
static TrackerScreen     trackerScreen;
static DetailScreen      detailScreen;
static ScenesScreen      scenesScreen;
static ScreensaverScreen screensaverScreen;
Screen* gHomeScreen    = &homeScreen;
Screen* gTrackerScreen = &trackerScreen;
Screen* gDetailScreen  = &detailScreen;
Screen* gScenesScreen  = &scenesScreen;
Screen* gScreensaver   = &screensaverScreen;

// --- Touch (XPT2046 lives on its own SPI bus on the CYD) -------------
static SPIClass            touchSPI(VSPI);
static XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

// --- Input edge detection + tick pacing ------------------------------
static bool     wasDown = false;
static uint32_t lastTick = 0;

// --- Backlight + idle state ------------------------------------------
static const int BL_CHANNEL = 0;            // LEDC channel (core 2.x)
enum BLState { BL_FULL, BL_DIM };
static BLState  blState = BL_FULL;
static uint32_t lastActivity = 0;           // millis() of last touch
static bool     inScreensaver = false;

static inline void blWrite(uint8_t duty) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(BACKLIGHT_PIN, duty);
#else
  ledcWrite(BL_CHANNEL, duty);
#endif
}

// Screensaver brightness: lower at night. Defaults to daytime if the
// clock isn't synced yet.
static uint8_t dimDuty() {
  int hour = 12;
  if (timeReady()) {
    time_t t = time(nullptr);
    struct tm tm;
    localtime_r(&t, &tm);
    hour = tm.tm_hour;
  }
  bool night = (hour >= NIGHT_START_HOUR) || (hour < DAY_START_HOUR);
  return night ? BL_DIM_NIGHT : BL_DIM_DAY;
}

static void setBacklight(BLState s) {
  blWrite(s == BL_FULL ? 255 : dimDuty());
  blState = s;
}

static void enterScreensaver() {
  inScreensaver = true;
  setBacklight(BL_DIM);
  screens.show(gScreensaver);
}

static void exitScreensaver() {
  inScreensaver = false;
  setBacklight(BL_FULL);
  screens.show(gHomeScreen);
}

static void backlightBegin() {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttach(BACKLIGHT_PIN, 5000, 8);
#else
  ledcSetup(BL_CHANNEL, 5000, 8);
  ledcAttachPin(BACKLIGHT_PIN, BL_CHANNEL);
#endif
  setBacklight(BL_FULL);
}

static void bootMessage(const char* msg) {
  tft.fillScreen(COLOR_BG);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_TEXT, COLOR_BG);
  tft.drawString("Stella", 160, 96, 4);
  tft.setTextColor(COLOR_MUTED, COLOR_BG);
  tft.drawString(msg, 160, 136, 2);
}

// Read + map a touch to screen pixels. Returns false when not touched.
static bool readTouch(int& x, int& y) {
  if (!touchscreen.touched()) return false;
  TS_Point p = touchscreen.getPoint();
  x = map(p.x, TOUCH_X_MIN, TOUCH_X_MAX, 0, 320);
  y = map(p.y, TOUCH_Y_MIN, TOUCH_Y_MAX, 0, 240);
  x = constrain(x, 0, 319);
  y = constrain(y, 0, 239);
  // Calibration helper: uncomment to see raw vs mapped values.
  // Serial.printf("raw %d,%d -> %d,%d\n", p.x, p.y, x, y);
  return true;
}

void setup() {
  Serial.begin(115200);

  tft.init();
  tft.setRotation(1);            // landscape, 320x240
  tft.invertDisplay(DISPLAY_INVERT);  // fix panels that render inverted
  backlightBegin();             // take over the backlight pin for dimming

  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchSPI);
  touchscreen.setRotation(1);

  // WiFi + NTP are only needed for dates/streaks; failure is non-fatal.
  bootMessage("Connecting WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 8000) delay(150);

  if (WiFi.status() == WL_CONNECTED) {
    bootMessage("Syncing time...");
    configTime(TZ_OFFSET_HOURS * 3600, 0, NTP_SERVER);
    uint32_t t1 = millis();
    while (!timeReady() && millis() - t1 < 6000) delay(150);
  } else {
    bootMessage("No WiFi - offline mode");
    delay(1200);
  }

  store.begin();
  screens.show(gHomeScreen);
  lastActivity = millis();
}

void loop() {
  uint32_t now = millis();

  // Idle steps down one level every IDLE_STEP_MS:
  //   sub-screen -> home (still bright) -> screensaver (dim).
  if (!inScreensaver && now - lastActivity >= IDLE_STEP_MS) {
    if (screens.current() != gHomeScreen) {
      screens.show(gHomeScreen);
      lastActivity = now;         // home gets its own full idle window
    } else {
      enterScreensaver();
    }
  }

  int x, y;
  bool down = readTouch(x, y);
  if (down && !wasDown) {         // act on press edge only
    wasDown = true;
    lastActivity = now;
    if (inScreensaver) {
      exitScreensaver();          // wake to home, swallow this tap
    } else {
      screens.touch(x, y);
    }
  } else if (!down && wasDown) {
    wasDown = false;
  }

  // Tick fast while the screensaver animates, ~1Hz otherwise.
  uint32_t tickEvery = inScreensaver ? 40 : 1000;
  if (now - lastTick > tickEvery) {
    lastTick = now;
    screens.tick();
    // Track the day/night brightness boundary while dimmed.
    if (inScreensaver) setBacklight(BL_DIM);
  }

  delay(10);
}
