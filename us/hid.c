// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "hid.h"

#include "chlib/ch559.h"
#include "chlib/led.h"
#include "chlib/serial.h"
#include "chlib/usb.h"
#include "hid_internal.h"
#include "hid_keyboard.h"
#include "hid_switch.h"
#include "hid_xbox.h"

//#define _DBG_DESC
//#define _DBG_HID_REPORT_DESC
//#define _DBG_HID_REPORT_DESC_DUMP
//#define _DBG_WITH_ONLY_HUB1

static struct hid* hid;
static struct usb_host host;
static struct hub_info hub_info[2];
static struct usb_info usb_info[2];

static void disconnected(uint8_t hub) {
  hub_info[hub].state = HID_STATE_DISCONNECTED;
  hub_info[hub].report_size = 0;
  if (!hid->report)
    return;
  hid->report(hub, &hub_info[hub], 0, 0);
}

static void check_device_desc(uint8_t hub, const uint8_t* data) {
  hub_info[hub].report_desc_size = 0;
  hub_info[hub].ep = 0;
  hub_info[hub].state = HID_STATE_CONNECTED;
  hub_info[hub].type = HID_TYPE_UNKNOWN;
  const struct usb_desc_device* desc = (const struct usb_desc_device*)data;

#ifdef _DBG_DESC
  Serial.printf("device class: %x\n", desc->bDeviceClass);
  Serial.printf("device subclass: %x\n", desc->bDeviceSubClass);
  Serial.printf("device protocol: %x\n", desc->bDeviceProtocol);
#endif  // _DBG_DESC

  usb_info[hub].class = desc->bDeviceClass;
  usb_info[hub].pid = desc->idProduct;

  if (hid_keyboard_check_device_desc(&hub_info[hub], desc) ||
      hid_xbox_check_device_desc(&hub_info[hub], desc) ||
      hid_switch_check_device_desc(&hub_info[hub], &usb_info[hub], desc)) {
    return;
  }
}

static void check_configuration_desc(uint8_t hub, const uint8_t* data) {
  const struct usb_desc_configuration* desc =
      (const struct usb_desc_configuration*)data;
  struct usb_desc_head* head;
  uint8_t class = usb_info[hub].class;
  bool target_interface = false;
  for (uint8_t i = sizeof(*desc); i < desc->wTotalLength; i += head->bLength) {
    head = (struct usb_desc_head*)(data + i);
    switch (head->bDescriptorType) {
      case USB_DESC_INTERFACE: {
        const struct usb_desc_interface* intf =
            (const struct usb_desc_interface*)(data + i);
#ifdef _DBG_DESC
        Serial.printf("interface class: %x\n", intf->bInterfaceClass);
        Serial.printf("interface subclass: %x\n", intf->bInterfaceSubClass);
        Serial.printf("interface protocol: %x\n", intf->bInterfaceProtocol);
#endif  // _DBG_DESC
        if (usb_info[hub].class == 0)
          class = intf->bInterfaceClass;
        target_interface =
            hid_keyboard_check_interface_desc(&hub_info[hub], intf) ||
            hid_xbox_360_check_interface_desc(&hub_info[hub], intf) ||
            hid_xbox_one_check_interface_desc(&hub_info[hub], intf);
        break;
      }
      case USB_DESC_HID: {
        const struct usb_desc_hid* hid = (const struct usb_desc_hid*)(data + i);
        hub_info[hub].report_desc_size = hid->wDescriptorLength;
        break;
      }
      case USB_DESC_ENDPOINT: {
        if (hub_info[hub].type == HID_TYPE_UNKNOWN && class != USB_CLASS_HID)
          break;
        if ((hub_info[hub].type == HID_TYPE_KEYBOARD ||
             hub_info[hub].type == HID_TYPE_XBOX_360 ||
             hub_info[hub].type == HID_TYPE_XBOX_ONE) &&
            !target_interface) {
          break;
        }
        const struct usb_desc_endpoint* ep =
            (const struct usb_desc_endpoint*)(data + i);
        if (ep->bEndpointAddress >= 128 && (ep->bmAttributes & 3) == 3) {
          // interrupt input.
          hub_info[hub].ep = ep->bEndpointAddress & 0x0f;
          usb_info[hub].ep_max_packet_size = ep->wMaxPacketSize;
        } else if (ep->bEndpointAddress < 128 && (ep->bmAttributes & 3) == 3) {
          // interrupt output.
          usb_info[hub].ep = ep->bEndpointAddress & 0x0f;
        }
        break;
      }
    }
  }
  if (hub_info[hub].report_desc_size && hub_info[hub].ep)
    hub_info[hub].state = HID_STATE_NOT_READY;

  if (hid_keyboard_initialize(&hub_info[hub]) ||
      hid_xbox_initialize(&hub_info[hub], &usb_info[hub])) {
    led_oneshot(L_PULSE_ONCE);
  }
}

