// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "chlib/ch559.h"
#include "chlib/led.h"
#include "client.h"
#include "hid.h"
#include "jvsio/JVSIO_c.h"
#include "soft485.h"

//#define _DBG_HID_REPORT_DUMP
//#define _DBG_JVS_BUTTON_DUMP

#define VER "1.10g"

static const char id[] = "SEGA ENTERPRISES,LTD.compat;MP07-IONA-US;ver" VER;

static struct JVSIO_DataClient data;
static struct JVSIO_SenseClient sense;
static struct JVSIO_LedClient led;

static int8_t coin_index_bias = 0;
static uint8_t sw[5] = { 0, 0, 0, 0, 0 };
static uint8_t coin_sw[2] = { 0, 0 };
static uint8_t coin[2] = { 0, 0 };

static void debug_putc(uint8_t val) { val; }

static void jvs_poll(struct JVSIO_Lib* io) {
  void (*original_putc)() = Serial.putc;
  Serial.putc = debug_putc;
  uint8_t len;
  uint8_t* data = io->getNextCommand(io, &len, 0);
  Serial.putc = original_putc;
  if (!data)
    return;

  switch (*data) {
   case kCmdReset:
    coin_index_bias = 0;
    break;
   case kCmdIoId:
    io->pushReport(io, kReportOk);
    for (uint8_t i = 0; id[i]; ++i)
      io->pushReport(io, id[i]);
    io->pushReport(io, 0x00);
    break;
   case kCmdFunctionCheck:
    io->pushReport(io, kReportOk);

    io->pushReport(io, 0x01);  // sw
    io->pushReport(io, 0x02);  // players
    io->pushReport(io, 0x10);  // buttons
    io->pushReport(io, 0x00);

    io->pushReport(io, 0x02);  // coin
    io->pushReport(io, 0x02);  // slots
    io->pushReport(io, 0x00);
    io->pushReport(io, 0x00);

    io->pushReport(io, 0x03);  // analog inputs
    io->pushReport(io, 0x08);  // channels
    io->pushReport(io, 0x00);  // bits
    io->pushReport(io, 0x00);

    io->pushReport(io, 0x12);  // general purpose driver
    io->pushReport(io, 0x08);  // slots
    io->pushReport(io, 0x00);
    io->pushReport(io, 0x00);

    io->pushReport(io, 0x00);
    break;
   case kCmdSwInput:
    io->pushReport(io, kReportOk);
    if (data[1] == 2 && data[2] == 2) {
      io->pushReport(io, sw[0]);
      io->pushReport(io, sw[1]);
      io->pushReport(io, sw[2]);
      io->pushReport(io, sw[3]);
      io->pushReport(io, sw[4]);
    } else {
      Serial.println("Err CmdSwInput");
    }
    break;
   case kCmdCoinInput:
    io->pushReport(io, kReportOk);
    if (data[1] <= 2) {
      for (uint8_t i = 0; i < data[1]; ++i) {
        io->pushReport(io, (0 << 6) | 0);
        io->pushReport(io, coin[i]);
      }
    } else {
      Serial.println("Err CmdCoinInput");
    }
    break;
   case kCmdAnalogInput:
    io->pushReport(io, kReportOk);
    for (uint8_t channel = 0; channel < data[1]; ++channel) {
      io->pushReport(io, 0x80);
      io->pushReport(io, 0x00);
    }
    break;
   case kCmdCoinSub:
   case kCmdCoinAdd:
    // Coin slot index should start with 1, but some PCB seem to expect starting
    // with 0. Following code detects the slot index 0 and sets the bias to 1
    // so that it offsets.
    if (data[1] == 0)
      coin_index_bias = 1;
    if (*data == kCmdCoinSub)
      coin[data[1] + coin_index_bias - 1] -= data[2];
    else
      coin[data[1] + coin_index_bias - 1] += data[2];
    io->pushReport(io, kReportOk);
    break;
   case kCmdDriverOutput:
    io->pushReport(io, kReportOk);
    break;
  }
}

static inline bool button_check(uint16_t index, const uint8_t* data) {
  if (index == 0xffff)
    return false;
  uint8_t byte = index >> 3;
  uint8_t bit = index & 7;
  return data[byte] & (1 << bit);
}

