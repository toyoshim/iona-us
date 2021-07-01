// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "hid.h"

#include "chlib/ch559.h"
#include "chlib/led.h"
#include "chlib/usb.h"

static struct usb_host host;

static void check_device_desc(uint8_t hub, const uint8_t* data) {
  hub;
  data;
  Serial.println("device desc");
  led_oneshot(1);
}

static void check_configuration_desc(uint8_t hub, const uint8_t* data) {
  hub;
  data;
  Serial.println("config desc");
}

static void check_hid_report_desc(uint8_t hub, const uint8_t* data) {
  hub;
  data;
  Serial.println("report desc");
}

static void in(uint8_t hub, const uint8_t* data) {
  hub;
  data;
  Serial.println("in");
}

void hid_init() {
  host.flags = USE_HUB1;  // | USE_HUB0;
  host.check_device_desc = check_device_desc;
  host.check_configuration_desc = check_configuration_desc;
  host.check_hid_report_desc = check_hid_report_desc;
  host.in = in;
  usb_host_init(&host);
}

void hid_poll() {
  usb_host_poll();
}