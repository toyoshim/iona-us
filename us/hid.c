// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "hid.h"

#include "chlib/ch559.h"
#include "chlib/led.h"
#include "chlib/usb.h"

static struct hid* hid;
static struct usb_host host;
static struct hub_info hub_info[2];

static void disconnected(uint8_t hub) {
  hub_info[hub].state = HID_STATE_DISCONNECTED;
}

static void check_configuration_desc(uint8_t hub, const uint8_t* data) {
  hub_info[hub].hid_report_desc_size = 0;
  hub_info[hub].hid_ep = 0;
  hub_info[hub].state = HID_STATE_CONNECTED;
  const struct usb_desc_configuration* desc =
      (const struct usb_desc_configuration*)data;
  struct usb_desc_head* head;
  for (uint8_t i = sizeof(*desc); i < desc->wTotalLength; i += head->bLength) {
    head = (struct usb_desc_head*)(data + i);
    switch (head->bDescriptorType) {
      case USB_DESC_HID: {
        const struct usb_desc_hid* hid = (const struct usb_desc_hid*)(data + i);
        hub_info[hub].hid_report_desc_size = hid->wDescriptorLength;
        break;
      }
      case USB_DESC_ENDPOINT: {
        const struct usb_desc_endpoint* ep =
            (const struct usb_desc_endpoint*)(data + i);
        if (ep->bEndpointAddress >= 128)
        hub_info[hub].hid_ep = ep->bEndpointAddress;
        break;
      }
    }
  }
  if (hub_info[hub].hid_report_desc_size && hub_info[hub].hid_ep)
    hub_info[hub].state = HID_STATE_NOT_READY;
}

#ifdef _DBG_HID
#  define REPORT0(s) Serial.println(s " (0)")
#  define REPORT1(s) Serial.printf(s " (1): %x\n", data[i + 1])
#  define REPORT2(s) Serial.printf(s " (2): %x%x\n", data[i + 1], data[i + 2])
#else
#  define REPORT0(s)
#  define REPORT1(s)
#  define REPORT2(s)
#  pragma disable_warning 110
#endif

static void check_hid_report_desc(uint8_t hub, const uint8_t* data) {
  if (hub_info[hub].state != HID_STATE_NOT_READY)
    return;
  const uint16_t size = hub_info[hub].hid_report_desc_size;
  hub_info[hub].hid_report_size = 0;
  hub_info[hub].axis[0] = 0xffff;
  hub_info[hub].axis[1] = 0xffff;
  hub_info[hub].dpad = 0xffff;
  for (uint8_t button = 0; button < 12; ++button)
    hub_info[hub].button[button] = 0xffff;
  hub_info[hub].report_id = 0;
  hub_info[hub].type = HID_TYPE_UNKNOWN;
  uint8_t report_size = 0;
  uint8_t report_count = 0;
  uint16_t usage_page = 0;
  uint8_t usage = 0;
  uint8_t button_index = 0;
  uint8_t analog_index = 0;
  for (uint16_t i = 0; i < size; ) {
    // Long items are not supported
    if (data[i] == 0xfe) {
      if ((i + 1) < size) {
        i += data[i + 1] + 3;
        continue;
      } else {
        break;
      }
    }
    // Short items
    uint8_t b_size = data[i] & 3;
    if (b_size == 0) { // 0 byte items
      switch (data[i]) {
        case 0xc0:
          REPORT0("M:End Collection");
          break;
        default:  // ignore
          break;
      }
      i++;
    } else if (b_size == 1) { // 1 bytes items
      if ((i + 1) >= size)
        break;
      switch (data[i]) {
        case 0x05:
          REPORT1("G:Usage Page");
          usage_page = data[i + 1];
          break;
        case 0x09:
          REPORT1("L:Usage");
          usage = data[i + 1];
          break;
        case 0x15:
          REPORT1("G:Logical Minimum");
          break;
        case 0x25:
          REPORT1("G:Logical Maximum");
          break;
        case 0x75:
          REPORT1("G:Report Size");
          report_size = data[i + 1];
          break;
        case 0x81:
          REPORT1("M:Input");
          if (usage_page == 0x01 && usage == 0x39 &&
              report_size == 4) { // Hat switch
          } else if (usage_page == 0xff00 && usage == 0x20 &&
                     report_size == 6) {  // PS4 counter
            hub_info[hub].type = HID_TYPE_PS4;
          } else if (report_size == 1) {  // Buttons
            for (uint8_t i = 0; i < report_count && button_index < 12; ++i) {
              hub_info[hub].button[button_index++] =
                  hub_info[hub].hid_report_size + i;
            }
          } else {  // Analog buttons
            for (uint8_t i = 0; i < report_count && analog_index < 2; ++i) {
              hub_info[hub].axis_size[analog_index] = report_size;
              hub_info[hub].axis[analog_index++] =
                hub_info[hub].hid_report_size + report_size * i;
            }
          }
          hub_info[hub].hid_report_size += report_size * report_count;
          break;
        case 0x85:
          if (hub_info[hub].hid_report_size)
            goto quit;
          REPORT1("G:Report ID");
          hub_info[hub].report_id = data[i + 1];
          break;
        case 0x95:
          REPORT1("G:Report Count");
          report_count = data[i + 1];
          break;
        case 0xa1:
          REPORT1("M:Collection");
          break;
        default: // not supported
          break;
      }
      i += 2;
    } else if (b_size == 2) {
      // 2 bytes items
      if ((i + 2) >= size)
        break;
      switch (data[i]) {
        case 0x06:
          REPORT2("G:Usage Page");
          usage_page = (data[i + 2] << 8) | data[i + 1];
          break;
        case 0x16:
          REPORT2("G:Logical Minimum");
          break;
        case 0x26:
          REPORT2("G:Logical Maximum");
          break;
        default: // not supported
          break;
      }
      i += 3;
    } else { // 4 bytes items
      i += 5;
    }
  }
 quit:
#if 0
  Serial.printf("Report Size for ID (%d): %d-bits (%d-Bytes)\n",
      0/*report_id*/,
      hub_info[hub].hid_report_size,
      hub_info[hub].hid_report_size / 8);
  for (uint8_t i = 0; i < 2; ++i)
    Serial.printf("axis %d: %d, %d\n", i, hub_info[hub].axis[i], hub_info[hub].axis_size[i]);
  for (uint8_t i = 0; i < 12; ++i)
    Serial.printf("button %d: %d\n", i, hub_info[hub].button[i]);
#endif
  hub_info[hub].state = HID_STATE_READY;
}

static void in(uint8_t hub, const uint8_t* data) {
  if (!hid->report)
    return;
  hid->report(hub, &hub_info[hub], data);
}

void hid_init(struct hid* new_hid) {
  hid = new_hid;
  host.flags = USE_HUB1;  // | USE_HUB0;
  host.disconnected = disconnected;
  host.check_device_desc = 0;
  host.check_configuration_desc = check_configuration_desc;
  host.check_hid_report_desc = check_hid_report_desc;
  host.in = in;
  usb_host_init(&host);
}

void hid_poll() {
  static uint8_t hub = 0;
  usb_host_poll();
  if (!usb_host_idle())
    return;
  if (hub_info[hub].state == HID_STATE_READY && usb_host_ready(hub)) {
    uint16_t size = hub_info[hub].hid_report_size / 8;
    if (hub_info[hub].report_id)
      size++;
    usb_host_in(hub, hub_info[hub].hid_ep, size);
  }
  hub++;
}