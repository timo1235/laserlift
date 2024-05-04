#pragma once

// WiFi Connection - Should not be used. ESPNow wont work if the WiFi has not channel 1
// #define WIFI_SSID "asdf"
// #define WIFI_PASS "1234567890"

// Enable debug output over the serial monitor with a baud of 115200
// Default: true
#define PROJECT_DEBUG true

// versioning
#define DEFAULT_TASK_PRIORITY 1
#define DEFAULT_TASK_CPU 1

#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
// Display Library
#include <U8g2lib.h>
// The Rotary Encoder
#include "RotaryEncoder.h"
// Stepper library
#include "FastAccelStepper.h"
#include <pinmap.h>

// DEBUG MACRO
#if PROJECT_DEBUG == true                               // Macros are usually in all capital letters.
    #define DPRINT(...) Serial.print(__VA_ARGS__)       // DPRINT is a macro, debug print
    #define DPRINTLN(...) Serial.println(__VA_ARGS__)   // DPRINTLN is a macro, debug print with new line
#else
    #define DPRINT(...)     // now defines a blank line
    #define DPRINTLN(...)   // now defines a blank line
#endif
