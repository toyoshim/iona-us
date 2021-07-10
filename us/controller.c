// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "controller.h"

#include "chlib/ch559.h"

//#define _DBG_HID_REPORT_DUMP
//#define _DBG_JVS_BUTTON_DUMP

static uint16_t raw_map[2] = { 0, 0 };
static uint8_t jvs_map[5] = { 0, 0, 0, 0, 0 };
static uint8_t coin_sw[2] = { 0, 0 };
static uint8_t coin[2] = { 0, 0 };

static inline bool button_check(uint16_t index, const uint8_t* data) {
  if (index == 0xffff)
    return false;
  uint8_t byte = index >> 3;
  uint8_t bit = index & 7;
  return data[byte] & (1 << bit);
}

void controller_init() {
  pinMode(4, 6, INPUT_PULLUP);
  pinMode(4, 7, INPUT_PULLUP);
}

void controller_update(
    uint8_t hub, const struct hub_info* info, const uint8_t* data,
    uint16_t size, uint16_t* mask) {
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
    jvs_map[1 + hub * 2 + 0] = 0;
    jvs_map[1 + hub * 2 + 1] = 0;
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

  raw_map[hub] =
      button_check(info->button[HID_BUTTON_SELECT], data) ? (1 << B_COIN ) : 0 |
      button_check(info->button[HID_BUTTON_START ], data) ? (1 << B_START) : 0 |
      button_check(info->button[HID_BUTTON_1     ], data) ? (1 << B_1    ) : 0 |
      button_check(info->button[HID_BUTTON_2     ], data) ? (1 << B_2    ) : 0 |
      button_check(info->button[HID_BUTTON_3     ], data) ? (1 << B_3    ) : 0 |
      button_check(info->button[HID_BUTTON_4     ], data) ? (1 << B_4    ) : 0 |
      button_check(info->button[HID_BUTTON_L1    ], data) ? (1 << B_5    ) : 0 |
      button_check(info->button[HID_BUTTON_R1    ], data) ? (1 << B_6    ) : 0 |
      button_check(info->button[HID_BUTTON_L2    ], data) ? (1 << B_7    ) : 0 |
      button_check(info->button[HID_BUTTON_R2    ], data) ? (1 << B_8    ) : 0 |
      button_check(info->button[HID_BUTTON_L3    ], data) ? (1 << B_9    ) : 0 |
      button_check(info->button[HID_BUTTON_R3    ], data) ? (1 << B_10   ) : 0;

  coin_sw[hub] = (coin_sw[hub] << 1) |
                 (raw_map[hub] & mask[B_COIN]) ? 0x01: 0;
  if ((coin_sw[hub] & 3) == 1)
    coin[hub]++;

  jvs_map[1 + hub * 2 + 0] =
      ((raw_map[hub] & mask[B_START]) ? 0x80 : 0) |
      (u ? 0x20 : 0) |
      (d ? 0x10 : 0) |
      (l ? 0x08 : 0) |
      (r ? 0x04 : 0) |
      ((raw_map[hub] & mask[B_1]) ? 0x01 : 0) |
      ((raw_map[hub] & mask[B_2]) ? 0x01 : 0);
  jvs_map[1 + hub * 2 + 1] =
      ((raw_map[hub] & mask[B_3]) ? 0x80 : 0) |
      ((raw_map[hub] & mask[B_4]) ? 0x40 : 0) |
      ((raw_map[hub] & mask[B_5]) ? 0x20 : 0) |
      ((raw_map[hub] & mask[B_6]) ? 0x10 : 0) |
      ((raw_map[hub] & mask[B_7]) ? 0x08 : 0) |
      ((raw_map[hub] & mask[B_8]) ? 0x04 : 0) |
      ((raw_map[hub] & mask[B_9]) ? 0x02 : 0) |
      ((raw_map[hub] & mask[B_10]) ? 0x01 : 0);
}

void controller_poll() {
  // Service button
  if (digitalRead(4, 6) == LOW)
    jvs_map[1] |= 0x40;
  else
    jvs_map[1] &= ~0x40;
  // Test button
  if (digitalRead(4, 7) == LOW)
    jvs_map[0] |= 0x80;
  else
    jvs_map[0] &= ~0x80;
#ifdef _DBG_JVS_BUTTON_DUMP
    static uint8_t old_jvs_map[5] = { 0, 0, 0, 0, 0 };
    if (old_jvs_map[0] != jvs_map[0] || old_jvs_map[1] != jvs_map[1] ||
        old_jvs_map[2] != jvs_map[2] || old_jvs_map[3] != jvs_map[3] ||
        old_jvs_map[4] != jvs_map[4]) {
      for (uint8_t i = 0; i < 5; ++i)
        old_jvs_map[i] = jvs_map[i];
      Serial.printc(jvs_map[0], BIN);
      Serial.putc('_');
      Serial.printc(jvs_map[1], BIN);
      Serial.putc('_');
      Serial.printc(jvs_map[2], BIN);
      Serial.putc('_');
      Serial.printc(jvs_map[3], BIN);
      Serial.putc('_');
      Serial.printc(jvs_map[4], BIN);
      Serial.println("");
    }
#endif  // _DBG_JVS_BUTTON_DUMP
}

uint16_t controller_raw(uint8_t player) {
  return raw_map[player];
}

uint8_t controller_jvs(uint8_t index) {
  return jvs_map[index];
}

uint8_t controller_coin(uint8_t player) {
  return coin[player];
}

void controller_coin_add(uint8_t player, uint8_t add) {
  coin[player] += add;
}

void controller_coin_sub(uint8_t player, uint8_t sub) {
  coin[player] -= sub;
}

