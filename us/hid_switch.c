// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "hid_switch.h"

#include "chlib/usb.h"
#include "hid.h"
#include "hid_internal.h"

enum {
  CONNECTED = 0,
  REQUEST_MAC,
  REQUEST_MAC_IN,
  REQUEST_MAC_RECV,
  HANDSHAKE,
  HANDSHAKE_IN,
  HANDSHAKE_RECV,
  BAUDRATE,
  BAUDRATE_IN,
  BAUDRATE_RECV,
  HANDSHAKE2,
  HANDSHAKE2_IN,
  HANDSHAKE2_RECV,
  NO_TIMEOUT,
  NO_TIMEOUT_IN,
  NO_TIMEOUT_RECV,
  NO_TIMEOUT2,
  NO_TIMEOUT2_IN,
  NO_TIMEOUT2_RECV,
  PLAYER_LED,
  PLAYER_LED_IN,
  PLAYER_LED_RECV,
  HOME_LED,
  HOME_LED_IN,
  HOME_LED_RECV,
  REPORT_MODE,
  REPORT_MODE_IN,
  REPORT_MODE_RECV,
  INITIALIZED,
};

static struct {
  uint8_t joycon;
  uint8_t data3;
  uint8_t data4;
} switch_info[2];

static uint8_t* create_sub_command(struct usb_info* usb_info,
                                   uint8_t sub_command,
                                   uint8_t* bytes,
                                   uint8_t size) {
  static uint8_t cmd[64];
  cmd[0] = 0x01;
  cmd[1] = usb_info->cmd_count++;
  cmd[2] = 0x00;
  cmd[3] = 0x01;
  cmd[4] = 0x40;
  cmd[5] = 0x40;
  cmd[6] = 0x00;
  cmd[7] = 0x01;
  cmd[8] = 0x40;
  cmd[9] = 0x40;
  cmd[10] = sub_command;
  uint8_t i;
  for (i = 0; i < size; ++i)
    cmd[11 + i] = bytes[i];
  for (i += 11; i < 64; ++i)
    cmd[i] = 0;
  return cmd;
}

bool hid_switch_check_device_desc(struct hub_info* hub_info,
                                  struct usb_info* usb_info,
                                  const struct usb_desc_device* desc) {
  if (desc->idVendor == 0x057e) {
    if (desc->idProduct == 0x2009 || desc->idProduct == 0x200e) {
      // Nintendo Switch Pro Controller, and Charging Grip.
      hub_info->type = HID_TYPE_SWITCH;
      usb_info->state = CONNECTED;
      return true;
    }
  }
  return false;
}

bool hid_switch_initialize(struct hub_info* hub_info) {
  if (hub_info->type != HID_TYPE_SWITCH)
    return false;

  // Their reporting HID Report Descriptors are completely fake.
  hub_info->report_size = 64 * 8;
  hub_info->axis[0] = 5 * 8;
  hub_info->axis_size[0] = 12;
  hub_info->axis_sign[0] = false;
  hub_info->axis_polarity[0] = false;
  hub_info->axis[1] = 6 * 8 + 4;
  hub_info->axis_size[1] = 12;
  hub_info->axis_sign[1] = false;
  hub_info->axis_polarity[1] = true;
  hub_info->hat = 0xffff;
  hub_info->dpad[0] = 4 * 8 + 1;
  hub_info->dpad[1] = 4 * 8 + 0;
  hub_info->dpad[2] = 4 * 8 + 3;
  hub_info->dpad[3] = 4 * 8 + 2;
  hub_info->button[0] = 2 * 8 + 0;
  hub_info->button[1] = 2 * 8 + 2;
  hub_info->button[2] = 2 * 8 + 3;
  hub_info->button[3] = 2 * 8 + 1;
  hub_info->button[4] = 4 * 8 + 6;
  hub_info->button[5] = 2 * 8 + 6;
  hub_info->button[6] = 4 * 8 + 7;
  hub_info->button[7] = 2 * 8 + 7;
  hub_info->button[8] = 3 * 8 + 0;
  hub_info->button[9] = 3 * 8 + 1;
  hub_info->button[10] = 3 * 8 + 3;
  hub_info->button[11] = 3 * 8 + 2;
  hub_info->report_id = 0x30;
  return true;
}
bool hid_switch_report(uint8_t hub,
                       struct hub_info* hub_info,
                       struct usb_info* usb_info,
                       uint8_t* data,
                       uint16_t size) {
  if (hub_info->type != HID_TYPE_SWITCH)
    return false;

  if (size == 0) {
    // retry against NAK response.
    if (usb_info->state != INITIALIZED)
      usb_info->state--;
    return true;
  }

  if (usb_info->state == INITIALIZED && data[0] == 0x30) {
    if (switch_info[hub].joycon != 0) {
      switch_info[hub].data3 = data[3];
      switch_info[hub].data4 = data[4];
      return true;
    }
    data[3] |= switch_info[hub].data3;
    data[4] |= switch_info[hub].data4;
    return false;
  }

  switch (usb_info->state) {
    case REQUEST_MAC_RECV:
      if (data[0] == 0x81 && data[1] == 0x01)
        break;
      goto retry;
    case HANDSHAKE_RECV:
    case HANDSHAKE2_RECV:
      if (data[0] == 0x81 && data[1] == 0x02)
        break;
      goto retry;
    case BAUDRATE_RECV:
      if (data[0] == 0x81 && data[1] == 0x03)
        break;
      goto retry;
    case NO_TIMEOUT_RECV:
      // No specific ACK. Any packet is ok to proceed.
      break;
    case NO_TIMEOUT2_RECV:
      if (data[0] == 0x21 && data[14] == 0x33)
        break;
      goto retry;
    case PLAYER_LED_RECV:
      if (data[0] == 0x21 && data[14] == 0x30)
        break;
      goto retry;
    case HOME_LED_RECV:
      if (data[0] == 0x21 && data[14] == 0x38)
        break;
      goto retry;
    case REPORT_MODE_RECV:
      if (data[0] == 0x21 && data[14] == 0x03)
        break;
      goto retry;
    default:
      return true;
  }
  usb_info->state++;
  if (usb_info->state == INITIALIZED) {
    if (usb_info->pid == 0x200e && switch_info[hub].joycon == 0) {
      switch_info[hub].joycon++;
      usb_info->state = REQUEST_MAC;
    }
  }
  return true;
retry:
  usb_info->state--;
  return true;
}

