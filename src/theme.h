#pragma once
#include <Arduino.h>

// Pack 8-bit RGB into a 16-bit (565) colour.
#define RGB565(r, g, b) ((uint16_t)((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3)))

// Shared layout
#define HEADER_H 34

// Palette - Catppuccin Mocha (cohesive soft-dark theme)
static const uint16_t COLOR_BG        = RGB565(24, 24, 37);    // Mantle
static const uint16_t COLOR_HEADER    = RGB565(30, 30, 46);    // Base
static const uint16_t COLOR_CARD      = RGB565(49, 50, 68);    // Surface0
static const uint16_t COLOR_CARD_DONE = RGB565(44, 58, 53);    // green-tinted
static const uint16_t COLOR_BADGE     = RGB565(69, 71, 90);    // Surface1
static const uint16_t COLOR_BAR       = RGB565(88, 91, 112);   // Surface2
static const uint16_t COLOR_ACCENT    = RGB565(137, 180, 250); // Blue
static const uint16_t COLOR_GREEN     = RGB565(166, 227, 161); // Green
static const uint16_t COLOR_TEXT      = RGB565(205, 214, 244); // Text
static const uint16_t COLOR_MUTED     = RGB565(147, 153, 178); // Overlay2
// Accent palette (Catppuccin Mocha) for per-scene icons / tiles.
static const uint16_t COLOR_RED       = RGB565(243, 139, 168); // Red
static const uint16_t COLOR_PEACH     = RGB565(250, 179, 135); // Peach
static const uint16_t COLOR_YELLOW    = RGB565(249, 226, 175); // Yellow
static const uint16_t COLOR_TEAL      = RGB565(148, 226, 213); // Teal
static const uint16_t COLOR_MAUVE     = RGB565(203, 166, 247); // Mauve
static const uint16_t COLOR_PINK      = RGB565(245, 194, 231); // Pink
