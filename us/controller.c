// Copyright 2023 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "controller.h"

#include "ch559.h"
#include "serial.h"
#include "settings.h"

// #define _DBG_HID_REPORT_DUMP
// #define _DBG_HID_DECODE_DUMP
// #define _DBG_HUB1_ONLY

static bool test_sw = false;
static bool service_sw = false;
static uint8_t coin_sw[2] = {0, 0};
static uint8_t coin[2] = {0, 0};
static uint8_t mahjong[4] = {0, 0, 0, 0};

static uint8_t digital_map[2][4];

static uint16_t analog[8];
static uint16_t rotary[2];
static uint16_t screen[4];
static uint8_t gear_sequence[2];
static uint8_t gear[2];

enum {
  MODE_NORMAL,
  MODE_MAHJONG,
};
static uint8_t mode = MODE_NORMAL;

static bool button_check(uint16_t index, const uint8_t* data) {
  if (index == 0xffff) return false;
  uint8_t byte = index >> 3;
  uint8_t bit = index & 7;
  return data[byte] & (1 << bit);
}

uint16_t analog_check(const struct hid_info* info,
                      const uint8_t* data,
                      uint8_t index,
                      bool polarity) {
  if (info->axis[index] == 0xffff) {
    return 0x8000;
  } else if (info->axis_size[index] == 8) {
    uint8_t v = data[info->axis[index] >> 3];
    v <<= info->axis_shift[index];
    if (info->axis_sign[index]) v += 0x80;
    if (info->axis_polarity[index] ^ polarity) v = 0xff - v;
    return v << 8;
  } else if (info->axis_size[index] == 10 || info->axis_size[index] == 12) {
    uint8_t byte_index = info->axis[index] >> 3;
    uint16_t l = data[byte_index + 0];
    uint16_t h = data[byte_index + 1];
    uint16_t v = (((h << 8) | l) >> (info->axis[index] & 7))
                 << (16 - info->axis_size[index]);
    v <<= info->axis_shift[index];
    if (info->axis_sign[index]) v += 0x8000;
    if (info->axis_polarity[index] ^ polarity) v = 0xffff - v;
    return v;
  } else if (info->axis_size[index] == 16) {
    uint8_t byte = info->axis[index] >> 3;
    uint16_t v = data[byte] | ((uint16_t)data[byte + 1] << 8);
    v <<= info->axis_shift[index];
    if (info->axis_sign[index]) v += 0x8000;
    if (info->axis_polarity[index] ^ polarity) v = 0xffff - v;
    return v;
  }
  return 0x8000;
}

static void mahjong_update(const uint8_t* data) {
  for (uint8_t i = 0; i < 4; ++i) mahjong[i] = 0;
  if (data[0] & 0x11) mahjong[0] |= 0x04;
  if (data[0] & 0x22) mahjong[1] |= 0x04;
  if (data[0] & 0x44) mahjong[3] |= 0x08;

  bool coin_key = false;
  for (uint8_t i = 2; i < 8; ++i) {
    if (0x04 <= data[i] && data[i] <= 0x07) mahjong[data[i] - 0x04] |= 0x80;
    else if (0x08 <= data[i] && data[i] <= 0x0b) mahjong[data[i] - 0x08] |= 0x20;
    else if (0x0c <= data[i] && data[i] <= 0x0f) mahjong[data[i] - 0x0c] |= 0x10;
    else if (0x10 <= data[i] && data[i] <= 0x11) mahjong[data[i] - 0x10] |= 0x08;
    else if (data[i] == 0x2c) mahjong[2] |= 0x08;
    else if (data[i] == 0x1d) mahjong[2] |= 0x04;
    else if (data[i] == 0x1e) mahjong[0] |= 0x02;
    else if (data[i] == 0x22) coin_key = true;
  }
  coin_sw[0] = (coin_sw[0] << 1) | (coin_key ? 1 : 0);
  if ((coin_sw[0] & 3) == 1) coin[0]++;
}

static void controller_reset_digital_map(uint8_t player) {
  for (uint8_t i = 0; i < 4; ++i) digital_map[player][i] = 0;
}

static void update_digital_map(uint8_t* dst, uint8_t* src, bool on) {
  if (!on) return;
  for (uint8_t i = 0; i < 4; ++i) dst[i] |= src[i];
}

void controller_reset(void) {
  for (uint8_t p = 0; p < 2; ++p) {
    controller_reset_digital_map(p);
    gear_sequence[p] = 0;
    gear[p] = 0;
  }
  for (uint8_t i = 0; i < 8; ++i) analog[i] = 0;
  for (uint8_t i = 0; i < 2; ++i) rotary[i] = 0;
  for (uint8_t i = 0; i < 4; ++i) screen[i] = 0;
}

