// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __soft485_h__
#define __soft485_h__

#include <stdbool.h>
#include <stdint.h>

#include "interrupt.h"

extern void soft485_int_tmr0() __interrupt INT_NO_TMR0 __using 0;
extern void soft485_int_uart() __interrupt INT_NO_UART1 __using 2;

void soft485_init();
void soft485_send(uint8_t val);
bool soft485_ready();
uint8_t soft485_recv();

void soft485_input();
void soft485_output();

void soft485_set_recv_speed(uint8_t mode);

#endif  // __soft485_h__
