// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "hid_guncon3.h"

#include "chlib/serial.h"
#include "hid.h"
#include "hid_internal.h"

enum {
  IDLE = 0,

  HUB_CONNECTED,
  HUB_GET_PORT_STATUS,
  HUB_PORT_RESET,
  HUB_PORT_RESET_WAIT,
  HUB_PORT_RESET_DONE,
  HUB_PORT_SET_ADDRESS,
  HUB_PORT_SET_ADDRESS_DONE,

  DEVICE_CONNECTED,
  DEVICE_READY,
};

// Based on information at https://www.beardypig.com/2016/01/06/guncon3/.
static uint8_t key[8] = {
    0x01, 0x12, 0x6f, 0x32, 0x24, 0x60, 0x17, 0x21,
};

static const uint8_t table[320] = {
    0x75, 0xc3, 0x10, 0x31, 0xb5, 0xd3, 0x69, 0x84, 0x89, 0xba, 0xd6, 0x89,
    0xbd, 0x70, 0x19, 0x8e, 0x58, 0xa8, 0x3d, 0x9b, 0x5d, 0xf0, 0x49, 0xe8,
    0xad, 0x9d, 0x7a, 0x0d, 0x7e, 0x24, 0xda, 0xfc, 0x0d, 0x14, 0xc5, 0x23,
    0x91, 0x11, 0xf5, 0xc0, 0x4b, 0xcd, 0x44, 0x1c, 0xc5, 0x21, 0xdf, 0x61,
    0x54, 0xed, 0xa2, 0x81, 0xb7, 0xe5, 0x74, 0x94, 0xb0, 0x47, 0xee, 0xf1,
    0xa5, 0xbb, 0x21, 0xc8, 0x91, 0xfd, 0x4c, 0x8b, 0x20, 0xc1, 0x7c, 0x09,
    0x58, 0x14, 0xf6, 0x00, 0x52, 0x55, 0xbf, 0x41, 0x75, 0xc0, 0x13, 0x30,
    0xb5, 0xd0, 0x69, 0x85, 0x89, 0xbb, 0xd6, 0x88, 0xbc, 0x73, 0x18, 0x8d,
    0x58, 0xab, 0x3d, 0x98, 0x5c, 0xf2, 0x48, 0xe9, 0xac, 0x9f, 0x7a, 0x0c,
    0x7c, 0x25, 0xd8, 0xff, 0xdc, 0x7d, 0x08, 0xdb, 0xbc, 0x18, 0x8c, 0x1d,
    0xd6, 0x3c, 0x35, 0xe1, 0x2c, 0x14, 0x8e, 0x64, 0x83, 0x39, 0xb0, 0xe4,
    0x4e, 0xf7, 0x51, 0x7b, 0xa8, 0x13, 0xac, 0xe9, 0x43, 0xc0, 0x08, 0x25,
    0x0e, 0x15, 0xc4, 0x20, 0x93, 0x13, 0xf5, 0xc3, 0x48, 0xcc, 0x47, 0x1c,
    0xc5, 0x20, 0xde, 0x60, 0x55, 0xee, 0xa0, 0x40, 0xb4, 0xe7, 0x74, 0x95,
    0xb0, 0x46, 0xec, 0xf0, 0xa5, 0xb8, 0x23, 0xc8, 0x04, 0x06, 0xfc, 0x28,
    0xcb, 0xf8, 0x17, 0x2c, 0x25, 0x1c, 0xcb, 0x18, 0xe3, 0x6c, 0x80, 0x85,
    0xdd, 0x7e, 0x09, 0xd9, 0xbc, 0x19, 0x8f, 0x1d, 0xd4, 0x3d, 0x37, 0xe1,
    0x2f, 0x15, 0x8d, 0x64, 0x06, 0x04, 0xfd, 0x29, 0xcf, 0xfa, 0x14, 0x2e,
    0x25, 0x1f, 0xc9, 0x18, 0xe3, 0x6d, 0x81, 0x84, 0x80, 0x3b, 0xb1, 0xe5,
    0x4d, 0xf7, 0x51, 0x78, 0xa9, 0x13, 0xad, 0xe9, 0x80, 0xc1, 0x0b, 0x25,
    0x93, 0xfc, 0x4d, 0x89, 0x23, 0xc2, 0x7c, 0x0b, 0x59, 0x15, 0xf6, 0x01,
    0x50, 0x55, 0xbf, 0x81, 0x75, 0xc3, 0x10, 0x31, 0xb5, 0xd3, 0x69, 0x84,
    0x89, 0xba, 0xd6, 0x89, 0xbd, 0x70, 0x19, 0x8e, 0x58, 0xa8, 0x3d, 0x9b,
    0x5d, 0xf0, 0x49, 0xe8, 0xad, 0x9d, 0x7a, 0x0d, 0x7e, 0x24, 0xda, 0xfc,
    0x0d, 0x14, 0xc5, 0x23, 0x91, 0x11, 0xf5, 0xc0, 0x4b, 0xcd, 0x44, 0x1c,
    0xc5, 0x21, 0xdf, 0x61, 0x54, 0xed, 0xa2, 0x81, 0xb7, 0xe5, 0x74, 0x94,
    0xb0, 0x47, 0xee, 0xf1, 0xa5, 0xbb, 0x21, 0xc8};

