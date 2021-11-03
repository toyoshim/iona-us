// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "mock.h"

struct usb_host* usb_host = nullptr;

extern "C" {

#include "chlib/led.h"

void led_oneshot(uint8_t shot) {}

void usb_host_init(struct usb_host* host) {
  usb_host = host;
}

void usb_host_poll() {}

bool usb_host_ready(uint8_t hub) {
  return false;
}

bool usb_host_idle() {
  return false;
}

bool usb_host_setup(uint8_t hub,
                    const struct usb_setup_req* req,
                    const uint8_t* data) {
  return false;
}

bool usb_host_in(uint8_t hub, uint8_t ep, uint8_t size) {
  return false;
}

bool usb_host_in_data0(uint8_t hub, uint8_t ep, uint8_t size) {
  return false;
}

bool usb_host_out(uint8_t hub, uint8_t ep, uint8_t* data, uint8_t size) {
  return false;
}

void usb_host_hub_switch(uint8_t hub, uint8_t address) {}
}