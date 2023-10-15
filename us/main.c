// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "ch559.h"
#include "hid.h"
#include "jvsio_client.h"
#include "jvsio_common.h"
#include "jvsio_node.h"
#include "led.h"
#include "serial.h"

#include "client.h"
#include "controller.h"
#include "settings.h"
#include "soft485.h"

// #define _DBG_HUB1_ONLY
// #define _DBG_ANALOG
// #define _DBG_DIGITAL

#define VER "2.17"

static const char sega_id[] =
    "SEGA ENTERPRISES,LTD.compat;MP07-IONA-US;ver" VER;
static const char namco_gun_id[] =
    "namco ltd.;JYU-PCB;compat Ver" VER ";MP07-IONA-US,2Coins 2Guns";
static const char namco_multi_id[] =
    "namco ltd.;NA-JV;Ver4.00;JPN,Multipurpose.,MP07-IONA-US,v" VER;
static const char namco_tss_id[] =
    "namco ltd.;TSS-I/O;Ver2.02;JPN,GUN-EXTENTION,MP07-IONA-US,v" VER;
static const char* ids[4] = {
    sega_id,
    namco_gun_id,
    namco_multi_id,
    namco_tss_id,
};

static int8_t coin_index_bias = 0;
static uint8_t gpout = 0;
static int8_t reserved_coin_index_bias = 0;
static uint8_t reserved_coin[2] = {0, 0};

static struct settings* settings = 0;

static void debug_putc(uint8_t val) {
  val;
}

