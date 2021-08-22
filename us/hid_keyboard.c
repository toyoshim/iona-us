// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "hid_keyboard.h"

#include "chlib/usb.h"
#include "hid.h"

bool hid_keyboard_check_device_desc(struct hub_info* hub_info,
                                    const struct usb_desc_device* desc) {
  if (desc->bDeviceClass == USB_CLASS_HID &&
      desc->bDeviceSubClass == USB_HID_SUBCLASS_BOOT &&
      desc->bDeviceProtocol == USB_HID_BOOT_PROTOCOL_KEYBOARD) {
    hub_info->type = HID_TYPE_KEYBOARD;
    return true;
  }
  return false;
}

bool hid_keyboard_check_interface_desc(struct hub_info* hub_info,
                                       const struct usb_desc_interface* desc) {
  if (desc->bInterfaceClass == USB_CLASS_HID &&
      desc->bInterfaceSubClass == USB_HID_SUBCLASS_BOOT &&
      desc->bInterfaceProtocol == USB_HID_BOOT_PROTOCOL_KEYBOARD) {
    hub_info->type = HID_TYPE_KEYBOARD;
    return true;
  }
  return false;
}

bool hid_keyboard_initialize(struct hub_info* hub_info) {
  if (hub_info->type != HID_TYPE_KEYBOARD)
    return false;
  hub_info->state = HID_STATE_READY;
  hub_info->report_size = 8;
  hub_info->report_id = 0;
  return true;
}