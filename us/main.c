// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "ch559.h"
#include "hid.h"
#include "jvsio/JVSIO_c.h"
#include "led.h"
#include "serial.h"

#include "client.h"
#include "controller.h"
#include "settings.h"
#include "soft485.h"

#define VER "2.00"

static const char sega_id[] =
    "SEGA ENTERPRISES,LTD.compat;MP07-IONA-US;ver" VER;
static const char namco_gun_id[] =
    "namco ltd.;JYU-PCB;compat Ver" VER ";MP07-IONA-US,2Coins 2Guns";
static const char namco_multi_id[] =
    "namco ltd.;NA-JV;Ver4.00;JPN,Multipurpose.,MP07-IONA-US,v" VER;
static const char* ids[3] = {
    sega_id,
    namco_gun_id,
    namco_multi_id,
};

static struct JVSIO_DataClient data;
static struct JVSIO_SenseClient sense;
static struct JVSIO_LedClient led;
static struct JVSIO_TimeClient time;

static int8_t coin_index_bias = 0;
static uint8_t gpout = 0;

static struct settings* settings = 0;

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
        const char* id = ids[settings->id];
        for (uint8_t i = 0; id[i]; ++i)
          io->pushReport(io, id[i]);
      }
      io->pushReport(io, 0x00);
      break;
    case kCmdFunctionCheck:
      io->pushReport(io, kReportOk);

      io->pushReport(io, 0x01);  // sw
      if (settings->id != IT_NAMCO_NAJV) {
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

      if (settings->analog_input_count) {
        io->pushReport(io, 0x03);  // analog inputs
        io->pushReport(io, settings->analog_input_count);
        io->pushReport(io, settings->analog_input_width);
        io->pushReport(io, 0x00);
      }

      if (settings->rotary_input_count) {
        io->pushReport(io, 0x04);  // rotary inputs
        io->pushReport(io, settings->rotary_input_count);
        io->pushReport(io, 0x00);
        io->pushReport(io, 0x00);
      }

      if (settings->screen_position_count) {
        io->pushReport(io, 0x06);  // screen position inputs
        io->pushReport(io, settings->screen_position_width);
        io->pushReport(io, settings->screen_position_width);
        io->pushReport(io, settings->screen_position_count);
      }

      io->pushReport(io, 0x12);  // general purpose driver
      io->pushReport(io, 0x12);  // slots
      io->pushReport(io, 0x00);
      io->pushReport(io, 0x00);

      if (settings->analog_output) {
        io->pushReport(io, 0x13);  // analog output
        io->pushReport(io, settings->analog_output);
        io->pushReport(io, 0x00);
        io->pushReport(io, 0x00);
      }

      if (settings->character_display_width) {
        io->pushReport(io, 0x14);  // character output
        io->pushReport(io, settings->character_display_width);
        io->pushReport(io, settings->character_display_height);
        io->pushReport(io, 0x00);  // code
      }

      io->pushReport(io, 0x00);
      break;
    case kCmdSwInput:
      io->pushReport(io, kReportOk);
      settings_rapid_sync();
      io->pushReport(io, controller_head());
      for (uint8_t player = 0; player < data[1]; ++player) {
        for (uint8_t i = 0; i < data[2]; ++i) {
          io->pushReport(io, controller_data(player, i, gpout));
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
      for (uint8_t i = 0; i < data[1]; ++i) {
        uint16_t value = controller_analog(i);
        io->pushReport(io, value >> 8);
        io->pushReport(io, value & 0xff);
      }
      break;
    case kCmdRotaryInput:
      io->pushReport(io, kReportOk);
      for (uint8_t i = 0; i < data[1]; ++i) {
        uint16_t value = controller_rotary(i);
        io->pushReport(io, value >> 8);
        io->pushReport(io, value & 0xff);
      }
      break;
    case kCmdScreenPositionInput:
      io->pushReport(io, kReportOk);
      {
        uint8_t index = data[1] - 1;
        uint16_t x = controller_screen(index, 0) >>
                     (16 - settings->screen_position_width);
        io->pushReport(io, x >> 8);
        io->pushReport(io, x & 0xff);
        uint16_t y = controller_screen(index, 1) >>
                     (16 - settings->screen_position_width);
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
    case kCmdNamco:
      if (data[4] == 0x14) {
        io->sendUnknownStatus(io);
      } else {
        io->pushReport(io, kReportOk);
        io->pushReport(io, 0x01);
      }
      break;
  }
}

static void detected() {
  led_oneshot(L_PULSE_ONCE);
}

static uint8_t get_flags() {
  return USE_HUB1 | USE_HUB0;
}

void main() {
  initialize();

  settings_init();
  settings = settings_get();

  delay(30);

  client_init(&data, &sense, &led, &time);

  struct JVSIO_Lib* io = JVSIO_open(&data, &sense, &led, &time, 1);
  io->begin(io);

  struct hid hid;
  hid.report = controller_update;
  hid.detected = detected;
  hid.get_flags = get_flags;
  hid_init(&hid);

  for (;;) {
    hid_poll();
    controller_poll();
    settings_poll();
    jvs_poll(io);
  }
}
