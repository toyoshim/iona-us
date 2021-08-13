// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#ifndef __hid_xbox_h__
#define __hid_xbox_h__

#include <stdbool.h>
#include <stdint.h>

struct hub_info;
struct usb_info;
struct usb_desc_device;
struct usb_desc_interface;

bool hid_xbox_check_device_desc(struct hub_info* hub_info,
                                const struct usb_desc_device* desc);

bool hid_xbox_check_interface_desc(struct hub_info* hub_info,
                                   const struct usb_desc_interface* intf);

bool hid_xbox_initialize(struct hub_info* hub_info, struct usb_info* usb_info);

bool hid_xbox_report(struct hub_info* hub_info,
                     const uint8_t* data,
                     uint16_t size);

void hid_xbox_360_poll(uint8_t hub,
                       struct hub_info* hub_info,
                       struct usb_info* usb_info);

void hid_xbox_one_poll(uint8_t hub,
                       struct hub_info* hub_info,
                       struct usb_info* usb_info);

#endif  // __hid_xbox_h__