static void report(
    uint8_t hub, const struct hub_info* info, const uint8_t* data, uint16_t size) {
  size;
#ifdef _DBG_HID_REPORT_DUMP
  static uint8_t old_data[256];
  bool modified = false;
  for (uint8_t i = 0; i < size; ++i) {
    if (old_data[i] == data[i])
      continue;
    modified = true;
    old_data[i] = data[i];
  }
  if (!modified)
    return;
  Serial.printf("Report %d Bytes: ", size);
  for (uint8_t i = 0; i < size; ++i)
    Serial.printf("%x,", data[i]);
  Serial.println("");
#endif  // _DBG_HID_REPORT_DUMP

  if (info->state != HID_STATE_READY) {
    sw[1 + hub * 2 + 0] = 0;
    sw[1 + hub * 2 + 1] = 0;
    return;
  }
  if (info->report_id)
    data++;
  uint8_t u = button_check(info->dpad[0], data) ? 1 : 0;
  uint8_t d = button_check(info->dpad[1], data) ? 1 : 0;
  uint8_t l = button_check(info->dpad[2], data) ? 1 : 0;
  uint8_t r = button_check(info->dpad[3], data) ? 1 : 0;
  if (info->axis[0] != 0xffff && info->axis_size[0] == 8) {
    uint8_t x = data[info->axis[0] >> 3];
    if (x < 64) l = 1;
    else if (x > 192) r = 1;
  }
  if (info->axis[0] != 0xffff && info->axis_size[0] == 16) {
    uint8_t byte = info->axis[0] >> 3;
    uint16_t x = data[byte] | ((uint16_t)data[byte + 1] << 8);
    if (info->axis_sign[0])
      x += 0x8000;
    if (info->axis_polarity[0])
      x = -x - 1;
    if (x < 0x4000) l = 1;
    else if (x > 0xc000) r = 1;
  }
  if (info->axis[1] != 0xffff && info->axis_size[1] == 8) {
    uint8_t y = data[info->axis[1] >> 3];
    if (y < 64) u = 1;
    else if (y > 192) d = 1;
  }
  if (info->axis[1] != 0xffff && info->axis_size[1] == 16) {
    uint8_t byte = info->axis[1] >> 3;
    uint16_t y = data[byte] | ((uint16_t)data[byte + 1] << 8);
    if (info->axis_sign[1])
      y += 0x8000;
    if (info->axis_polarity[1])
      y = -y - 1;
    if (y < 0x4000) u = 1;
    else if (y > 0xc000) d = 1;
  }
  if (info->hat != 0xffff) {
    uint8_t byte = info->hat >> 3;
    uint8_t bit = info->hat & 7;
    uint8_t hat = (data[byte] >> bit) & 0xf;
    switch (hat) {
      case 0:
        u = 1;
        break;
      case 1:
        u = 1;
        r = 1;
        break;
      case 2:
        r = 1;
        break;
      case 3:
        r = 1;
        d = 1;
        break;
      case 4:
        d = 1;
        break;
      case 5:
        d = 1;
        l = 1;
        break;
      case 6:
        l = 1;
        break;
      case 7:
        l = 1;
        u = 1;
        break;
    }
  }
  coin_sw[hub] = (coin_sw[hub] << 1) |
                 (button_check(info->button[ 8], data) ? 0x01 : 0);
  if ((coin_sw[hub] & 3) == 1)
    coin[hub]++;
  sw[1 + hub * 2 + 0] = (button_check(info->button[ 9], data) ? 0x80 : 0) |
                        (u ? 0x20 : 0) |
                        (d ? 0x10 : 0) |
                        (l ? 0x08 : 0) |
                        (r ? 0x04 : 0) |
                        (button_check(info->button[ 0], data) ? 0x02 : 0) |
                        (button_check(info->button[ 1], data) ? 0x01 : 0);
  sw[1 + hub * 2 + 1] = (button_check(info->button[ 2], data) ? 0x80 : 0) |
                        (button_check(info->button[ 3], data) ? 0x40 : 0) |
                        (button_check(info->button[ 4], data) ? 0x20 : 0) |
                        (button_check(info->button[ 5], data) ? 0x10 : 0) |
                        (button_check(info->button[ 6], data) ? 0x08 : 0) |
                        (button_check(info->button[ 7], data) ? 0x04 : 0) |
                        (button_check(info->button[10], data) ? 0x02 : 0) |
                        (button_check(info->button[11], data) ? 0x01 : 0);
}

void main() {
  initialize();
  pinMode(4, 6, INPUT_PULLUP);
  pinMode(1, 7, INPUT_PULLUP);
  delay(30);
  Serial.println(id);

  data_client(&data);
  sense_client(&sense);
  led_client(&led);

  struct JVSIO_Lib* io = JVSIO_open(&data, &sense, &led, 1);
  io->begin(io);
  Serial.println("JVS I/O ready");

  struct hid hid;
  hid.report = report;
  hid_init(&hid);
  Serial.println("USB Host ready");

  for (;;) {
    hid_poll();
    led_poll();
    if (digitalRead(4, 6) == LOW)
      sw[1] |= 0x40;
    else
      sw[1] &= ~0x40;
    if (digitalRead(1, 7) == LOW)
      sw[0] |= 0x80;
    else
      sw[0] &= ~0x80;
    jvs_poll(io);
#ifdef _DBG_JVS_BUTTON_DUMP
    static uint8_t old_sw[5] = { 0, 0, 0, 0, 0 };
    if (old_sw[0] != sw[0] || old_sw[1] != sw[1] || old_sw[2] != sw[2] ||
        old_sw[3] != sw[3] || old_sw[4] != sw[4]) {
      for (uint8_t i = 0; i < 5; ++i)
        old_sw[i] = sw[i];
      Serial.printc(sw[0], BIN);
      Serial.putc('_');
      Serial.printc(sw[1], BIN);
      Serial.putc('_');
      Serial.printc(sw[2], BIN);
      Serial.putc('_');
      Serial.printc(sw[3], BIN);
      Serial.putc('_');
      Serial.printc(sw[4], BIN);
      Serial.println("");
    }
#endif  // _DBG_JVS_BUTTON_DUMP
  }
}