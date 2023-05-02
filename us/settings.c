// Copyright 2023 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "settings.h"

#include "ch559.h"

static __code uint8_t* flash = (__code uint8_t*)0xf000;

static struct settings settings;

bool settings_init() {
  if (flash[0] != 'I' || flash[1] != 'O' || flash[2] != 'N' ||
      flash[3] != 'C') {
    return false;
  }
  return true;
}

void settings_poll() {}

struct settings* settings_get() {
  return &settings;
}