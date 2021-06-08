// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#ifndef __jamma_h__
#define __jamma_h__

#include <stdbool.h>
#include <stdint.h>

void jamma_init();
void jamma_reset();
void jamma_update(bool swap);
void jamma_sync();
uint8_t jamma_get_sw(uint8_t index, bool rapid_mode, bool rapid_mask);
uint8_t jamma_get_coin(uint8_t index);
void jamma_sub_coin(uint8_t index, uint8_t sub);
void jamma_add_coin(uint8_t index, uint8_t sub);

#endif  // __jamma_h__