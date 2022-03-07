// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "ch559.h"
#include "hid.h"
#include "led.h"
#include "serial.h"

#include "client.h"
#include "controller.h"
#include "jvsio/JVSIO_c.h"
#include "settings.h"
#include "soft485.h"

#define VER "1.42a"

static const char sega_id[] =
    "SEGA ENTERPRISES,LTD.compat;MP07-IONA-US;ver" VER;
static const char namco_gun_id[] =
    "namco ltd.;JYU-PCB;compat Ver" VER ";MP07-IONA-US,2Coins 2Guns";
static const char namco_multi_id[] =
    "namco ltd.;NA-JV;compat Ver" VER ";MP07-IONA-US,Multipurpose.";
static const char namco_multi_exact_id[] =
    "namco ltd.;NA-JV;Ver1.00;JPN,Multipurpose.";
static const char* ids[4] = {
    sega_id,
    namco_multi_exact_id,  // tentative.
    namco_gun_id,
    namco_multi_id,
};

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
      {
        const char* id = ids[settings_options_id()];
        for (uint8_t i = 0; id[i]; ++i)
          io->pushReport(io, id[i]);
      }
      io->pushReport(io, 0x00);
      break;
    case kCmdFunctionCheck:
      io->pushReport(io, kReportOk);

      io->pushReport(io, 0x01);  // sw
      if (settings_options_id() != 3) {
        io->pushReport(io, 0x02);  // players
        io->pushReport(io, 0x10);  // buttons
        io->pushReport(io, 0x00);
      } else {
        io->pushReport(io, 0x01);  // players
        io->pushReport(io, 0x18);  // buttons
        io->pushReport(io, 0x00);
      }

      io->pushReport(io, 0x02);  // coin
      io->pushReport(io, 0x02);  // slots
      io->pushReport(io, 0x00);
      io->pushReport(io, 0x00);

      io->pushReport(io, 0x03);  // analog inputs
      if (settings_options_id() != 3) {
        io->pushReport(io, 0x06);  // channels
        io->pushReport(io, 0x00);  // bits
      } else {
        io->pushReport(io, 0x08);  // channels
        io->pushReport(io, 0x10);  // bits
      }
      io->pushReport(io, 0x00);

      if (settings_options_rotary()) {
        io->pushReport(io, 0x04);  // rotary inputs
        io->pushReport(io, 0x02);  // channels
        io->pushReport(io, 0x00);
        io->pushReport(io, 0x00);
      }

      if (settings_options_screen_position()) {
        io->pushReport(io, 0x06);  // screen position inputs
        if (settings_options_id() != 3) {
          io->pushReport(io, 0x0a);  // Xbits
          io->pushReport(io, 0x0a);  // Ybits
          io->pushReport(io, 0x02);  // channels
        } else {
          io->pushReport(io, 0x10);  // Xbits
          io->pushReport(io, 0x10);  // Ybits
          io->pushReport(io, 0x01);  // channels
        }
      }

      io->pushReport(io, 0x12);  // general purpose driver
      io->pushReport(io, 0x12);  // slots
      io->pushReport(io, 0x00);
      io->pushReport(io, 0x00);

      if (settings_options_id() == 3) {
        io->pushReport(io, 0x13);  // analog output
        io->pushReport(io, 0x02);  // channel
        io->pushReport(io, 0x00);
        io->pushReport(io, 0x00);

        io->pushReport(io, 0x14);  // character output
        io->pushReport(io, 0x10);  // character
        io->pushReport(io, 0x01);  // line
        io->pushReport(io, 0x00);  // code
      }

      io->pushReport(io, 0x00);
      break;
    case kCmdSwInput:
      io->pushReport(io, kReportOk);
      {
        uint8_t lines = 1 + data[1] * data[2];
        if (settings_mode() != S_NORMAL) {
          for (uint8_t i = 0; i < lines; ++i)
            io->pushReport(io, 0);
        } else {
          settings_rapid_sync();
          controller_map(0, settings_rapid_mask(0), settings_button_masks(0));
          controller_map(1, settings_rapid_mask(1), settings_button_masks(1));
          for (uint8_t i = 0; i < lines; ++i)
            io->pushReport(io, controller_jvs(i, gpout));
        }
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
      {
        uint8_t* analog = settings_options_analog();
        for (uint8_t channel = 0; channel < data[1]; ++channel) {
          uint8_t index = channel < 6 ? (analog[channel] & 0x7f) : channel;
          bool sign = channel < 6 && analog[channel] & 0x80;
          uint16_t value = controller_analog(index);
          if (sign)
            value = 0xffff - value;
          io->pushReport(io, value >> 8);
          io->pushReport(io, value & 0xff);
        }
      }
      break;
    case kCmdRotaryInput:
      io->pushReport(io, kReportOk);
      {
        uint8_t* analog = settings_options_analog();
        for (uint8_t channel = 0; channel < data[1]; ++channel) {
          uint8_t index = channel < 6 ? (analog[channel] & 0x7f) : channel;
          bool sign = channel < 6 && (analog[channel] & 0x80) == 0x80;
          uint16_t value = controller_analog(index);
          if (sign)
            value = 0xffff - value;
          io->pushReport(io, value >> 8);
          io->pushReport(io, value & 0xff);
        }
      }
      break;
    case kCmdScreenPositionInput:
      io->pushReport(io, kReportOk);
      {
        uint8_t channel = data[1] - 1;
        uint8_t* analog = settings_options_analog();
        uint8_t index =
            channel < 3 ? (analog[channel * 2 + 0] & 0x7f) : channel * 2 + 0;
        bool sign = channel < 3 && (analog[channel * 2 + 0] & 0x80) == 0x80;
        uint16_t x = controller_analog(index);
        if (sign)
          x = 0xffff - x;
        x = x >> 6;
        io->pushReport(io, x >> 8);
        io->pushReport(io, x & 0xff);
        index =
            channel < 3 ? (analog[channel * 2 + 1] & 0x7f) : channel * 2 + 1;
        sign = channel < 3 && (analog[channel * 2 + 1] & 0x80) == 0x80;
        uint16_t y = controller_analog(index);
        if (sign)
          y = 0xffff - y;
        if (settings_options_id() != 3)
          y = y >> 6;
        io->pushReport(io, y >> 8);
        io->pushReport(io, y & 0xff);
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
    case kCmdAnalogOutput:
    case kCmdCharacterOutput:
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
  Serial.println(ids[settings_options_id()]);

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
