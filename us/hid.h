// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#ifndef __hid_h__
#define __hid_h__

#include "chlib/usb_host.h"

enum {
  HID_TYPE_UNKNOWN,
  HID_TYPE_PS4,
};

enum {
  HID_STATE_DISCONNECTED,
  HID_STATE_CONNECTED,
  HID_STATE_NOT_READY,
  HID_STATE_READY,
};

struct hub_info {
  uint16_t report_desc_size;
  uint16_t report_size;
  uint16_t axis[2];
  uint16_t dpad;
  uint16_t button[12];
  uint8_t axis_size[2];
  uint8_t report_id;
  uint8_t type;
  uint8_t ep;
  uint8_t state;
};

struct hid {
  void (*report)(uint8_t hub, const struct hub_info*, const uint8_t* data);
};

void hid_init(struct hid* hid);
struct hub_info* hid_get_info(uint8_t hub);
void hid_poll();

#endif  // __hid_h__
