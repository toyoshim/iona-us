// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "chlib/ch559.h"
#include "chlib/led.h"
#include "chlib/serial.h"
#include "client.h"
#include "controller.h"
#include "hid.h"
#include "jvsio/JVSIO_c.h"
#include "settings.h"
#include "soft485.h"

#define VER "1.32"

static const char id[] = "SEGA ENTERPRISES,LTD.compat;MP07-IONA-US;ver" VER;

static struct JVSIO_DataClient data;
static struct JVSIO_SenseClient sense;
static struct JVSIO_LedClient led;

static int8_t coin_index_bias = 0;
static uint8_t gpout = 0;

static void debug_putc(uint8_t val) {
  val;
}

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

      io->pushReport(io, 0x04);  // rotary inputs
      io->pushReport(io, 0x02);  // channels
      io->pushReport(io, 0x00);
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
        if (settings_mode() != S_NORMAL) {
          io->pushReport(io, 0);
          io->pushReport(io, 0);
          io->pushReport(io, 0);
          io->pushReport(io, 0);
          io->pushReport(io, 0);
        } else {
          settings_rapid_sync();
          controller_map(0, settings_rapid_mask(0), settings_button_masks(0));
          controller_map(1, settings_rapid_mask(1), settings_button_masks(1));
          io->pushReport(io, controller_jvs(0, gpout));
          io->pushReport(io, controller_jvs(1, gpout));
          io->pushReport(io, controller_jvs(2, gpout));
          io->pushReport(io, controller_jvs(3, gpout));
          io->pushReport(io, controller_jvs(4, gpout));
        }
      } else {
        Serial.println("Err CmdSwInput");
      }
      break;
    case kCmdCoinInput:
      io->pushReport(io, kReportOk);
      if (data[1] <= 2) {
        for (uint8_t i = 0; i < data[1]; ++i) {
          io->pushReport(io, (0 << 6) | 0);
          io->pushReport(io, controller_coin(i));
        }
      } else {
        Serial.println("Err CmdCoinInput");
      }
      break;
    case kCmdAnalogInput:
      io->pushReport(io, kReportOk);
      for (uint8_t channel = 0; channel < data[1]; ++channel) {
        uint16_t analog = controller_analog(channel);
        io->pushReport(io, analog >> 8);
        io->pushReport(io, analog & 0xff);
      }
      break;
    case kCmdRotaryInput:
      io->pushReport(io, kReportOk);
      for (uint8_t channel = 0; channel < data[1]; ++channel) {
        // Experimentally reports analog inputs.
        uint16_t analog = controller_analog(channel);
        io->pushReport(io, analog >> 8);
        io->pushReport(io, analog & 0xff);
      }
      break;
    case kCmdCoinSub:
    case kCmdCoinAdd:
      // Coin slot index should start with 1, but some PCB seem to expect
      // starting with 0. Following code detects the slot index 0 and sets the
      // bias to 1 so that it offsets.
      if (data[1] == 0)
        coin_index_bias = 1;
      if (*data == kCmdCoinSub)
        controller_coin_sub(data[1] + coin_index_bias - 1, data[3]);
      else
        controller_coin_add(data[1] + coin_index_bias - 1, data[3]);
      io->pushReport(io, kReportOk);
      break;
    case kCmdDriverOutput:
      gpout = data[2];
      io->pushReport(io, kReportOk);
      break;
  }
}

static void report(uint8_t hub,
                   const struct hub_info* info,
                   const uint8_t* data,
                   uint16_t size) {
  controller_update(hub, info, data, size, settings_button_masks(hub));
}

void main() {
  initialize();
  controller_init();
  settings_init();
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
    controller_poll();
    settings_poll();
    jvs_poll(io);
  }
}
