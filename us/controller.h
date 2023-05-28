// Copyright 2023 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#ifndef __controller_h__
#define __controller_h__

#include <stdbool.h>
#include <stdint.h>

#include "hid.h"

void controller_reset();
void controller_update(const uint8_t hub,
                       const struct hub_info* info,
                       const uint8_t* data,
                       uint16_t size);
void controller_poll();
uint8_t controller_head();
uint8_t controller_data(uint8_t player, uint8_t index, uint8_t gpout);
uint8_t controller_coin(uint8_t player);
uint16_t controller_analog(uint8_t index);
uint16_t controller_rotary(uint8_t index);
uint16_t controller_screen(uint8_t index, uint8_t axis);
void controller_coin_add(uint8_t player, uint8_t add);
void controller_coin_sub(uint8_t player, uint8_t sub);
void controller_coin_set(uint8_t player, uint8_t value);

#endif  // __controller_h__