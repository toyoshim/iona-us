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

bool iona_usb_out(uint8_t ep, uint8_t* data, uint8_t size);
bool iona_usb_in(uint8_t ep, uint8_t size);

enum {
  TRANSACTION_STATE_IDLE,
  TRANSACTION_STATE_SETUP,
  TRANSACTION_STATE_IN,
  TRANSACTION_STATE_OUT,
};

static struct hid hid;
static struct usb_host* usb_host = 0;
static struct settings settings;
static uint8_t communication_buffer[1024];
static uint8_t transaction_state = TRANSACTION_STATE_IDLE;

void detected(void) {
  // printf("detected as type %d\n", hid_get_info(0)->type);
}

bool timer3_tick_raw_between(uint16_t begin, uint16_t end) {
  uint16_t now = timer3_tick_raw();
  if (begin < end) {
    return begin < now && now < end;
  }
  return end < now || now < begin;
}

void usb_host_init(struct usb_host* host) {
  usb_host = host;
}

void usb_host_poll(void) {}

bool usb_host_ready(uint8_t hub) {
  return transaction_state == TRANSACTION_STATE_IDLE;
}

bool usb_host_idle(void) {
  return transaction_state == TRANSACTION_STATE_IDLE;
}

bool usb_host_setup(uint8_t hub,
                    const struct usb_setup_req* req,
                    const uint8_t* data) {
  // TODO
  puts("usb_host_setup");
  return true;
}

bool usb_host_in(uint8_t hub, uint8_t ep, uint8_t size) {
  transaction_state = TRANSACTION_STATE_IN;
  return iona_usb_in(ep, size);
}

bool usb_host_in_data0(uint8_t hub, uint8_t ep, uint8_t size) {
  // TODO
  puts("usb_host_in_data0");
  return true;
}

bool usb_host_out(uint8_t hub, uint8_t ep, uint8_t* data, uint8_t size) {
  transaction_state = TRANSACTION_STATE_OUT;
  return iona_usb_out(ep, data, size);
}

bool usb_host_hid_get_report(uint8_t hub,
                             uint8_t type,
                             uint8_t id,
                             uint8_t size) {
  // TODO
  puts("usb_host_hid_get_report");
  return true;
}

void usb_host_hub_switch(uint8_t hub, uint8_t address) {
  // TODO
  puts("usb_host_hub_switch");
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

EMSCRIPTEN_KEEPALIVE uint8_t
iona_usb_host_check_configuration_desc(const uint8_t* desc) {
  if (!usb_host) {
    error();
    return 0;
  }
  return usb_host->check_configuration_desc(0, desc);
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

EMSCRIPTEN_KEEPALIVE void iona_poll(void) {
  hid_poll();
}

EMSCRIPTEN_KEEPALIVE void iona_transaction_complete(uint8_t size) {
  if (transaction_state == TRANSACTION_STATE_IN) {
    usb_host->in(0, communication_buffer, size);
  }
  transaction_state = TRANSACTION_STATE_IDLE;
}

EMSCRIPTEN_KEEPALIVE void* iona_get_communication_buffer(void) {
  return communication_buffer;
}

EMSCRIPTEN_KEEPALIVE bool iona_is_device_ready(void) {
  return hid_get_info(0)->state == HID_STATE_READY;
}

EMSCRIPTEN_KEEPALIVE uint8_t iona_get_device_type(void) {
  return hid_get_info(0)->type;
}

EMSCRIPTEN_KEEPALIVE uint16_t iona_get_digital_states(void) {
  return (controller_data(0, 0, 0) << 8) | controller_data(0, 1, 0);
}

EMSCRIPTEN_KEEPALIVE uint16_t iona_get_analog_state(uint8_t index) {
  return controller_analog(index);
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
  settings.sequence[0].on = true;
  settings.sequence[0].invert = false;
  settings.sequence[0].pattern = 0xff;
  settings.sequence[0].bit = 1;
  settings.sequence[0].mask = 0xff;

  hid.report = controller_update;
  hid.detected = detected;
  hid.get_flags = 0;
  hid_init(&hid);
}