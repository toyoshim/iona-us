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
  if (index == 0xffff) {
    return false;
  }
  uint8_t byte = index >> 3;
  uint8_t bit = index & 7;
  return data[byte] & (1 << bit);
}

uint16_t analog_check(const struct hid_info* info,
                      const uint8_t* data,
                      uint8_t index,
                      bool polarity) {
  if (info->axis[index] == 0xffff) {
    // return 0x8000;
  } else if (info->axis_size[index] == 8) {
    uint8_t v = data[info->axis[index] >> 3];
    v <<= info->axis_shift[index];
    if (info->axis_sign[index]) {
      v += 0x80;
    }
    if (info->axis_polarity[index] ^ polarity) {
      v = 0xff - v;
    }
    return v << 8;
  } else if (info->axis_size[index] == 10 || info->axis_size[index] == 12) {
    uint8_t byte_index = info->axis[index] >> 3;
    uint16_t l = data[byte_index + 0];
    uint16_t h = data[byte_index + 1];
    uint16_t v = (((h << 8) | l) >> (info->axis[index] & 7))
                 << (16 - info->axis_size[index]);
    v <<= info->axis_shift[index];
    if (info->axis_sign[index]) {
      v += 0x8000;
    }
    if (info->axis_polarity[index] ^ polarity) {
      v = 0xffff - v;
    }
    return v;
  } else if (info->axis_size[index] == 16) {
    uint8_t byte = info->axis[index] >> 3;
    uint16_t v = data[byte] | ((uint16_t)data[byte + 1] << 8);
    v <<= info->axis_shift[index];
    if (info->axis_sign[index]) {
      v += 0x8000;
    }
    if (info->axis_polarity[index] ^ polarity) {
      v = 0xffff - v;
    }
    return v;
  }
  return 0x8000;
}

static void mahjong_update(const uint8_t* data) {
  uint8_t i;
  for (i = 0; i < 4; ++i) {
    mahjong[i] = 0;
  }
  if (data[0] & 0x11) {
    // Ctrl: Kan
    mahjong[0] |= 0x04;
  }
  if (data[0] & 0x22) {
    // Shift: Reach
    mahjong[1] |= 0x04;
  }
  if (data[0] & 0x44) {
    // Alt: Pon
    mahjong[3] |= 0x08;
  }
  bool coin_key = false;
  for (i = 2; i < 8; ++i) {
    if (0x04 <= data[i] && data[i] <= 0x07) {
      // A-D
      mahjong[data[i] - 0x04] |= 0x80;
    } else if (0x08 <= data[i] && data[i] <= 0x0b) {
      // E-H
      mahjong[data[i] - 0x08] |= 0x20;
    } else if (0x0c <= data[i] && data[i] <= 0x0f) {
      // I-L
      mahjong[data[i] - 0x0c] |= 0x10;
    } else if (0x10 <= data[i] && data[i] <= 0x11) {
      // M-N
      mahjong[data[i] - 0x10] |= 0x08;
    } else if (data[i] == 0x2c) {
      // Space: Chi
      mahjong[2] |= 0x08;
    } else if (data[i] == 0x1d) {
      // Z: Ron
      mahjong[2] |= 0x04;
    } else if (data[i] == 0x1e) {
      // 1: Start
      mahjong[0] |= 0x02;
    } else if (data[i] == 0x22) {
      // 5: Coin
      coin_key = true;
    }
  }
  coin_sw[0] = (coin_sw[0] << 1) | (coin_key ? 1 : 0);
  if ((coin_sw[0] & 3) == 1) {
    coin[0]++;
  }
}

static void controller_reset_digital_map(uint8_t player) {
  for (uint8_t i = 0; i < 4; ++i) {
    digital_map[player][i] = 0;
  }
}

static void update_digital_map(uint8_t* dst, uint8_t* src, bool on) {
  if (!on) {
    return;
  }
  for (uint8_t i = 0; i < 4; ++i) {
    dst[i] |= src[i];
  }
}

