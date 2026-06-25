#pragma once
#include <TFT_eSPI.h>
#include "habit_store.h"
#include "screen.h"
#include "screen_manager.h"

// Global app context, defined in main.cpp and shared by all screens.
extern TFT_eSPI      tft;
extern HabitStore    store;
extern ScreenManager screens;

extern int g_selectedHabit;     // habit shown on the detail screen

// Navigation targets as base pointers (avoids screen header cross-includes).
extern Screen* gHomeScreen;     // Stella landing
extern Screen* gTrackerScreen;  // Daily Tracker grid
extern Screen* gDetailScreen;   // single-habit detail
extern Screen* gScenesScreen;   // IKEA Dirigera scenes
extern Screen* gScreensaver;    // dimmed floating clock
