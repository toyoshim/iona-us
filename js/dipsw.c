// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "dipsw.h"

#include "chlib/ch559.h"

static uint8_t dipsw = 0xff;
static bool rapid = false;
static uint8_t rapid_count = 0;
static uint8_t rapid_mod = 0;
static uint8_t rapid_th = 0;
static bool swap = false;

static uint8_t peek() {
  return ~digitalReadPort(2) & 0x0f;
}

void dipsw_init() {
  for (uint8_t pin = 0; pin < 4; ++pin)
    pinMode(2, pin, INPUT_PULLUP);
  dipsw_update();
}

void dipsw_update() {
  uint8_t old = dipsw;
  dipsw = peek();
  if (old == dipsw)
    return;
  Serial.print("DIPSW: ");
  Serial.printc(dipsw, BIN);
  Serial.println("");
  rapid = dipsw & 1;
  switch (dipsw & 0x0c) {
    case 0x00:  // __00 - 12
      rapid_mod = 5;
      rapid_th = 2;
      break;
    case 0x08:  // __01 - 15
      rapid_mod = 4;
      rapid_th = 2;
      break;
    case 0x04:  // __10 - 20
      rapid_mod = 3;
      rapid_th = 1;
      break;
    case 0x0c:  // __11 - 30
      rapid_mod = 2;
      rapid_th = 1;
      break;
  }
#if !defined(NO_SWAP)
  swap = dipsw & 2;
#endif
}

void dipsw_sync() {
  if (!rapid)
    return;
  rapid_count++;
  if (rapid_count == rapid_mod)
    rapid_count = 0;
}

bool dipsw_get_rapid_mode() {
  return rapid;
}

bool dipsw_get_rapid_mask() {
  return rapid_count < rapid_th;
}

bool dipsw_get_swap_mode() {
  return swap;
}