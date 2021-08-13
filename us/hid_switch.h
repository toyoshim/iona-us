// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#ifndef __hid_switch_h__
#define __hid_switch_h__

#include <stdbool.h>
#include <stdint.h>

struct hub_info;
struct usb_info;
struct usb_desc_device;
struct usb_desc_interface;

bool hid_switch_check_device_desc(struct hub_info* hub_info,
                                  struct usb_info* usb_info,
                                  const struct usb_desc_device* desc);

bool hid_switch_initialize(struct hub_info* hub_info);

bool hid_switch_report(uint8_t hub,
                       struct hub_info* hub_info,
                       struct usb_info* usb_info,
                       const uint8_t* data,
                       uint16_t size);

void hid_switch_poll(uint8_t hub, struct usb_info* usb_info);

#endif  // __hid_switch_h__