void decode(uint8_t* data) {
  uint8_t key_offset =
      (((((key[1] ^ key[2]) - key[3] - key[4]) ^ key[5]) + key[6] - key[7]) ^
       data[14]) +
      0x26;
  uint8_t key_index = 4;

  for (int8_t x = 12; x >= 0; x--) {
    uint8_t byte = data[x];
    for (int8_t y = 4; y > 1; y--) {
      key_offset--;

      uint8_t bkey = table[key_offset + 0x41];
      uint8_t keyr = key[key_index];
      if (--key_index == 0)
        key_index = 7;

      if ((bkey & 3) == 0)
        byte = (byte - bkey) - keyr;
      else if ((bkey & 3) == 1)
        byte = ((byte + bkey) + keyr);
      else
        byte = ((byte ^ bkey) ^ keyr);
    }
    data[x] = byte;
  }
}

bool hid_guncon3_check_device_desc(struct hub_info* hub_info,
                                   struct usb_info* usb_info,
                                   const struct usb_desc_device* desc) {
  if (desc->idVendor == 0x0c12 && desc->idProduct == 0x8801) {
    hub_info->type = HID_TYPE_ZAPPER;
    usb_info->state = HUB_CONNECTED;
    return true;
  }
  if (desc->idVendor == 0x0b9a && desc->idProduct == 0x0800) {
    hub_info->type = HID_TYPE_ZAPPER;
    usb_info->state = DEVICE_CONNECTED;
    return true;
  }
  usb_info->state = IDLE;
  return false;
}

bool hid_guncon3_check_interface_desc(struct usb_info* usb_info) {
  if (usb_info->state != IDLE)
    return true;
  return false;
}

bool hid_guncon3_initialize(struct hub_info* hub_info,
                            struct usb_info* usb_info) {
  if (usb_info->state == IDLE)
    return false;
  hub_info->report_size = 13;
  hub_info->report_id = 0;
  hub_info->axis[0] = 24;
  hub_info->axis[1] = 40;
  hub_info->axis_size[0] = 16;
  hub_info->axis_size[1] = 16;
  hub_info->axis_sign[0] = true;
  hub_info->axis_sign[1] = true;
  hub_info->axis_polarity[0] = false;
  hub_info->axis_polarity[1] = true;
  for (uint8_t i = 2; i < 4; ++i)
    hub_info->axis[i] = 0xffff;
  hub_info->hat = 0xffff;
  for (uint8_t i = 0; i < 4; ++i)
    hub_info->dpad[i] = 0xffff;
  hub_info->button[0] = 0x0d;
  for (uint8_t i = 1; i < 13; ++i)
    hub_info->button[i] = 0xffff;
  hub_info->button[9] = 0x03;
  hub_info->state = HID_STATE_READY;
  return false;
}

bool hid_guncon3_report(struct usb_info* usb_info,
                        uint8_t* data,
                        uint16_t size) {
  if (usb_info->state == IDLE)
    return false;

  switch (usb_info->state) {
    case HUB_GET_PORT_STATUS:
      usb_info->state = HUB_PORT_RESET;
      break;
    case HUB_PORT_RESET_WAIT:
      if (size == 4 && data[0] == 0x03 && data[1] == 0x01 && data[2] == 0x10 &&
          data[3] == 0x00) {
        // PORT_CONNECTION, PORT_ENABLE, PORT_POWER,
        // and C_PORT_RESET are set.
        usb_info->state = HUB_PORT_RESET_DONE;
      }
      break;
    case DEVICE_READY:
      if (size != 15)
        break;
      decode(data);
      return false;
    default:
      return false;
  }
  return true;
}

void hid_guncon3_poll(uint8_t hub, struct usb_info* usb_info) {
  static const struct usb_setup_req get_port_status = {
      USB_REQ_DIR_IN | USB_REQ_TYPE_CLASS | USB_REQ_RECPT_OTHER, USB_GET_STATUS,
      0x0000, 0x0001, 0x0004};

  if (usb_info->state == IDLE)
    return;

  switch (usb_info->state) {
    case HUB_CONNECTED: {
      usb_host_setup(hub, &get_port_status, 0);
      usb_info->state = HUB_GET_PORT_STATUS;
      break;
    }
    case HUB_PORT_RESET: {
      static const struct usb_setup_req set_port_reset_feature = {
          USB_REQ_DIR_OUT | USB_REQ_TYPE_CLASS | USB_REQ_RECPT_OTHER,
          USB_SET_FEATURE, USB_FEATURE_PORT_RESET, 0x0001, 0x0000};
      usb_host_setup(hub, &set_port_reset_feature, 0);
      usb_info->state = HUB_PORT_RESET_WAIT;
      break;
    }
    case HUB_PORT_RESET_WAIT: {
      usb_host_setup(hub, &get_port_status, 0);
      // usb_host_in(hub, hub_info->ep, usb_info->ep_max_packet_size);
      break;
    }
    case HUB_PORT_RESET_DONE: {
      static const struct usb_setup_req clear_port_reset_feature = {
          USB_REQ_DIR_OUT | USB_REQ_TYPE_CLASS | USB_REQ_RECPT_OTHER,
          USB_CLEAR_FEATURE, USB_FEATURE_C_PORT_RESET, 0x0001, 0x0000};
      usb_host_setup(hub, &clear_port_reset_feature, 0);
      usb_info->state = HUB_PORT_SET_ADDRESS;
      break;
    }
    case HUB_PORT_SET_ADDRESS:
      usb_host_hub_switch(hub, 3 + hub);
      usb_info->state = HUB_PORT_SET_ADDRESS_DONE;
      break;
    case DEVICE_CONNECTED:
      usb_host_out(hub, 2, key, 8);
      usb_info->state = DEVICE_READY;
      break;
    case DEVICE_READY:
      usb_host_in(hub, 2, 15);
      break;
  }
}