#ifdef _DBG_HID_REPORT_DESC
#define REPORT0(s) Serial.println(s " (0)")
#define REPORT1(s) Serial.printf(s " (1): %x\n", data[i + 1])
#define REPORT2(s) Serial.printf(s " (2): %x%x\n", data[i + 2], data[i + 1])
#else
#define REPORT0(s)
#define REPORT1(s)
#define REPORT2(s)
#pragma disable_warning 110
#endif

static void check_hid_report_desc(uint8_t hub, const uint8_t* data) {
  if (hub_info[hub].state != HID_STATE_NOT_READY)
    return;
#ifdef _DBG_HID_REPORT_DESC_DUMP
  {
    for (uint16_t i = 0; i < hub_info[hub].report_desc_size; ++i)
      Serial.printf("0x%x, ", data[i]);
  }
#endif
  const uint16_t size = hub_info[hub].report_desc_size;
  hub_info[hub].report_size = 0;
  for (uint8_t button = 0; button < 4; ++button)
    hub_info[hub].axis[button] = 0xffff;
  hub_info[hub].hat = 0xffff;
  for (uint8_t button = 0; button < 4; ++button)
    hub_info[hub].dpad[button] = 0xffff;
  for (uint8_t button = 0; button < 13; ++button)
    hub_info[hub].button[button] = 0xffff;
  hub_info[hub].report_id = 0;
  uint8_t report_size = 0;
  uint8_t report_count = 0;
  uint16_t usage_page = 0;
  uint8_t usage_index = 0;
  uint32_t usages[12];
  uint8_t button_index = 0;
  for (usage_index = 0; usage_index < 12; ++usage_index)
    usages[usage_index] = 0;
  usage_index = 0;
  uint8_t analog_index = 0;
  for (uint16_t i = 0; i < size;) {
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
    if (b_size == 0) {  // 0 byte items
      switch (data[i]) {
        case 0xc0:
          REPORT0("M:End Collection");
          for (usage_index = 0; usage_index < 12; ++usage_index)
            usages[usage_index] = 0;
          usage_index = 0;
          break;
        default:  // ignore
          break;
      }
      i++;
    } else if (b_size == 1) {  // 1 bytes items
      if ((i + 1) >= size)
        break;
      switch (data[i]) {
        case 0x05:
          REPORT1("G:Usage Page");
          usage_page = data[i + 1];
          break;
        case 0x09:
          REPORT1("L:Usage");
          if (usage_index < 12)
            usages[usage_index++] = ((uint32_t)usage_page << 16) | data[i + 1];
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
          if (usages[0] == 0x00010039 && report_size == 4 &&
              (data[i + 1] & 1) == 0) {  // Hat switch
            hub_info[hub].hat = hub_info[hub].report_size;
          } else if (usages[0] == 0xff000020 &&
                     report_size == 6) {  // PS4 counter
            hub_info[hub].type = HID_TYPE_PS4;
          } else if (report_size == 1) {  // Buttons
            for (uint8_t i = 0; i < report_count && button_index < 13; ++i) {
              hub_info[hub].button[button_index++] =
                  hub_info[hub].report_size + i;
            }
          } else if ((data[i + 1] & 1) == 0) {  // Analog buttons
            for (uint8_t i = 0; i < report_count && analog_index < 4; ++i) {
              if (usages[i] == 0x00010030)
                analog_index = 0;
              else if (usages[i] == 0x00010031)
                analog_index = 1;
              else if (usages[i] == 0x00010032)
                analog_index = 2;
              else if (usages[i] == 0x00010035)
                analog_index = 3;
              hub_info[hub].axis_size[analog_index] = report_size;
              hub_info[hub].axis_sign[analog_index] = false;
              hub_info[hub].axis_polarity[analog_index] = false;
              hub_info[hub].axis[analog_index++] =
                  hub_info[hub].report_size + report_size * i;
              while (analog_index < 4 &&
                     hub_info[hub].axis[analog_index] != 0xffff) {
                analog_index++;
              }
            }
          }
          usage_index = 0;
          hub_info[hub].report_size += report_size * report_count;
          break;
        case 0x85:
          if (hub_info[hub].report_size &&
              (hub_info[hub].hat != 0xffff || hub_info[hub].dpad[3] != 0xffff ||
               hub_info[hub].axis[1] != 0xffff) &&
              hub_info[hub].button[1] != 0xffff) {
            goto quit;
          }
          hub_info[hub].report_size = 0;
          for (uint8_t button = 0; button < 2; ++button)
            hub_info[hub].axis[button] = 0xffff;
          hub_info[hub].hat = 0xffff;
          for (uint8_t button = 0; button < 4; ++button)
            hub_info[hub].dpad[button] = 0xffff;
          for (uint8_t button = 0; button < 12; ++button)
            hub_info[hub].button[button] = 0xffff;
          for (uint8_t button = 0; button < 12; ++button)
            usages[button] = 0;
          usage_index = 0;
          button_index = 0;
          analog_index = 0;
          REPORT1("G:Report ID");
          hub_info[hub].report_id = data[i + 1];
          break;
        case 0x95:
          REPORT1("G:Report Count");
          report_count = data[i + 1];
          break;
        case 0xa1:
          REPORT1("M:Collection");
          for (usage_index = 0; usage_index < 12; ++usage_index)
            usages[usage_index] = 0;
          usage_index = 0;
          break;
        default:  // not supported
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
        case 0x0a:
          REPORT2("L:Usage");
          if (usage_index < 12) {
            usages[usage_index++] =
                ((uint32_t)usage_page << 16) | (data[i + 2] << 8) | data[i + 1];
          }
          break;
        case 0x16:
          REPORT2("G:Logical Minimum");
          break;
        case 0x26:
          REPORT2("G:Logical Maximum");
          break;
        default:  // not supported
          break;
      }
      i += 3;
    } else {  // 4 bytes items
      i += 5;
    }
  }
quit:
#ifdef _DBG_HID_REPORT_DESC
  Serial.printf("Report Size for ID (%d): %d-bits (%d-Bytes)\n",
                hub_info[hub].report_id, hub_info[hub].report_size,
                hub_info[hub].report_size / 8);
  for (uint8_t i = 0; i < 4; ++i)
    Serial.printf("axis %d: %d, %d\n", i, hub_info[hub].axis[i],
                  hub_info[hub].axis_size[i]);
  Serial.printf("hat: %d\n", hub_info[hub].hat);
  for (uint8_t i = 0; i < 13; ++i)
    Serial.printf("button %d: %d\n", i, hub_info[hub].button[i]);
#endif
  hub_info[hub].state = HID_STATE_READY;
  if (hub_info[hub].type == HID_TYPE_SWITCH)
    hid_switch_initialize(&hub_info[hub]);
  if (hub_info[hub].type != HID_TYPE_UNKNOWN)
    led_oneshot(L_PULSE_ONCE);
}

