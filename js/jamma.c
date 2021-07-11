// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "jamma.h"

#include "chlib/ch559.h"

static uint8_t out;
static uint8_t sw[5];
static uint8_t current_coin_sw[2];
static uint8_t coin_sw[2];
static uint8_t coin_count[2];

void jamma_init() {
  // Input, pull-up
  for (uint8_t port = 0; port <= 4; ++port) {
    if (port == 2)
      continue;
    for (uint8_t pin = 0; pin < 8; ++pin) {
      if (port == 0 && pin < 4)
        continue;
      pinMode(port, pin, INPUT_PULLUP);
    }
  }

  jamma_reset();

  for (uint8_t i = 0; i < 2; ++i)
    coin_count[i] = 0;
}

void jamma_reset() {
  for (uint8_t i = 0; i < 2; ++i) {
    current_coin_sw[i] = 1;
    coin_sw[i] = 1;
  }
  for (uint8_t i = 0; i < 5; ++i)
    sw[i] = 0;
}

void jamma_update(bool swap) {
  uint8_t p0 = digitalReadPort(0);
  uint8_t p1 = digitalReadPort(1);
  uint8_t p3 = digitalReadPort(3);
  uint8_t p4 = digitalReadPort(4);

  uint8_t csw1 = p4 & 1;
  uint8_t csw2 = (p4 & 2) >> 1;
  current_coin_sw[0] &= csw1;
  current_coin_sw[1] &= csw2;

  sw[0] = (p4 & 4) ? 0 : 0x80;
  sw[1] = ~p3;
  sw[2] = (~p4 & 0xb0) | (~p1 & 0x40);
  sw[3] = (~p1 & 0xbf);
  sw[4] = ~p0 & 0xf0;

  if (swap) {
    uint8_t tmp;
#if defined(ALT_SWAP)
    tmp = sw[1];
    sw[1] &= 0xfc;
    sw[1] |= (tmp & 1) << 1;
    sw[1] |= sw[2] >> 7;
    sw[2] <<= 1;
    sw[2] |= (tmp & 2) << 5;
    tmp = sw[3];
    sw[3] &= 0xfc;
    sw[3] |= (tmp & 1) << 1;
    sw[3] |= sw[4] >> 7;
    sw[4] <<= 1;
    sw[4] |= (tmp & 2) << 5;
#else
    tmp = sw[1];
    sw[1] &= 0xfc;
    sw[1] |= (tmp & 2) >> 1;
    sw[1] |= (sw[2] >> 5) & 2;
    sw[2] >>= 1;
    sw[2] |= (tmp & 1) << 7;
    tmp = sw[3];
    sw[3] &= 0xfc;
    sw[3] |= (tmp & 2) >> 1;
    sw[3] |= (sw[4] >> 5) & 2;
    sw[4] >>= 1;
    sw[4] |= (tmp & 1) << 7;
#endif
  }
}

void jamma_sync() {
  for (uint8_t i = 0; i < 2; ++i) {
    if (!coin_sw[i] && current_coin_sw[i])
      coin_count[i]++;
    coin_sw[i] = current_coin_sw[i];
    current_coin_sw[i] = 1;
  }
}

uint8_t jamma_get_sw(uint8_t index, bool rapid_mode, bool rapid_mask) {
  uint8_t result = sw[index];
  if (rapid_mode)
    switch (index) {
      case 0:
        break;
      case 1:
      case 3:
        if (!rapid_mask)
          result |= (sw[index + 1] >> 5) & 0x03;
        break;
      case 2:
      case 4:
        if (!rapid_mask)
          result |= (result << 3) & 0x80;
        break;
    }
  return result;
}

uint8_t jamma_get_coin(uint8_t index) {
  return coin_count[index];
}

void jamma_sub_coin(uint8_t index, uint8_t sub) {
  if (index < 2)
    coin_count[index] -= sub;
}

void jamma_add_coin(uint8_t index, uint8_t add) {
  if (index < 2)
    coin_count[index] += add;
}