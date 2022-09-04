// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "settings.h"

#include "ch559.h"
#include "flash.h"

#include "controller.h"

static uint8_t mode = S_NORMAL;
static uint16_t mode_step = 0;
static uint16_t mode_data = 0;
static uint8_t mode_player = 0;
static uint16_t poll_msec = 0;
static uint16_t poll_interval = 17;
static bool last_buttons[2] = {false, false};
static uint8_t client_led_mode = L_ON;
static uint8_t rapid_step[2] = {0, 0};
static uint8_t analog_base[16] = {0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0};

// sizeof(struct settings) should be <= 1020
struct settings {
  uint8_t player_setting[2];
  struct player_setting settings[10];  // 32 * 10
  struct common_setting {
    uint8_t options;
    uint8_t reserved;
    uint8_t analog[6];
  } common[10];  // 8 * 10
} settings;

void reset() {
  for (uint8_t i = 0; i < 2; ++i)
    settings.player_setting[i] = 0;
  for (uint8_t i = 0; i < 10; ++i) {
    settings.settings[i].magic = i;
    settings.settings[i].speed = 1;
    settings.settings[i].rapid_fire = 0;
    for (uint8_t b = 0; b < 12; ++b)
      settings.settings[i].button_masks[b] = (uint16_t)1 << b;
    for (uint8_t b = 0; b < 2; ++b)
      settings.settings[i].padding[b] = 0;
    settings.common[i].options = 0;
    settings.common[i].reserved = 0;
    for (uint8_t b = 0; b < 6; ++b)
      settings.common[i].analog[b] = b;
  }
  settings_save();
}

static void mode_layout() {
  uint16_t buttons = controller_raw(mode_player);
  if (mode_data && !buttons) {
    if (mode_step < 12) {
      uint8_t setting = settings.player_setting[mode_player];
      settings.settings[setting].button_masks[mode_step] = mode_data;
      led_mode(L_FASTER_BLINK);
    }
    mode_data = 0;
    mode_step++;
  }
  if (mode_step < 12)
    mode_data |= buttons;
  if (mode_data)
    led_mode(L_OFF);
}

static void quit_layout() {
  if (mode_step == 0)
    return;
  for (uint8_t i = mode_step; i < 12; ++i)
    settings.settings[settings.player_setting[mode_player]].button_masks[i] = 0;
}

static void mode_rapid() {
  uint16_t buttons = controller_raw(mode_player);
  if (mode_data && !buttons) {
    uint8_t setting = settings.player_setting[mode_player];
    settings.settings[setting].rapid_fire = mode_data;
    led_mode(L_FAST_BLINK);
    mode_data = 0;
  }
  mode_data |= buttons;
  if (mode_data)
    led_mode(L_OFF);
}

static void mode_speed() {
  uint16_t buttons = ((uint16_t)controller_jvs(1 + mode_player * 2, 0) << 8) |
                     controller_jvs(1 + mode_player * 2 + 1, 0);
  uint8_t setting = settings.player_setting[mode_player];
  if (buttons & 0x0200) {
    settings.settings[setting].speed = 1;  // 30
  } else if (buttons & 0x0100) {
    settings.settings[setting].speed = 2;  // 20
  } else if (buttons & 0x0080) {
    settings.settings[setting].speed = 3;  // 15
  } else if (buttons & 0x0040) {
    settings.settings[setting].speed = 4;  // 12
  } else if (buttons & 0x0020) {
    settings.settings[setting].speed = 5;  // 10
  } else if (buttons & 0x0010) {
    settings.settings[setting].speed = 6;  // 8.5
  }
  led_mode((buttons & 0x03c0) ? L_OFF : L_BLINK);
}

