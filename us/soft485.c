// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "soft485.h"

#include "chlib/ch559.h"
#include "chlib/io.h"

static bool receiving = true;

void soft485_init() {
  Serial.println("soft485");
  // RXD1 connect P4.0, disable TXD1 by default

  SER1_LCR |= bLCR_DLAB;  // Allow SER1_DIV, SER1_DLM, and SER1_DLL use
  SER1_DIV = 1;
  // { SER1_DLM, SER1_DLL } = Fsys(48M) * 2 / SER1_DIV / 16 / baudrate(115200)
  SER1_DLM = 0;
  SER1_DLL = 52;  // should be set before enabling FIFO
  SER1_LCR &= ~bLCR_DLAB;

  // no parity, stop bit 1-bit, no interrupts by default
  SER1_LCR |= bLCR_WORD_SZ0 | bLCR_WORD_SZ1;  // data length 8-bits
  SER1_FCR = bFCR_FIFO_EN;  // Enable FIFO
}

void soft485_send(uint8_t val) {
  val;
}

bool soft485_ready() {
  if (!receiving)
    return false;
  return (SER1_LSR & bLSR_DATA_RDY) != 0;
}

uint8_t soft485_recv() {
  while (!soft485_ready());
  return SER1_FIFO;
}

void soft485_input() {
  SER1_FCR = bFCR_R_FIFO_CLR;  // Clear FIFO
  SER1_FCR = bFCR_FIFO_EN;  // Enable FIFO
  receiving = true;
}

void soft485_output() {
  SER1_FCR = 0;  // Disable FIFO
  receiving = false;
}