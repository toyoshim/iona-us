// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#ifndef __dipsw_h__
#define __dipsw_h__

#include <stdbool.h>
#include <stdint.h>

void dipsw_init();
void dipsw_update();
void dipsw_sync();
bool dipsw_get_rapid_mode();
bool dipsw_get_rapid_mask();
bool dipsw_get_swap_mode();

#endif  // __dipsw_h__