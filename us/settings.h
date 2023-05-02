// Copyright 2023 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#ifndef __settings_h__
#define __settings_h__

#include <stdbool.h>
#include <stdint.h>

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
  bool data_signal_adjustment;
  bool jvs_dash_support;
};

bool settings_init();
void settings_poll();
struct settings* settings_get();

#endif  // __settings_h__