static void mode_select() {
  uint16_t buttons = ((uint16_t)controller_jvs(1 + mode_player * 2, 0) << 8) |
                     controller_jvs(1 + mode_player * 2 + 1, 0);
  if (buttons & 0x0200) {
    settings.player_setting[mode_player] = 0;
  } else if (buttons & 0x0100) {
    settings.player_setting[mode_player] = 1;
  } else if (buttons & 0x0080) {
    settings.player_setting[mode_player] = 2;
  } else if (buttons & 0x0040) {
    settings.player_setting[mode_player] = 3;
  } else if (buttons & 0x0020) {
    settings.player_setting[mode_player] = 4;
  } else if (buttons & 0x0010) {
    settings.player_setting[mode_player] = 5;
  } else if (buttons & 0x0008) {
    settings.player_setting[mode_player] = 6;
  } else if (buttons & 0x0004) {
    settings.player_setting[mode_player] = 7;
  } else if (buttons & 0x0002) {
    settings.player_setting[mode_player] = 8;
  } else if (buttons & 0x0001) {
    settings.player_setting[mode_player] = 9;
  }
  led_mode((buttons & 0x03c0) ? L_ON : L_OFF);
}

static void mode_analog() {
  uint8_t data = mode_data;
  mode_data = 0;
  int8_t diff = 0;
  for (uint8_t i = 0; i < 16; ++i) {
    uint8_t analog_data = controller_analog(i) >> 9;
    int8_t diff = analog_data - analog_base[i];
    if (diff < -16) {
      mode_data = i + 0x81;
      break;
    } else if (diff > 16) {
      mode_data = i + 0x01;
      break;
    }
  }
  if (mode_step < 6 && mode_data == 0 && data != mode_data) {
    led_oneshot(L_PULSE_TWICE);
    settings.common[settings.player_setting[0]].analog[mode_step] = data - 1;
    mode_step++;
  }
}

static void quit_option() {
  uint16_t buttons =
      ((uint16_t)controller_jvs(1, 0) << 8) | controller_jvs(2, 0);
  settings.common[settings.player_setting[0]].options = buttons >> 2;
}

static void quit_reset() {
  reset();
  led_oneshot(L_PULSE_THREE_TIMES);
}

static void slow_poll() {
  bool button0 = controller_button(B_TEST);
  bool button1 = controller_button(B_SERVICE);
  bool changed = false;
  uint8_t next_mode = S_NORMAL;
  switch (mode) {
    case S_NORMAL: {
      changed = button0 && button1;
      next_mode = S_WAIT;
      mode_step = 0;
      break;
    }
    case S_WAIT:
      changed = !button0 && !button1;
      if (mode_step < 300)
        mode_step++;
      if (30 < mode_step && mode_step < 300) {
        // 0.5 sec ~ 5 sec
        next_mode = S_LAYOUT;
        led_mode(L_FASTER_BLINK);
      } else {
        led_mode(client_led_mode);
      }
      break;
    case S_LAYOUT:
    case S_RAPID:
    case S_SPEED:
    case S_SELECT:
    case S_ANALOG:
    case S_OPTION: {
      bool change_to_next = last_buttons[0] && !button0;
      bool change_to_select = (mode < S_SELECT) && last_buttons[1] && !button1;
      bool change_to_reset = (mode < S_SELECT) && button0 && button1;
      changed = change_to_next | change_to_select | change_to_reset;
      next_mode = mode + 1;
      if (change_to_select)
        next_mode = S_SELECT;
      else if (change_to_reset)
        next_mode = S_RESET;
      else if (next_mode == (S_SPEED + 1) || (next_mode == (S_OPTION + 1)))
        next_mode = S_NORMAL;
      break;
    }
    case S_RESET:
      changed = !button0 && !button1;
      next_mode = S_NORMAL;
      break;
  }
  last_buttons[0] = button0;
  last_buttons[1] = button1;
  if (changed) {
    switch (mode) {
      case S_LAYOUT:
        quit_layout();
        settings_save();
        break;
      case S_RAPID:
      case S_SPEED:
      case S_SELECT:
      case S_ANALOG:
        settings_save();
        break;
      case S_OPTION:
        quit_option();
        settings_save();
        break;
      case S_RESET:
        quit_reset();
        break;
    }
    mode = next_mode;
    mode_step = 0;
    mode_data = 0;
    switch (mode) {
      case S_NORMAL:
        led_mode(client_led_mode);
        break;
      case S_LAYOUT:
        mode_player = 0xff;
        led_mode(L_FASTER_BLINK);
        break;
      case S_RAPID:
        led_mode(L_FAST_BLINK);
        break;
      case S_SPEED:
        led_mode(L_BLINK);
        break;
      case S_SELECT:
        led_mode(L_OFF);
        break;
      case S_ANALOG:
        led_mode(L_FASTER_BLINK);
        for (uint8_t i = 0; i < 16; ++i)
          analog_base[i] = controller_analog(i) >> 9;
        break;
      case S_OPTION:
        led_mode(L_FAST_BLINK);
        break;
    }
  }
  if (mode_player == 0xff && mode != S_ANALOG) {
    uint16_t player1 = controller_raw(0);
    uint16_t player2 = controller_raw(1);
    if (!player1 && !player2)
      return;
    mode_player = player1 ? 0 : 1;
  }
  switch (mode) {
    case S_LAYOUT:
      mode_layout();
      break;
    case S_RAPID:
      mode_rapid();
      break;
    case S_SPEED:
      mode_speed();
      break;
    case S_SELECT:
      mode_select();
      break;
    case S_ANALOG:
      mode_analog();
      break;
  }
}

