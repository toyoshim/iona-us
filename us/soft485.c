// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "soft485.h"

#include "chlib/ch559.h"
#include "chlib/io.h"

static bool receiving = true;
static uint8_t count = 20;
static uint8_t data[] = { 0, 1, 0, 0, 0, 0, 0, 1, 0, 1 };  // start, 'A', stop
static uint8_t tx_buffer[8];
static volatile uint8_t tx_wr_ptr = 0;
static volatile uint8_t tx_rd_ptr = 0;

void soft485_int() __interrupt INT_NO_TMR0 __using 0 {
  if (count == 20) {
    if (tx_wr_ptr == tx_rd_ptr)
      return;
    for (uint8_t i = 0; i < 8; ++i)
      data[i + 1] = (tx_buffer[tx_rd_ptr] >> i) & 1;
    tx_rd_ptr = (tx_rd_ptr + 1) & 7;
    count = 0;
  }
  if (0 == (count & 1)) {
    count++;
    return;
  }
  if (data[count >> 1]) {
    P4_0 = 1;
    P1_1 = 0;  // XXX: proto
  } else {
    P4_0 = 0;
    P1_1 = 1;  // XXX: proto
  }
  count++;
}

void soft485_init() {
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

  // Timer0 mode 2, 8-bit auto
  TMOD = (TMOD | bT0_M1) & ~bT0_M0;
  T2MOD |= bT0_CLK;  // faster clock
  TL0 = 0;
  TH0 = 48;  // 115200 x2
  ET0 = 1;
  EA = 1;
}

void soft485_send(uint8_t val) {
  uint8_t next = (tx_wr_ptr + 1) & 7;
  while (tx_rd_ptr == next);
  tx_buffer[tx_wr_ptr] = val;
  tx_wr_ptr = next;
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
  pinMode(4, 0, INPUT_PULLUP);
  pinMode(1, 1, INPUT_PULLUP);  // XXX: proto
  SER1_FCR = bFCR_R_FIFO_CLR;  // Clear FIFO
  SER1_FCR = bFCR_FIFO_EN;  // Enable FIFO
  TR0 = 0;  // Stop timer count
  receiving = true;
}

void soft485_output() {
  SER1_FCR = 0;  // Disable FIFO
  digitalWrite(4, 0, HIGH);
  digitalWrite(1, 1, LOW);  // XXX: proto
  pinMode(4, 0, OUTPUT);
  pinMode(1, 1, OUTPUT);  // XXX: proto
  TL0 = 0;
  count = 20;
  TR0 = 1;  // Start timer count
  receiving = false;
}