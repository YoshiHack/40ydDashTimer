#pragma once

// Safe default GPIOs for ESP32 DevKitC (you can change later)
static const int PIN_BEAM  = 25;  // phototransistor input
static const int PIN_LED   = 2;   // built-in LED on many boards
static const int PIN_BUZ   = -1;  // set later if you add buzzer
static const int PIN_BTN   = -1;  // optional reset button
