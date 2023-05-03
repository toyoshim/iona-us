// Copyright 2023 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#ifndef __settings_h__
#define __settings_h__

#include <stdbool.h>
#include <stdint.h>

#include "led.h"

enum IdType {
  IT_SEGA,
  IT_NAMCO_JYU,
  IT_NAMCO_NAJV,
};

enum AnalogType {
  AT_NONE,
  AT_DIGITAL,
  AT_ANALOG,
  AT_ROTARY,
  AT_SCREEN,
};

// Decoded struct of https://github.com/toyoshim/iona-us/wiki/v2-settings
struct settings {
  uint8_t id;
  uint8_t analog_input_count;
  uint8_t analog_input_width;
  uint8_t rotary_input_count;
  uint8_t screen_position_count;
  uint8_t screen_position_width;
  uint8_t analog_output;
  uint8_t character_display_width;
  uint8_t character_display_height;
  bool jvs_dash_support;
  bool data_signal_adjustment;
  uint8_t analog_type[2][6];
  uint8_t analog_index[2][6];
  bool analog_polarity[2][6];
  struct {
    uint8_t data[4];
  } digital_map[2][16];
  uint8_t rapid_fire[2][12];
  struct {
    uint8_t pattern;
    uint8_t bit;
    uint8_t mask;
    bool on;
  } sequence[8];
};

void settings_init();
void settings_poll();
struct settings* settings_get();
bool settings_test_pressed();
bool settings_service_pressed();
void settings_led_mode(uint8_t mode);
void settings_rapid_sync();

#endif  // __settings_h__