static void hid_report(uint8_t hub, uint8_t* data, uint16_t size) {
  if (hid_xbox_report(&hub_info[hub], data, size))
    return;
  if (hid_switch_report(hub, &hub_info[hub], &usb_info[hub], data, size))
    return;
  if (hid->report && size)
    hid->report(hub, &hub_info[hub], data, size);
}

void hid_init(struct hid* new_hid) {
  hid = new_hid;
#ifdef _DBG_WITH_ONLY_HUB1
  host.flags = USE_HUB1;
#else
  host.flags = USE_HUB1 | USE_HUB0;
#endif
  host.disconnected = disconnected;
  host.check_device_desc = check_device_desc;
  host.check_string_desc = 0;
  host.check_configuration_desc = check_configuration_desc;
  host.check_hid_report_desc = check_hid_report_desc;
  host.in = hid_report;
  usb_host_init(&host);
}

struct hub_info* hid_get_info(uint8_t hub) {
  return &hub_info[hub];
}

void hid_poll() {
  static uint8_t hub = 0;
  usb_host_poll();
  if (!usb_host_idle())
    return;
  if (hub_info[hub].state == HID_STATE_READY && usb_host_ready(hub)) {
    switch (hub_info[hub].type) {
      case HID_TYPE_XBOX_360:
        hid_xbox_360_poll(hub, &hub_info[hub], &usb_info[hub]);
        break;
      case HID_TYPE_XBOX_ONE:
        hid_xbox_one_poll(hub, &hub_info[hub], &usb_info[hub]);
        break;
      case HID_TYPE_SWITCH:
        hid_switch_poll(hub, &usb_info[hub]);
        break;
      default: {
        uint16_t size = hub_info[hub].report_size / 8;
        if (hub_info[hub].report_id)
          size++;
        usb_host_in(hub, hub_info[hub].ep, size);
        // usb_host_hid_get_report(hub, hub_info[hub].report_id, size);
        break;
      }
    }
  }
  hub = (hub + 1) & 1;
}