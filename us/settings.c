// Copyright 2023 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "settings.h"

#include "ch559.h"
#include "timer3.h"

static __code uint8_t* flash = (__code uint8_t*)0xf000;

static struct settings settings;
static uint8_t current_setting;  // valid range is [0...5]
static uint16_t poll_msec = 0;
static uint8_t led_current_mode = 0;

// NORMAL
//   ↓ Press TEST and SERVICE for 1sec
// CONFIG (Click Test increase the setting ID)
//   ↓ Click TEST
// NORMAL
enum state { S_NORMAL = 0, S_CONFIG };

static uint8_t state = S_NORMAL;
static uint8_t state_val = 0;

static bool test_pressed() {
  return digitalRead(4, 7) == LOW;
}

static bool service_pressed() {
  return digitalRead(4, 6) == LOW;
}

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
  settings.rotary_input_count = (core1 & 0x20) ? 2 : 0;
  settings.screen_position_count = (core1 >> 2) & 7;
  if (settings.screen_position_count > 2) {
    settings.screen_position_count = 0;
  }
  settings.screen_position_width = (core1 & 1) ? 16 : 10;
  settings.analog_output = (core2 & 0x40) ? 2 : 0;
  settings.character_display_width = (core2 & 0x08) ? 16 : 0;
  settings.character_display_height = (core2 & 0x08) ? 1 : 0;
  settings.jvs_dash_support = core2 & 4;
  settings.data_signal_adjustment = core2 & 2;

  for (uint8_t p = 0; p < 2; ++p) {
    for (uint8_t i = 0; i < 6; ++i) {
      const uint8_t data = flash[offset++];
      const uint8_t type = data >> 4;
      settings.analog_type[p][i] = (type < 5) ? type : 0;
      settings.analog_index[p][i] = data & 7;
      settings.analog_polarity[p][i] = data & 8;
    }
  }

  for (uint8_t p = 0; p < 2; ++p) {
    for (uint8_t i = 0; i < 16; ++i) {
      for (uint8_t j = 0; j < 4; ++j) {
        settings.digital_map[p][i].data[j] = flash[offset++];
      }
    }
  }

  for (uint8_t p = 0; p < 2; ++p) {
    for (uint8_t i = 0; i < 6; ++i) {
      const uint8_t data = flash[offset++];
      settings.rapid_fire[p][i * 2 + 0] = (data >> 4) & 7;
      settings.rapid_fire[p][i * 2 + 1] = data & 7;
    }
  }
  settings.sequence[0].pattern = 0xff;
  settings.sequence[0].bit = 1;
  settings.sequence[0].mask = 0xff;
  settings.sequence[0].on = true;
  for (uint8_t i = 1; i < 8; ++i) {
    settings.sequence[i].pattern = flash[offset++];
    settings.sequence[i].bit = 1;
    settings.sequence[i].mask = (2 << (flash[offset++] & 7)) - 1;
    settings.sequence[i].on = true;
  }
}

void settings_init() {
  led_init(1, 5, LOW);

  if (flash[0] != 'I' || flash[1] != 'O' || flash[2] != 'N' ||
      flash[3] != 'C' || flash[4] != 1) {
    led_mode(L_BLINK_THREE_TIMES);
    for (;;) {
      led_poll();
    }
  }
  current_setting = 0;
  apply();

  settings_led_mode(L_BLINK);
}

void settings_poll() {
  led_poll();
  if (timer3_tick_msec_between(poll_msec, poll_msec + 17)) {
    return;
  }
  poll_msec = timer3_tick_msec();
  switch (state) {
    case S_NORMAL:
      if (service_pressed() && test_pressed()) {
        state_val++;
        if (state_val >= 60) {
          state = S_CONFIG;
          state_val = 3;
          led_mode(L_OFF);
        }
      } else {
        state_val = 0;
      }
      break;
    case S_CONFIG: {
      bool service = state_val & 2;
      bool test = state_val & 1;
      bool cur_service = service_pressed();
      bool cur_test = test_pressed();
      if (!service && cur_service) {
        state = S_NORMAL;
        state_val = 0;
        led_mode(led_current_mode);
      } else if (!test && cur_test) {
        current_setting++;
        if (current_setting > 5) {
          current_setting = 0;
        }
        apply();
        led_oneshot(current_setting);
      }
      state_val = (cur_service ? 2 : 0) | (cur_test ? 1 : 0);
    } break;
  }
}

struct settings* settings_get() {
  return &settings;
}

bool settings_test_pressed() {
  return (state == S_NORMAL) && test_pressed();
}

bool settings_service_pressed() {
  return (state == S_NORMAL) && service_pressed();
}

void settings_led_mode(uint8_t mode) {
  led_current_mode = mode;
  if (state == S_NORMAL) {
    led_mode(led_current_mode);
  }
}

void settings_rapid_sync() {
  // TODO
}