void settings_init() {
  led_init(1, 5, LOW);
  settings_led_mode(L_BLINK);

  // Initialize data in flash, and get a copy
  flash_init(*(uint32_t*)"IONB");
  flash_read(4, (uint8_t*)settings, sizeof(settings));
  if (settings.player_setting[0] > 10)
    reset();

  last_buttons[0] = controller_button(B_TEST);
  last_buttons[1] = controller_button(B_SERVICE);
  poll_msec = timer3_tick_msec();
}

void settings_save() {
  // Store settings to flash
  flash_write(4, (const uint8_t*)settings, sizeof(settings));
}

void settings_poll() {
  if (!timer3_tick_msec_between(poll_msec, poll_msec + poll_interval)) {
    poll_msec = timer3_tick_msec();
    slow_poll();
  }

  led_poll();
}

uint16_t settings_rapid_mask(uint8_t player) {
  if (mode != S_NORMAL && mode != S_WAIT)
    return 0xffff;
  bool rapid_mask =
      rapid_step[player] >
      (settings.settings[settings.player_setting[player]].speed >> 1);
  return rapid_mask
             ? ~settings.settings[settings.player_setting[player]].rapid_fire
             : 0xffff;
}

uint16_t* settings_button_masks(uint8_t player) {
  return settings.settings[settings.player_setting[player]].button_masks;
}

uint8_t settings_mode() {
  return mode;
}

uint8_t settings_options_id() {
  uint8_t options = settings.common[settings.player_setting[0]].options;
  return ((options >> 5) & 2) | (options >> 7);
}

bool settings_options_pulldown() {
  uint8_t options = settings.common[settings.player_setting[0]].options;
  return (options & 0x20) == 0;
}

bool settings_options_rotary() {
  uint8_t options = settings.common[settings.player_setting[0]].options;
  return (options & 0x10) != 0;
}

bool settings_options_screen_position() {
  uint8_t options = settings.common[settings.player_setting[0]].options;
  return (options & 0x08) != 0;
}

bool settings_options_analog_lever() {
  uint8_t options = settings.common[settings.player_setting[0]].options;
  return (options & 0x04) == 0;
}

bool settings_options_analog_input() {
  uint8_t options = settings.common[settings.player_setting[0]].options;
  return (options & 0x02) != 0;
}

uint8_t* settings_options_analog() {
  return settings.common[settings.player_setting[0]].analog;
}

void settings_rapid_sync() {
  for (uint8_t i = 0; i < 2; ++i) {
    rapid_step[i] = rapid_step[i] + 1;
    if (rapid_step[i] > settings.settings[settings.player_setting[i]].speed)
      rapid_step[i] = 0;
  }
}

void settings_led_mode(uint8_t client_mode) {
  client_led_mode = client_mode;
  if (mode == S_NORMAL || mode == S_WAIT)
    led_mode(client_mode);
}

void settings_flip_options_pulldown() {
  settings.common[settings.player_setting[0]].options ^= 0x20;
  settings_save();
}
