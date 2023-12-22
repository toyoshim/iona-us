// Copyright 2023 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "settings.h"

#include "ch559.h"
#include "gpio.h"
#include "io.h"
#include "timer3.h"

#include "controller.h"

static __code uint8_t* flash = (__code uint8_t*)0xf000;
static __code uint8_t* opt_flash = (__code uint8_t*)0xec00;

static struct settings settings;
static uint8_t current_setting;  // valid range is [0...5]
static uint16_t poll_msec = 0;
static uint8_t led_current_mode = 0;
enum adjust_index {
  AI_X_MIN = 0,
  AI_Y_MIN = 1,
  AI_X_MAX = 2,
  AI_Y_MAX = 3,

  AI_X_BASE = 0,
  AI_Y_BASE = 1,
  AI_X_SCALE = 2,
  AI_Y_SCALE = 3,
};
static uint16_t adjust[4] = {0x0000, 0x0000, 0x0400, 0x0400};

// NORMAL
//   ↓ Press TEST and SERVICE for 1sec
// CONFIG (Click Test increase the setting ID)
//   ↓ Click TEST
// NORMAL
enum state { S_NORMAL = 0, S_PRE_CONFIG, S_CONFIG, S_ADJUST };

static uint8_t state = S_NORMAL;
static uint8_t state_val = 0;

static bool test_pressed(void) {
  return digitalRead(4, 7) == LOW;
}

static bool service_pressed(void) {
  return digitalRead(4, 6) == LOW;
}

static void opt_flash_write(uint16_t addr, uint8_t val_l, uint8_t val_h) {
  ROM_ADDR_H = addr >> 8;
  ROM_ADDR_L = addr & 0xff;
  ROM_DATA_H = val_h;
  ROM_DATA_L = val_l;
  ROM_CTRL = 0x9a;
}

static void opt_flash_write_w(uint16_t addr, uint16_t val) {
  opt_flash_write(addr, val & 0xff, val >> 8);
}

static void store_opt(void) {
  SAFE_MOD = 0x55;
  SAFE_MOD = 0xaa;
  GLOBAL_CFG |= bCODE_WE;
  ROM_ADDR_H = 0xec;
  ROM_ADDR_L = 0x00;
  ROM_CTRL = 0xa6;

  if (ROM_STATUS == 0xc0) {
    opt_flash_write(0xec00, 'i', 'o');
    opt_flash_write(0xec02, 'n', '@');
    opt_flash_write(0xec04, 0, current_setting);
    opt_flash_write_w(0xec06, adjust[0]);
    opt_flash_write_w(0xec08, adjust[1]);
    opt_flash_write_w(0xec0a, adjust[2]);
    opt_flash_write_w(0xec0c, adjust[3]);
  }

  GLOBAL_CFG &= ~bCODE_WE;
  SAFE_MOD = 0x00;
}

static void apply(void) {
  uint16_t offset = 10;
  for (uint8_t id = 0; id < current_setting; ++id) {
    offset += 169;
  }
  const uint8_t core0 = flash[offset++];
  const uint8_t core1 = flash[offset++];
  const uint8_t core2 = flash[offset++];
  settings.id = core0 >> 5;
  if (settings.id > 3) {
    settings.id = 0;
  }
  settings.analog_input_count = ((core0 >> 2) & 7) << 1;
  settings.analog_input_width = (core0 & 3) ? 16 : 0;
  settings.analog_input_mask =
      (settings.analog_input_width == 1) ? 0xffff : 0xff00;
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
    settings.gear_sequence_support[p] = false;
    for (uint8_t i = 0; i < 6; ++i) {
      const uint8_t data = flash[offset++];
      uint8_t set1 = (data >> 4) & 15;
      uint8_t set2 = data & 15;
      settings.rapid_fire[p][i * 2 + 0] = (set1 < 8) ? set1 : 0;
      settings.rapid_fire[p][i * 2 + 1] = (set2 < 8) ? set2 : 0;
      settings.gear_control[p][i * 2 + 0] =
          (set1 == 8 || set1 == 9) ? (set1 - 7) : 0;
      settings.gear_control[p][i * 2 + 1] =
          (set2 == 8 || set2 == 9) ? (set2 - 7) : 0;
      if (set1 == 8 || set1 == 9 || set2 == 8 || set2 == 9) {
        settings.gear_sequence_support[p] = true;
      }
    }
  }
  settings.sequence[0].pattern = 0xff;
  settings.sequence[0].bit = 1;
  settings.sequence[0].mask = 0xff;
  settings.sequence[0].invert = false;
  settings.sequence[0].on = true;
  for (uint8_t i = 1; i < 8; ++i) {
    settings.sequence[i].pattern = flash[offset++];
    settings.sequence[i].bit = 1;
    settings.sequence[i].mask = (2 << (flash[offset] & 7)) - 1;
    settings.sequence[i].invert = flash[offset++] & 0x80;
    settings.sequence[i].on = true;
  }

  controller_reset();
}

