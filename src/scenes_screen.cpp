#include "scenes_screen.h"
#include "app.h"
#include "theme.h"
#include "ui.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

// Large back button + hit area (matches the other screens).
static const int BACK_X = 6, BACK_Y = 2, BACK_W = 100, BACK_H = 30;
static const int BACK_HIT_X = 0, BACK_HIT_Y = 0, BACK_HIT_W = 120, BACK_HIT_H = 40;

int ScenesScreen::shownCount() { return SCENE_COUNT < 4 ? SCENE_COUNT : 4; }

void ScenesScreen::tileRect(int i, int& x, int& y, int& w, int& h) {
  int col = i % 2, row = i / 2;
  w = 148;
  h = 91;
  x = 8 + col * 156;
  y = 42 + row * 99;
}

void ScenesScreen::drawHeader() {
  tft.fillRect(0, 0, 320, HEADER_H, COLOR_HEADER);
  tft.fillRoundRect(BACK_X, BACK_Y, BACK_W, BACK_H, 6, COLOR_BADGE);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_TEXT, COLOR_BADGE);
  tft.drawString("< Back", BACK_X + BACK_W / 2, BACK_Y + BACK_H / 2, 4);
  tft.setTextColor(COLOR_TEXT, COLOR_HEADER);
  tft.drawString("Scenes", 218, HEADER_H / 2, 4);
}

// Accent colour per scene icon.
static uint16_t iconColor(uint8_t k) {
  switch (k) {
    case IC_GAME: return COLOR_MAUVE;
    case IC_HOME: return COLOR_GREEN;
    case IC_AWAY: return COLOR_RED;
    case IC_SUN:  return COLOR_YELLOW;
    case IC_MOON: return COLOR_TEAL;
    case IC_BULB: return COLOR_PEACH;
    case IC_WORK:
    default:      return COLOR_ACCENT;
  }
}

// Draw a simple glyph (dark, on the pastel disc) centred at (cx,cy).
// `disc` is the disc colour, used to carve negative space.
static void drawIcon(uint8_t k, int cx, int cy, uint16_t disc) {
  uint16_t fg = COLOR_BG;
  switch (k) {
    case IC_SUN:
      tft.fillCircle(cx, cy, 5, fg);
      for (int a = 0; a < 360; a += 45) {
        float r = a * PI / 180.0f;
        tft.drawLine(cx + cosf(r) * 8, cy + sinf(r) * 8,
                     cx + cosf(r) * 11, cy + sinf(r) * 11, fg);
      }
      break;
    case IC_MOON:
      tft.fillCircle(cx, cy, 8, fg);
      tft.fillCircle(cx + 4, cy - 3, 7, disc);  // carve the crescent
      break;
    case IC_HOME:
      tft.fillTriangle(cx, cy - 9, cx - 9, cy - 1, cx + 9, cy - 1, fg);  // roof
      tft.fillRect(cx - 6, cy - 1, 12, 9, fg);                           // body
      tft.fillRect(cx - 2, cy + 2, 4, 6, disc);                          // door
      break;
    case IC_AWAY:  // hollow house = nobody home
      tft.drawTriangle(cx, cy - 9, cx - 9, cy - 1, cx + 9, cy - 1, fg);
      tft.drawRect(cx - 6, cy - 1, 12, 9, fg);
      break;
    case IC_GAME:
      tft.fillRoundRect(cx - 10, cy - 5, 20, 11, 5, fg);
      tft.fillCircle(cx - 5, cy, 2, disc);
      tft.fillCircle(cx + 5, cy, 2, disc);
      break;
    case IC_BULB:
      tft.fillCircle(cx, cy - 2, 7, fg);
      tft.fillRect(cx - 3, cy + 4, 6, 4, fg);
      break;
    case IC_WORK:
    default:  // book
      tft.fillRect(cx - 9, cy - 7, 8, 14, fg);
      tft.fillRect(cx + 1, cy - 7, 8, 14, fg);
      tft.drawFastVLine(cx, cy - 7, 14, disc);
      break;
  }
}

void ScenesScreen::drawTile(int i, const char* status) {
  int x, y, w, h;
  tileRect(i, x, y, w, h);
  uint16_t bg = status ? COLOR_BADGE : COLOR_CARD;  // highlight while running
  tft.fillRoundRect(x, y, w, h, 8, bg);

  // Coloured icon disc
  uint16_t disc = iconColor(SCENES[i].icon);
  int cx = x + w / 2, cy = y + 26;
  tft.fillCircle(cx, cy, 16, disc);
  drawIcon(SCENES[i].icon, cx, cy, disc);

  // Scene name
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_TEXT, bg);
  tft.drawString(SCENES[i].name, cx, y + 52, 2);

  // Status / hint line
  tft.setTextColor(status ? disc : COLOR_MUTED, bg);
  tft.drawString(status ? status : "Tap to run", cx, y + 72, 2);
}

void ScenesScreen::enter() {
  tft.fillScreen(COLOR_BG);
  drawHeader();
  for (int i = 0; i < shownCount(); i++) drawTile(i, nullptr);
}

// Fire a scene on the hub. Blocks ~1-3s for the TLS handshake + request.
// ponytail: self-signed cert -> setInsecure(); fine on a trusted LAN.
static bool triggerScene(const char* id) {
  if (WiFi.status() != WL_CONNECTED) return false;
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  String url = String("https://") + DIRIGERA_IP + ":8443/v1/scenes/" + id + "/trigger";
  if (!http.begin(client, url)) return false;
  http.addHeader("Authorization", String("Bearer ") + DIRIGERA_TOKEN);
  int code = http.POST("");          // POST /scenes/{id}/trigger, empty body
  http.end();
  return code >= 200 && code < 300;  // hub returns 202 Accepted on success
}

void ScenesScreen::handleTouch(int tx, int ty) {
  if (ui::inRect(tx, ty, BACK_HIT_X, BACK_HIT_Y, BACK_HIT_W, BACK_HIT_H)) {
    screens.show(gHomeScreen);
    return;
  }
  for (int i = 0; i < shownCount(); i++) {
    int x, y, w, h;
    tileRect(i, x, y, w, h);
    if (ui::inRect(tx, ty, x, y, w, h)) {
      drawTile(i, "...");                          // immediate feedback
      bool ok = triggerScene(SCENES[i].id);
      drawTile(i, ok ? "Done" : "Failed");
      delay(700);
      drawTile(i, nullptr);                        // back to idle look
      return;
    }
  }
}
