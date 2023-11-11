// Copyright 2023 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include <emscripten/emscripten.h>
#include <stdint.h>
#include <stdio.h>

#include "hid.h"
#include "serial.h"
#include "usb_host.h"

#include "controller.h"
#include "settings.h"

static struct hid hid;
static struct usb_host* usb_host = 0;
static struct settings settings;
static uint8_t communication_buffer[1024];

void usb_host_init(struct usb_host* host) {
  usb_host = host;
}

struct settings* settings_get(void) {
  return &settings;
}

static void error(void) {
  puts("iona_init may not be called");
}

EMSCRIPTEN_KEEPALIVE void iona_usb_host_check_device_desc(const uint8_t* desc) {
  if (!usb_host) {
    return error();
  }
  usb_host->check_device_desc(0, desc);
}

EMSCRIPTEN_KEEPALIVE void iona_usb_host_check_configuration_desc(
    const uint8_t* desc) {
  if (!usb_host) {
    return error();
  }
  usb_host->check_configuration_desc(0, desc);
}

EMSCRIPTEN_KEEPALIVE void iona_usb_host_check_hid_report_desc(
    const uint8_t* desc) {
  if (!usb_host) {
    return error();
  }
  usb_host->check_hid_report_desc(0, desc);
}

EMSCRIPTEN_KEEPALIVE void iona_usb_host_check_hid_report(uint8_t* data,
                                                         uint16_t size) {
  if (!usb_host) {
    return error();
  }
  usb_host->hid_report(0, data, size);
}

EMSCRIPTEN_KEEPALIVE void* iona_get_communication_buffer(void) {
  return communication_buffer;
}

EMSCRIPTEN_KEEPALIVE void iona_init(void) {
  serial_init();

  for (int i = 0; i < 6; ++i) {
    settings.analog_type[0][i] = 2;
    settings.analog_index[0][i] = i;
  }
  for (int i = 0; i < 16; ++i) {
    settings.digital_map[0][i].data[0] = 0x20 >> i;
    settings.digital_map[0][i].data[1] = 0x2000 >> i;
  }
  for (int i = 0; i < 12; ++i) {
    settings.rapid_fire[0][i] = 0;
  }

  hid.report = controller_update;
  hid.detected = 0;
  hid.get_flags = 0;
  hid_init(&hid);
}