bool JVSIO_Client_receiveCommand(uint8_t node,
                                 uint8_t* data,
                                 uint8_t len,
                                 bool commit) {
  node;
  len;
  if (!data) {
    coin_index_bias = reserved_coin_index_bias;
    controller_coin_set(0, reserved_coin[0]);
    controller_coin_set(1, reserved_coin[1]);
    return true;
  }

  bool handled = true;

  switch (*data) {
    case kCmdReset:
      coin_index_bias = 0;
      break;
    case kCmdIoId:
      JVSIO_Node_pushReport(kReportOk);
      {
        const char* id = ids[settings->id];
        for (uint8_t i = 0; id[i]; ++i) {
          JVSIO_Node_pushReport(id[i]);
        }
      }
      JVSIO_Node_pushReport(0x00);
      break;
    case kCmdFunctionCheck: {
      JVSIO_Node_pushReport(kReportOk);

      JVSIO_Node_pushReport(0x01);  // sw
      uint8_t players = 2;
      uint8_t gpouts = 18;
      if (settings->id == IT_NAMCO_NAJV) {
        JVSIO_Node_pushReport(0x01);  // players
        JVSIO_Node_pushReport(0x18);  // buttons
        JVSIO_Node_pushReport(0x00);
        gpouts = 18;
        players = 1;
      } else if (settings->id == IT_NAMCO_TSS) {
        JVSIO_Node_pushReport(0x01);  // players
        JVSIO_Node_pushReport(0x0c);  // buttons
        JVSIO_Node_pushReport(0x00);
        gpouts = 3;
        players = 1;
      } else {
        JVSIO_Node_pushReport(0x02);  // players
        JVSIO_Node_pushReport(0x10);  // buttons
        JVSIO_Node_pushReport(0x00);
      }

      JVSIO_Node_pushReport(0x02);     // coin
      JVSIO_Node_pushReport(players);  // slots
      JVSIO_Node_pushReport(0x00);
      JVSIO_Node_pushReport(0x00);

      if (settings->analog_input_count) {
        JVSIO_Node_pushReport(0x03);  // analog inputs
        JVSIO_Node_pushReport(settings->analog_input_count);
        JVSIO_Node_pushReport(settings->analog_input_width);
        JVSIO_Node_pushReport(0x00);
      }

      if (settings->rotary_input_count) {
        JVSIO_Node_pushReport(0x04);  // rotary inputs
        JVSIO_Node_pushReport(settings->rotary_input_count);
        JVSIO_Node_pushReport(0x00);
        JVSIO_Node_pushReport(0x00);
      }

      if (settings->screen_position_count) {
        JVSIO_Node_pushReport(0x06);  // screen position inputs
        JVSIO_Node_pushReport(settings->screen_position_width);
        JVSIO_Node_pushReport(settings->screen_position_width);
        JVSIO_Node_pushReport(settings->screen_position_count);
      }

      JVSIO_Node_pushReport(0x12);    // general purpose driver
      JVSIO_Node_pushReport(gpouts);  // slots
      JVSIO_Node_pushReport(0x00);
      JVSIO_Node_pushReport(0x00);

      if (settings->analog_output) {
        JVSIO_Node_pushReport(0x13);  // analog output
        JVSIO_Node_pushReport(settings->analog_output);
        JVSIO_Node_pushReport(0x00);
        JVSIO_Node_pushReport(0x00);
      }

      if (settings->character_display_width) {
        JVSIO_Node_pushReport(0x14);  // character output
        JVSIO_Node_pushReport(settings->character_display_width);
        JVSIO_Node_pushReport(settings->character_display_height);
        JVSIO_Node_pushReport(0x00);  // code
      }

      JVSIO_Node_pushReport(0x00);
    } break;
    case kCmdSwInput:
      JVSIO_Node_pushReport(kReportOk);
      settings_rapid_sync();
      JVSIO_Node_pushReport(controller_head());
      for (uint8_t player = 0; player < data[1]; ++player) {
        for (uint8_t i = 0; i < data[2]; ++i) {
          JVSIO_Node_pushReport(controller_data(player, i, gpout));
        }
      }
      break;
    case kCmdCoinInput:
      JVSIO_Node_pushReport(kReportOk);
      if (data[1] <= 2) {
        for (uint8_t i = 0; i < data[1]; ++i) {
          JVSIO_Node_pushReport((0 << 6) | 0);
          JVSIO_Node_pushReport(controller_coin(i));
        }
      } else {
        Serial.println("Err CmdCoinInput");
      }
      break;
    case kCmdAnalogInput:
      JVSIO_Node_pushReport(kReportOk);
      for (uint8_t i = 0; i < data[1]; ++i) {
        uint16_t value = controller_analog(i) & settings->analog_input_mask;
        JVSIO_Node_pushReport(value >> 8);
        JVSIO_Node_pushReport(value & 0xff);
      }
      break;
    case kCmdRotaryInput:
      JVSIO_Node_pushReport(kReportOk);
      for (uint8_t i = 0; i < data[1]; ++i) {
        uint16_t value = controller_rotary(i);
        JVSIO_Node_pushReport(value >> 8);
        JVSIO_Node_pushReport(value & 0xff);
      }
      break;
    case kCmdScreenPositionInput:
      JVSIO_Node_pushReport(kReportOk);
      {
        uint8_t index = data[1] - 1;
        uint16_t x = controller_screen(index, 0) >>
                     (16 - settings->screen_position_width);
        uint16_t y = controller_screen(index, 1) >>
                     (16 - settings->screen_position_width);
        if (settings->id == IT_NAMCO_NAJV || settings->id == IT_NAMCO_TSS) {
          x = settings_adjust_x(x);
          y = settings_adjust_y(y);
        }
        JVSIO_Node_pushReport(x >> 8);
        JVSIO_Node_pushReport(x & 0xff);
        JVSIO_Node_pushReport(y >> 8);
        JVSIO_Node_pushReport(y & 0xff);
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
      JVSIO_Node_pushReport(kReportOk);
      break;
    case kCmdDriverOutput:
      gpout = data[2];
      JVSIO_Node_pushReport(kReportOk);
      break;
    case kCmdAnalogOutput:
    case kCmdCharacterOutput:
      JVSIO_Node_pushReport(kReportOk);
      break;
    default:
      handled = false;
  }
  if (commit) {
    reserved_coin_index_bias = coin_index_bias;
    reserved_coin[0] = controller_coin(0);
    reserved_coin[1] = controller_coin(1);
  }
  return handled;
}

static void detected(void) {
  led_oneshot(L_PULSE_ONCE);
}

static uint8_t get_flags(void) {
#ifdef _DBG_HUB1_ONLY
  return USE_HUB1;
#else
  return USE_HUB1 | USE_HUB0;
#endif
}

void main(void) {
  initialize();

  settings_init();
  settings = settings_get();

  delay(30);

  client_init();

  struct hid hid;
  hid.report = controller_update;
  hid.detected = detected;
  hid.get_flags = get_flags;
  hid_init(&hid);

  Serial.printf("%s\n", ids[settings->id]);

  void (*original_putc)(void) = Serial.putc;
  Serial.putc = debug_putc;

  for (;;) {
    JVSIO_Node_run(true);
    if (!JVSIO_Node_isBusy()) {
      // Serial.putc = original_putc;
      hid_poll();
      settings_poll();
      controller_poll();
      // Serial.putc = debug_putc;
#ifdef _DBG_NAMCO_TSS
      {
        Serial.putc = original_putc;
        uint16_t x =
            controller_screen(0, 0) >> (16 - settings->screen_position_width);
        uint16_t y =
            controller_screen(0, 1) >> (16 - settings->screen_position_width);
        if (settings->id == IT_NAMCO_NAJV || settings->id == IT_NAMCO_TSS) {
          x = settings_adjust_x(x);
          y = settings_adjust_y(y);
        }
        Serial.printf("%x%x,%x%x\r", (x >> 8) & 0xff, x & 0xff, (y >> 8) & 0xff,
                      y & 0xff);
        Serial.putc = debug_putc;
      }
#endif
#ifdef _DBG_ANALOG
      {
        for (uint8_t i = 0; i < 6; ++i) {
          uint16_t v = controller_analog(i);
          Serial.printf("%x%x,", (v >> 8) & 0xff, v & 0xff);
        }
        Serial.printf("\r");
      }
#endif
#ifdef _DBG_DIGITAL
      {
        static bool init = false;
        if (!init) {
          Serial.printf(
              "Ts T1 T2 T3 -- -- -- -- St Sv Up Dw Lf Rt B1 B2 B3 B4 B5 B6 B7 "
              "B8 "
              "B9 B10");
          Serial.printf("\n");
          init = true;
        }
        for (uint8_t i = 0; i < 3; ++i) {
          uint8_t v =
              (i == 0) ? controller_head() : controller_data(1, i - 1, 0);
          for (uint8_t b = 0x80; b != 0; b >>= 1) {
            Serial.printf("%d  ", (v & b) ? 1 : 0);
          }
        }
        Serial.printf("\r");
      }
#endif
    }
    client_poll();
  }
}
