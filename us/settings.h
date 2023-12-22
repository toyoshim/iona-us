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
  IT_NAMCO_TSS,
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
  uint16_t analog_input_mask;
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
    bool invert;
    bool on;
  } sequence[8];
  bool gear_sequence_support[2];
  uint8_t gear_control[2][12];
};

void settings_init(void);
void settings_poll(void);
struct settings* settings_get(void);
bool settings_test_pressed(void);
bool settings_service_pressed(void);
void settings_led_mode(uint8_t mode);
void settings_rapid_sync(void);
uint16_t settings_adjust_x(uint16_t x);
uint16_t settings_adjust_y(uint16_t y);

#endif  // __settings_h__