void controller_update(uint8_t hub_index,
                       const struct hid_info* info,
                       const uint8_t* data,
                       uint16_t size) {
  if (hub_index >= 2) return;
  const uint8_t player = hub_index;

#ifdef _DBG_HID_REPORT_DUMP
  static uint8_t old_data[256];
  bool modified = false;
  for (uint8_t i = 0; i < size; ++i) {
    if (old_data[i] == data[i]) continue;
    modified = true;
    old_data[i] = data[i];
  }
  if (!modified) return;
  Serial.printf("Report %d Bytes: ", size);
  for (uint8_t i = 0; i < size; ++i) Serial.printf("%x,", data[i]);
  Serial.println("");
#endif

  controller_reset_digital_map(player);
  if (info->state != HID_STATE_READY) return;

  if (info->type == HID_TYPE_KEYBOARD) {
    if (size == 8) {
      mode = MODE_MAHJONG;
      mahjong_update(data);
    }
    return;
  } else if (mode == MODE_MAHJONG) {
    mode = MODE_NORMAL;
  }

  if (info->report_id && info->report_id != data[0]) return;
  if (info->report_id) data++;

  struct settings* settings = settings_get();

  bool u = button_check(info->dpad[0], data);
  bool d = button_check(info->dpad[1], data);
  bool l = button_check(info->dpad[2], data);
  bool r = button_check(info->dpad[3], data);

  uint8_t alt_digital = 0;
  for (uint8_t i = 0; i < 6; ++i) {
    uint8_t type = settings->analog_type[player][i];
    if (type == AT_NONE) continue;
    uint8_t index = settings->analog_index[player][i];
    uint16_t value = analog_check(info, data, i, settings->analog_polarity[player][i]);

    switch (type) {
      case AT_DIGITAL:
        switch (index) {
          case 0: l |= value < 0x6000; r |= value > 0xa000; break;
          case 1: u |= value < 0x6000; d |= value > 0xa000; break;
          case 2: alt_digital |= (value < 0x6000) ? 4 : (value > 0xa000) ? 8 : 0; break;
          case 3: alt_digital |= (value < 0x6000) ? 1 : (value > 0xa000) ? 2 : 0; break;
        }
        break;
      case AT_ANALOG: analog[index] = value; break;
      case AT_ROTARY: rotary[index] = value; break;
      case AT_SCREEN: screen[index] = value; break;
    }
  }

  if (info->hat != 0xffff) {
    uint8_t byte = info->hat >> 3;
    uint8_t bit = info->hat & 7;
    uint8_t hat = (data[byte] >> bit) & 0xf;
    if (hat == 0 || hat == 1 || hat == 7) u = true;
    if (hat == 1 || hat == 2 || hat == 3) r = true;
    if (hat == 3 || hat == 4 || hat == 5) d = true;
    if (hat == 5 || hat == 6 || hat == 7) l = true;
  }

  update_digital_map(digital_map[player], settings->digital_map[player][0].data, u);
  update_digital_map(digital_map[player], settings->digital_map[player][1].data, d);
  update_digital_map(digital_map[player], settings->digital_map[player][2].data, l);
  update_digital_map(digital_map[player], settings->digital_map[player][3].data, r);

  if (alt_digital) {
    uint8_t alt_player = (player + 1) & 1;
    update_digital_map(digital_map[player], settings->digital_map[alt_player][0].data, alt_digital & 1);
    update_digital_map(digital_map[player], settings->digital_map[alt_player][1].data, alt_digital & 2);
    update_digital_map(digital_map[player], settings->digital_map[alt_player][2].data, alt_digital & 4);
    update_digital_map(digital_map[player], settings->digital_map[alt_player][3].data, alt_digital & 8);
  }

  for (uint8_t i = 0; i < 12; ++i) {
    uint8_t rf = settings->rapid_fire[player][i];
    update_digital_map(digital_map[player], settings->digital_map[player][4 + i].data,
                       (settings->sequence[rf].on && button_check(info->button[i], data)) ^ settings->sequence[rf].invert);
  }

  if (settings->gear_sequence_support[player]) {
    uint8_t current = 0;
    for (uint8_t i = 0; i < 12; ++i) {
      if (!settings->gear_control[player][i]) continue;
      if (button_check(info->button[i], data)) current = settings->gear_control[player][i];
    }

    if (gear[player] != current) {
      gear[player] = current;
      if (current == 1 && gear_sequence[player] < 5) gear_sequence[player]++;
      else if (current == 2 && gear_sequence[player] > 0) gear_sequence[player]--;
    }

    static const uint8_t gear_bits[6] = {0xa0, 0x60, 0x80, 0x40, 0x90, 0x50};
    if (gear_sequence[player] < 6)
      digital_map[player][1] |= gear_bits[gear_sequence[player]];
  }

  coin_sw[player] = (coin_sw[player] << 1) | ((digital_map[player][0] >> 6) & 1);
  if ((coin_sw[player] & 3) == 1) coin[player]++;
}

void controller_poll(void) {
  service_sw = settings_service_pressed();
  test_sw = settings_test_pressed();
}

uint8_t controller_head(void) {
  return test_sw ? 0x80 : 0;
}

uint8_t controller_data(uint8_t player, uint8_t index, uint8_t gpout) {
  if (mode == MODE_MAHJONG) {
    uint8_t service = service_sw ? 0x40 : 0;
    if (gpout == 0x40) return mahjong[0] | service;
    if (gpout == 0x20) return mahjong[1] | service;
    if (gpout == 0x10) return mahjong[2] | service;
    if (gpout == 0x80 || gpout == 0x08) return mahjong[3] | service;
    return service;
  }
  if (player >= 2) return 0;
  uint8_t line = (player << 1) + index;
  if (line >= 4) return 0;
  uint8_t data = digital_map[player][line];
  if (!line) {
    data &= ~0x40;
    if (service_sw && !player) data |= 0x40;
  }
  return data;
}

uint8_t controller_coin(uint8_t player) {
  return coin[player];
}

uint16_t controller_analog(uint8_t index) {
  return index < 8 ? analog[index] : 0x8000;
}

uint16_t controller_rotary(uint8_t index) {
  return index < 2 ? rotary[index] : 0x8000;
}

uint16_t controller_screen(uint8_t index, uint8_t axis) {
  uint8_t screen_index = (index << 1) + axis;
  return screen_index < 4 ? screen[screen_index] : 0x8000;
}

void controller_coin_add(uint8_t player, uint8_t add) {
  coin[player] += add;
}

void controller_coin_sub(uint8_t player, uint8_t sub) {
  coin[player] -= sub;
}

void controller_coin_set(uint8_t player, uint8_t value) {
  coin[player] = value;
}