void controller_reset(void) {
  for (uint8_t p = 0; p < 2; ++p) {
    controller_reset_digital_map(p);
    gear_sequence[p] = 0;
    gear[p] = 0;
  }
  for (uint8_t i = 0; i < 8; ++i) {
    analog[i] = 0;
  }
  for (uint8_t i = 0; i < 2; ++i) {
    rotary[i] = 0;
  }
  for (uint8_t i = 0; i < 4; ++i) {
    screen[i] = 0;
  }
}

void controller_update(uint8_t hub_index,
                       const struct hid_info* info,
                       const uint8_t* data,
                       uint16_t size) {
#ifdef _DBG_HUB1_ONLY
  const uint8_t hub = 0;
  hub_index;
#else
  const uint8_t hub = hub_index;
#endif
#ifdef _DBG_HID_REPORT_DUMP
  static uint8_t old_data[256];
  bool modified = false;
  for (uint8_t i = 0; i < size; ++i) {
    if (old_data[i] == data[i]) {
      continue;
    }
    modified = true;
    old_data[i] = data[i];
  }
  if (!modified) {
    return;
  }
  Serial.printf("Report %d Bytes: ", size);
  for (uint8_t i = 0; i < size; ++i) {
    Serial.printf("%x,", data[i]);
  }
  Serial.println("");
#endif  // _DBG_HID_REPORT_DUMP
#ifdef _DBG_HID_DECODE_DUMP
  for (uint8_t i = 0; i < 6; ++i) {
    uint16_t value = analog_check(
        info, data, i, settings_get()->analog_polarity[hub][i]);
    Serial.printf("analog %d: %x%x\n", i, value >> 8, value & 0xff);
  }
  Serial.printf("digital: ");
  for (uint8_t i = 0; i < 13; ++i) {
    Serial.printf("%d ", button_check(info->button[i], data) ? 1 : 0);
  }
  Serial.println("");
#endif  // _DBG_HID_DECODE_DUMP
  controller_reset_digital_map(hub);

  if (info->state != HID_STATE_READY) {
    return;
  }

  if (info->type == HID_TYPE_KEYBOARD) {
    if (size == 8) {
      mode = MODE_MAHJONG;
      mahjong_update(data);
    }
    return;
  } else if (mode == MODE_MAHJONG) {
    mode = MODE_NORMAL;
  }

  if (info->report_id) {
    if (info->report_id != data[0]) {
      return;
    }
    data++;
  }

  struct settings* settings = settings_get();
  // Analog to Digital pad map from another controller.
  bool u = button_check(info->dpad[0], data);
  bool d = button_check(info->dpad[1], data);
  bool l = button_check(info->dpad[2], data);
  bool r = button_check(info->dpad[3], data);

  uint8_t alt_digital = 0;
  for (uint8_t i = 0; i < 6; ++i) {
    uint8_t type = settings->analog_type[hub][i];
    if (type == AT_NONE) {
      continue;
    }
    uint8_t index = settings->analog_index[hub][i];
    uint16_t value =
        analog_check(info, data, i, settings->analog_polarity[hub][i]);
    switch (type) {
      case AT_DIGITAL:
        switch (index) {
          case 0:
            l |= value < 0x6000;
            r |= value > 0xa000;
            break;
          case 1:
            u |= value < 0x6000;
            d |= value > 0xa000;
            break;
          case 2:
            alt_digital |= (value < 0x6000) ? 4 : (value > 0xa000) ? 8 : 0;
            break;
          case 3:
            alt_digital |= (value < 0x6000) ? 1 : (value > 0xa000) ? 2 : 0;
            break;
        }
        break;
      case AT_ANALOG:
        analog[index] = value;
        break;
      case AT_ROTARY:
        rotary[index] = value;
        break;
      case AT_SCREEN:
        screen[index] = value;
        break;
    }
  }

  if (info->hat != 0xffff) {
    uint8_t byte = info->hat >> 3;
    uint8_t bit = info->hat & 7;
    uint8_t hat = (data[byte] >> bit) & 0xf;
    switch (hat) {
      case 0:
        u |= true;
        break;
      case 1:
        u |= true;
        r |= true;
        break;
      case 2:
        r |= true;
        break;
      case 3:
        r |= true;
        d |= true;
        break;
      case 4:
        d |= true;
        break;
      case 5:
        d |= true;
        l |= true;
        break;
      case 6:
        l |= true;
        break;
      case 7:
        l |= true;
        u |= true;
        break;
    }
  }

  update_digital_map(digital_map[hub], settings->digital_map[hub][0].data, u);
  update_digital_map(digital_map[hub], settings->digital_map[hub][1].data, d);
  update_digital_map(digital_map[hub], settings->digital_map[hub][2].data, l);
  update_digital_map(digital_map[hub], settings->digital_map[hub][3].data, r);
  if (alt_digital) {
    uint8_t alt_hub = (hub + 1) & 1;
    update_digital_map(digital_map[hub], settings->digital_map[alt_hub][0].data,
                       alt_digital & 1);
    update_digital_map(digital_map[hub], settings->digital_map[alt_hub][1].data,
                       alt_digital & 2);
    update_digital_map(digital_map[hub], settings->digital_map[alt_hub][2].data,
                       alt_digital & 4);
    update_digital_map(digital_map[hub], settings->digital_map[alt_hub][3].data,
                       alt_digital & 8);
  }
  for (uint8_t i = 0; i < 12; ++i) {
    uint8_t rapid_fire = settings->rapid_fire[hub][i];
    update_digital_map(digital_map[hub], settings->digital_map[hub][4 + i].data,
                       (settings->sequence[rapid_fire].on &&
                        button_check(info->button[i], data)) ^
                           settings->sequence[rapid_fire].invert);
  }
  if (settings->gear_sequence_support[hub]) {
    uint8_t current_gear = 0;
    for (uint8_t i = 0; i < 12; ++i) {
      if (settings->gear_control[hub][i] == 0) {
        continue;
      }
      if (button_check(info->button[i], data)) {
        current_gear = settings->gear_control[hub][i];
      }
    }
    if (gear[hub] != current_gear) {
      gear[hub] = current_gear;
      if (current_gear == 1) {
        if (gear_sequence[hub] < 5) {
          gear_sequence[hub]++;
        }
      } else if (current_gear == 2) {
        if (gear_sequence[hub] > 0) {
          gear_sequence[hub]--;
        }
      }
    }
    switch (gear_sequence[hub]) {
      case 0:
        digital_map[hub][1] |= 0xa0;  // 1010_0000
        break;
      case 1:
        digital_map[hub][1] |= 0x60;  // 0110_0000
        break;
      case 2:
        digital_map[hub][1] |= 0x80;  // 1000_0000
        break;
      case 3:
        digital_map[hub][1] |= 0x40;  // 0100_0000
        break;
      case 4:
        digital_map[hub][1] |= 0x90;  // 1001_0000
        break;
      case 5:
        digital_map[hub][1] |= 0x50;  // 0101_0000
        break;
    }
  }

  coin_sw[hub] = (coin_sw[hub] << 1) | ((digital_map[hub][0] >> 6) & 1);
  if ((coin_sw[hub] & 3) == 1) {
    coin[hub]++;
  }
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
    if (gpout == 0x40)
      return mahjong[0] | service;
    if (gpout == 0x20)
      return mahjong[1] | service;
    if (gpout == 0x10)
      return mahjong[2] | service;
    if (gpout == 0x80 || gpout == 0x08)
      return mahjong[3] | service;
    return service;
  }
  uint8_t line = (player << 1) + index;
  if (line >= 4) {
    return 0;
  }
  uint8_t data = digital_map[0][line] | digital_map[1][line];
  if (!line) {
    data &= ~0x40;
    if (!player && service_sw) {
      data |= 0x40;
    }
  }
  return data;
}

uint8_t controller_coin(uint8_t player) {
  return coin[player];
}

uint16_t controller_analog(uint8_t index) {
  if (index < 8) {
    return analog[index];
  }
  return 0x8000;
}

uint16_t controller_rotary(uint8_t index) {
  if (index < 2) {
    return rotary[index];
  }
  return 0x8000;
}

uint16_t controller_screen(uint8_t index, uint8_t axis) {
  uint8_t screen_index = (index << 1) + axis;
  if (screen_index < 4) {
    return screen[screen_index];
  }
  return 0x8000;
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