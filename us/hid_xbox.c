// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "hid_xbox.h"

#include "chlib/usb.h"
#include "hid.h"
#include "hid_internal.h"

// state
enum {
  CONNECTED,
  INITIALIZED,
};

bool hid_xbox_check_device_desc(struct hub_info* hub_info,
                                const struct usb_desc_device* desc) {
  if (desc->idVendor == 0x045e) {
    // Microsoft Xbox 360 / ONE official controllers.
    if (desc->idProduct == 0x028e) {
      hub_info->type = HID_TYPE_XBOX_360;
      hub_info->report_desc_size = 1;
      return true;
    }
    if (desc->idProduct == 0x02d1 || desc->idProduct == 0x02dd ||
        desc->idProduct == 0x02e3 || desc->idProduct == 0x02ea ||
        desc->idProduct == 0x0b00 || desc->idProduct == 0x0b0a ||
        desc->idProduct == 0x0b12) {
      hub_info->type = HID_TYPE_XBOX_ONE;
      hub_info->report_desc_size = 1;
      return true;
    }
  } else if (desc->bDeviceClass == 0xff && desc->bDeviceSubClass == 0x47 &&
             desc->bDeviceProtocol == 0xd0) {
    // Might be a Xbox One compatible controller.
    hub_info->type = HID_TYPE_XBOX_ONE;
    hub_info->report_desc_size = 1;
    return true;
  }
  return false;
}

bool hid_xbox_360_check_interface_desc(struct hub_info* hub_info,
                                       const struct usb_desc_interface* intf) {
  if (intf->bInterfaceClass == 0xff && intf->bInterfaceSubClass == 0x5d &&
      intf->bInterfaceProtocol == 0x01) {
    // Might be a Xbox 360 compatible controller.
    hub_info->type = HID_TYPE_XBOX_360;
    hub_info->report_desc_size = 1;
    return true;
  }
  return false;
}

bool hid_xbox_initialize(struct hub_info* hub_info, struct usb_info* usb_info) {
  if (hub_info->state != HID_STATE_NOT_READY)
    return false;

  if (hub_info->type != HID_TYPE_XBOX_360 &&
      hub_info->type != HID_TYPE_XBOX_ONE) {
    return false;
  }

  hub_info->state = HID_STATE_READY;
  usb_info->state = CONNECTED;
  usb_info->cmd_count = 0;

  if (hub_info->type == HID_TYPE_XBOX_360) {
    // https://github.com/xboxdrv/xboxdrv/blob/stable/PROTOCOL
    hub_info->report_size = 20 * 8;
    hub_info->axis[0] = 6 * 8;
    hub_info->axis_size[0] = 16;
    hub_info->axis_sign[0] = true;
    hub_info->axis_polarity[0] = false;
    hub_info->axis[1] = 8 * 8;
    hub_info->axis_size[1] = 16;
    hub_info->axis_sign[1] = true;
    hub_info->axis_polarity[1] = true;
    hub_info->hat = 0xffff;
    hub_info->dpad[0] = 16 + 0;
    hub_info->dpad[1] = 16 + 1;
    hub_info->dpad[2] = 16 + 2;
    hub_info->dpad[3] = 16 + 3;
    hub_info->button[0] = 24 + 6;
    hub_info->button[1] = 24 + 4;
    hub_info->button[2] = 24 + 5;
    hub_info->button[3] = 24 + 7;
    hub_info->button[4] = 24 + 0;
    hub_info->button[5] = 24 + 1;
    hub_info->button[6] = 39;
    hub_info->button[7] = 47;
    hub_info->button[8] = 16 + 5;
    hub_info->button[9] = 16 + 4;
    hub_info->button[10] = 16 + 6;
    hub_info->button[11] = 16 + 7;
    hub_info->report_id = 0;
  } else {
    // https://github.com/quantus/xbox-one-controller-protocol
    hub_info->report_size = 18 * 8;
    hub_info->axis[0] = 10 * 8;
    hub_info->axis_size[0] = 16;
    hub_info->axis_sign[0] = true;
    hub_info->axis_polarity[0] = false;
    hub_info->axis[1] = 12 * 8;
    hub_info->axis_size[1] = 16;
    hub_info->axis_sign[1] = true;
    hub_info->axis_polarity[1] = true;
    hub_info->hat = 0xffff;
    hub_info->dpad[0] = 40 + 0;
    hub_info->dpad[1] = 40 + 1;
    hub_info->dpad[2] = 40 + 2;
    hub_info->dpad[3] = 40 + 3;
    hub_info->button[0] = 32 + 6;
    hub_info->button[1] = 32 + 4;
    hub_info->button[2] = 32 + 5;
    hub_info->button[3] = 32 + 7;
    hub_info->button[4] = 40 + 4;
    hub_info->button[5] = 40 + 5;
    hub_info->button[6] = 57;
    hub_info->button[7] = 73;
    hub_info->button[8] = 32 + 3;
    hub_info->button[9] = 32 + 2;
    hub_info->button[10] = 40 + 6;
    hub_info->button[11] = 40 + 7;
    hub_info->report_id = 0;
  }
  return true;
}

bool hid_xbox_report(struct hub_info* hub_info,
                     const uint8_t* data,
                     uint16_t size) {
  if (!size)
    return false;
  if (hub_info->type == HID_TYPE_XBOX_360 && size != 20 && data[0] != 0x00)
    return true;
  if (hub_info->type == HID_TYPE_XBOX_ONE && size != 18 && data[0] != 0x20)
    return true;
  return false;
}

void hid_xbox_360_poll(uint8_t hub,
                       struct hub_info* hub_info,
                       struct usb_info* usb_info) {
  if (usb_info->state == CONNECTED) {
    static uint8_t initialize[] = {0x01, 0x03, 0x00};
    initialize[2] = 0x02 + hub;
    usb_host_out(hub, usb_info->ep, initialize, sizeof(initialize));
    usb_info->state = INITIALIZED;
  } else if (usb_info->state == INITIALIZED) {
    usb_host_in(hub, hub_info->ep, 20);
  }
}

void hid_xbox_one_poll(uint8_t hub,
                       struct hub_info* hub_info,
                       struct usb_info* usb_info) {
  if (usb_info->state == CONNECTED) {
    static uint8_t initialize[] = {0x05, 0x20, 0x00, 0x01, 0x00};
    initialize[2] = usb_info->cmd_count++;
    usb_host_out(hub, usb_info->ep, initialize, sizeof(initialize));
    usb_info->state = INITIALIZED;
  } else if (usb_info->state == INITIALIZED) {
    usb_host_in(hub, hub_info->ep, usb_info->ep_max_packet_size);
  }
}
