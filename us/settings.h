// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#ifndef __settings_h__
#define __settings_h__

#include <stdbool.h>
#include <stdint.h>

// Should be included from the main source file to set up interrupt handler.
#include "led.h"

// Settings mode
enum {
  S_NORMAL,
  S_WAIT,
  S_LAYOUT,
  S_RAPID,
  S_SPEED,

  S_SELECT,
  S_ANALOG,
  S_OPTION,

  S_RESET,
};

struct player_setting {
  uint8_t magic;
  uint8_t speed;  // Rapid fire speed (flip per every N msec)
  uint16_t rapid_fire;
  uint16_t button_masks[12];
  uint16_t padding[2];
};

void settings_init(void);
void settings_save(void);
void settings_poll(void);

uint16_t settings_rapid_mask(uint8_t player);
uint16_t* settings_button_masks(uint8_t player);
uint8_t settings_options_id(void);
bool settings_options_pulldown(void);
bool settings_options_rotary(void);
bool settings_options_screen_position(void);
bool settings_options_analog_lever(void);
bool settings_options_analog_input(void);
uint8_t* settings_options_analog(void);
bool settings_options_dash(void);
uint8_t settings_mode(void);
void settings_rapid_sync(void);

void settings_led_mode(uint8_t mode);
void settings_flip_options_pulldown(void);

#endif  // __settings_h__