// Copyright 2023 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "settings.h"

#include "ch559.h"

static __code uint8_t* flash = (__code uint8_t*)0xf000;

static struct settings settings;
static uint8_t current_setting;  // valid range is [0...5]

static void apply() {
  uint16_t offset = 10;
  for (uint8_t i = 0; i < current_setting; ++i) {
    offset += 169;
  }
  const uint8_t core0 = flash[offset++];
  const uint8_t core1 = flash[offset++];
  const uint8_t core2 = flash[offset++];
  settings.id = core0 >> 5;
  if (settings.id > 2) {
    settings.id = 0;
  }
  settings.analog_input_count = ((core0 >> 2) & 7) << 1;
  settings.analog_input_width = (core0 & 3) ? 16 : 0;
  settings.rotary_input_count = core1 >> 5;
  settings.screen_position_count = (core1 >> 2) & 7;
  settings.screen_position_width = core1 & 3;
  settings.analog_output = (core2 >> 6) & 3;
  settings.character_display_width = (core2 >> 3) & 7;
  settings.character_display_height;
  settings.jvs_dash_support = core2 & 4;
  settings.data_signal_adjustment = core2 & 2;
}

bool settings_init() {
  if (flash[0] != 'I' || flash[1] != 'O' || flash[2] != 'N' ||
      flash[3] != 'C' || flash[4] != 1) {
    return false;
  }
  current_setting = 0;
  apply();
  return true;
}

void settings_poll() {}

struct settings* settings_get() {
  return &settings;
}