// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "hid_switch.h"

#include "chlib/usb.h"
#include "hid.h"
#include "hid_internal.h"

enum {
  SWITCH_CONNECTED = 0,
};

bool hid_switch_check_device_desc(struct hub_info* hub_info,
                                  struct usb_info* usb_info,
                                  const struct usb_desc_device* desc) {
  if (desc->idVendor == 0x057e) {
    if (desc->idProduct == 0x2009 || desc->idProduct == 0x200e) {
      // Nintendo Switch Pro Controller, and Charging Grip.
      hub_info->type = HID_TYPE_SWITCH;
      usb_info->state = SWITCH_CONNECTED;
      return true;
    }
  }
  return false;
}

bool hid_switch_report(struct hub_info* hub_info,
                       const uint8_t* data,
                       uint16_t size) {
  hub_info;
  data;
  size;
  return false;
}

void hid_switch_poll(uint8_t hub,
                     struct hub_info* hub_info,
                     struct usb_info* usb_info) {
  hub;
  hub_info;
  usb_info;
}