void hid_switch_poll(uint8_t hub, struct usb_info* usb_info) {
  uint8_t ep = switch_info[hub].joycon == 0 ? 1 : 2;
  switch (usb_info->state) {
    case CONNECTED:
      switch_info[hub].joycon = 0;
      switch_info[hub].data3 = 0;
      switch_info[hub].data4 = 0;
      break;
    case REQUEST_MAC: {
      static uint8_t request_mac[2] = {0x80, 0x01};
      usb_host_out(hub, ep, request_mac, sizeof(request_mac));
      break;
    }
    case HANDSHAKE:
    case HANDSHAKE2: {
      static uint8_t handshake[2] = {0x80, 0x02};
      usb_host_out(hub, ep, handshake, sizeof(handshake));
      break;
    }
    case BAUDRATE: {
      static uint8_t baudrate[2] = {0x80, 0x03};
      usb_host_out(hub, ep, baudrate, sizeof(baudrate));
      break;
    }
    case NO_TIMEOUT: {
      static uint8_t no_timeout[2] = {0x80, 0x04};
      usb_host_out(hub, ep, no_timeout, sizeof(no_timeout));
      break;
    }
    case NO_TIMEOUT2: {
      usb_host_out(hub, ep, create_sub_command(usb_info, 0x33, 0, 0), 64);
      break;
    }
    case PLAYER_LED: {
      uint8_t led[1] = {0x01 + hub};
      usb_host_out(hub, ep,
                   create_sub_command(usb_info, 0x30, led, sizeof(led)), 64);
      break;
    }
    case HOME_LED: {
      uint8_t led[4] = {0x01, 0xf0, 0xf0, 0x00};
      usb_host_out(hub, ep,
                   create_sub_command(usb_info, 0x38, led, sizeof(led)), 64);
    }
    case REPORT_MODE: {
      uint8_t mode[1] = {0x30};
      usb_host_out(hub, ep,
                   create_sub_command(usb_info, 0x03, mode, sizeof(mode)), 64);
    }
    case REQUEST_MAC_IN:
    case HANDSHAKE_IN:
    case HANDSHAKE2_IN:
    case BAUDRATE_IN:
    case NO_TIMEOUT_IN:
    case NO_TIMEOUT2_IN:
    case PLAYER_LED_IN:
    case HOME_LED_IN:
    case REPORT_MODE_IN:
      usb_host_in(hub, ep, 64);
      break;
    case INITIALIZED:
      usb_host_in(hub, ep, 64);
      if (usb_info->pid == 0x200e)
        switch_info[hub].joycon = (switch_info[hub].joycon + 1) & 1;
      return;
    default:
      return;
  }
  usb_info->state++;
}
