// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __soft485_h__
#define __soft485_h__

#include <stdbool.h>
#include <stdint.h>

void soft485_init();
void soft485_send(uint8_t val);
bool soft485_ready();
uint8_t soft485_recv();

void soft485_input();
void soft485_output();

#endif  // __soft485_h__