void settings_init(void) {
  pinMode(4, 6, INPUT_PULLUP);
  pinMode(4, 7, INPUT_PULLUP);

  led_init(1, 5, LOW);

  if (flash[0] != 'I' || flash[1] != 'O' || flash[2] != 'N' ||
      flash[3] != 'C' || flash[4] != 1) {
    led_mode(L_BLINK_THREE_TIMES);
    for (;;) {
      led_poll();
    }
  }
  current_setting = 0;
  if (opt_flash[0] == 'i' && opt_flash[1] == 'o' && opt_flash[2] == 'n' &&
      opt_flash[3] == '@') {
    current_setting = opt_flash[5];
    adjust[0] = (opt_flash[7] << 8) | opt_flash[6];
    adjust[1] = (opt_flash[9] << 8) | opt_flash[8];
    adjust[2] = (opt_flash[11] << 8) | opt_flash[10];
    adjust[3] = (opt_flash[13] << 8) | opt_flash[12];
  }
  apply();

  settings_led_mode(L_BLINK);
}

void settings_poll(void) {
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
          state = S_PRE_CONFIG;
          led_mode(L_OFF);
        }
      } else {
        state_val = 0;
      }
      break;
    case S_PRE_CONFIG: {
      if (service_pressed() && test_pressed()) {
        state_val++;
        if (state_val >= 180) {
          state = S_ADJUST;
          state_val = 0;
          adjust[AI_X_MIN] = adjust[AI_Y_MIN] = 0xffff;
          adjust[AI_X_MAX] = adjust[AI_Y_MAX] = 0x0000;
          led_mode(L_BLINK);
        }
      } else {
        state_val = 3;
        state = S_CONFIG;
      }
    } break;
    case S_CONFIG: {
      bool service = state_val & 2;
      bool test = state_val & 1;
      bool cur_service = service_pressed();
      bool cur_test = test_pressed();
      if (!service && cur_service) {
        state = S_NORMAL;
        state_val = 0;
        led_mode(led_current_mode);
        store_opt();
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
    case S_ADJUST: {
      uint8_t data = controller_data(0, 0, 0) & 0x03;
      data |= controller_data(0, 1, 0);
      state_val =
          (state_val & 0xf0) | ((state_val << 1) & 0x0f) | (data ? 1 : 0);
      // Take 4 corner positions on triggers.
      if ((state_val & 0x0f) == 1) {
        uint16_t x = controller_screen(0, 0);
        if (x < adjust[AI_X_MIN]) {
          adjust[AI_X_MIN] = x;
        }
        if (adjust[AI_X_MAX] < x) {
          adjust[AI_X_MAX] = x;
        }
        uint16_t y = controller_screen(0, 1);
        if (y < adjust[AI_Y_MIN]) {
          adjust[AI_Y_MIN] = y;
        }
        if (adjust[AI_Y_MAX] < y) {
          adjust[AI_Y_MAX] = y;
        }
        state_val += 0x10;
        if (state_val >= 0x40) {
          // Obtains base and scale.
          state = S_NORMAL;
          uint32_t scale;
          scale = (uint32_t)640 << 16;
          scale /= (adjust[AI_X_MAX] - adjust[AI_X_MIN]);
          adjust[AI_X_SCALE] = scale;
          scale = (uint32_t)960 << 16;
          scale /= (adjust[AI_Y_MAX] - adjust[AI_Y_MIN]);
          adjust[AI_Y_SCALE] = scale;
          state_val = 0;
          store_opt();
          led_mode(led_current_mode);
        }
      }
    } break;
  }
}

struct settings* settings_get(void) {
  return &settings;
}

bool settings_test_pressed(void) {
  return (state == S_NORMAL) && test_pressed();
}

bool settings_service_pressed(void) {
  return (state == S_NORMAL) && service_pressed();
}

void settings_led_mode(uint8_t mode) {
  led_current_mode = mode;
  if (state == S_NORMAL) {
    led_mode(led_current_mode);
  }
}

void settings_rapid_sync(void) {
  for (uint8_t i = 1; i < 8; ++i) {
    settings.sequence[i].bit <<= 1;
    if (0 == (settings.sequence[i].bit & settings.sequence[i].mask)) {
      settings.sequence[i].bit = 1;
    }
    settings.sequence[i].on =
        settings.sequence[i].pattern & settings.sequence[i].bit;
  }
}

uint16_t settings_adjust_x(uint16_t x) {
  if (x < adjust[AI_X_BASE]) {
    return 0;
  }
  x -= adjust[AI_X_BASE];
  uint32_t ax = (uint32_t)x * adjust[AI_X_SCALE];
  if (ax > ((uint32_t)639 << 16)) {
    return 639;
  }
  return ax >> 16;
}

uint16_t settings_adjust_y(uint16_t y) {
  if (y < adjust[AI_Y_BASE]) {
    return 0;
  }
  y -= adjust[AI_Y_BASE];
  uint32_t ay = (uint32_t)y * adjust[AI_Y_SCALE];
  if (ay > ((uint32_t)959 << 16)) {
    return 959;
  }
  return ay >> 16;
}