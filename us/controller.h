// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#ifndef __controller_h__
#define __controller_h__

#include <stdbool.h>
#include <stdint.h>

#include "hid.h"
#include "led.h"

// Button bitmap in raw format
enum {
  B_COIN = 0,
  B_START,
  B_1,
  B_2,
  B_3,
  B_4,
  B_5,
  B_6,
  B_7,
  B_8,
  B_9,
  B_10,
};

// Button
enum {
  B_TEST,
  B_SERVICE,
};

void controller_init();
void controller_update(uint8_t hub,
                       const struct hub_info* info,
                       const uint8_t* data,
                       uint16_t size,
                       uint16_t* mask);
void controller_map(uint8_t player,
                    uint16_t rapid_mask,
                    uint16_t* button_masks);
void controller_poll();
uint16_t controller_raw(uint8_t player);
uint8_t controller_jvs(uint8_t index, uint8_t gpout);
uint8_t controller_coin(uint8_t player);
uint16_t controller_analog(uint8_t index);
void controller_coin_add(uint8_t player, uint8_t add);
void controller_coin_sub(uint8_t player, uint8_t sub);
bool controller_button(uint8_t button);

#endif  // __controller_h__