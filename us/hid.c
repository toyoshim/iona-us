// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "hid.h"

#include "chlib/ch559.h"
#include "chlib/led.h"
#include "chlib/serial.h"
#include "chlib/usb.h"

//#define _DBG_HID_REPORT_DESC
//#define _DBG_HID_REPORT_DESC_DUMP
//#define _DBG_WITH_ONLY_HUB1

static struct hid* hid;
static struct usb_host host;
static struct hub_info hub_info[2];
static struct {
  uint8_t class;
  uint16_t ep_max_packet_size;
  uint8_t ep;
  uint8_t state;
  uint8_t cmd_count;
} usb_info[2];

// state for XBOX series.
enum {
  XBOX_CONNECTED,
  XBOX_INITIALIZED,
};

static uint8_t xbox_360_initialize[] = {0x01, 0x03, 0x00};
static uint8_t xbox_one_initialize[] = {0x05, 0x20, 0x00, 0x01, 0x00};

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

  usb_info[hub].class = desc->bDeviceClass;

  if (desc->idVendor == 0x045e) {  // Microsoft
    if (desc->idProduct == 0x028e) {
      hub_info[hub].type = HID_TYPE_XBOX_360;
      hub_info[hub].report_desc_size = 1;
    }
    if (desc->idProduct == 0x02d1 || desc->idProduct == 0x02dd ||
        desc->idProduct == 0x02e3 || desc->idProduct == 0x02ea ||
        desc->idProduct == 0x0b00 || desc->idProduct == 0x0b0a ||
        desc->idProduct == 0x0b12) {
      hub_info[hub].type = HID_TYPE_XBOX_ONE;
      hub_info[hub].report_desc_size = 1;
    }
  } else if (desc->bDeviceClass == 0xff && desc->bDeviceSubClass == 0x47 &&
             desc->bDeviceProtocol == 0xd0) {
    // Might be a Xbox One compatible controller.
    hub_info[hub].type = HID_TYPE_XBOX_ONE;
    hub_info[hub].report_desc_size = 1;
  }
}

static void check_configuration_desc(uint8_t hub, const uint8_t* data) {
  const struct usb_desc_configuration* desc =
      (const struct usb_desc_configuration*)data;
  struct usb_desc_head* head;
  uint8_t class = usb_info[hub].class;
  for (uint8_t i = sizeof(*desc); i < desc->wTotalLength; i += head->bLength) {
    head = (struct usb_desc_head*)(data + i);
    switch (head->bDescriptorType) {
      case USB_DESC_INTERFACE: {
        const struct usb_desc_interface* intf =
            (const struct usb_desc_interface*)(data + i);
        if (usb_info[hub].class == 0)
          class = intf->bInterfaceClass;
        if (intf->bInterfaceClass == 0xff && intf->bInterfaceSubClass == 0x5d &&
            intf->bInterfaceProtocol == 0x01) {
          // Might be a Xbox 360 compatible controller.
          hub_info[hub].type = HID_TYPE_XBOX_360;
          hub_info[hub].report_desc_size = 1;
        }
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

  if (hub_info[hub].state == HID_STATE_NOT_READY &&
      (hub_info[hub].type == HID_TYPE_XBOX_ONE ||
       hub_info[hub].type == HID_TYPE_XBOX_360)) {
    hub_info[hub].state = HID_STATE_READY;
    usb_info[hub].state = XBOX_CONNECTED;
    usb_info[hub].cmd_count = 0;
    if (hub_info[hub].type == HID_TYPE_XBOX_360) {
      // https://github.com/xboxdrv/xboxdrv/blob/stable/PROTOCOL
      hub_info[hub].report_size = 20 * 8;
      hub_info[hub].axis[0] = 6 * 8;
      hub_info[hub].axis_size[0] = 16;
      hub_info[hub].axis_sign[0] = true;
      hub_info[hub].axis_polarity[0] = false;
      hub_info[hub].axis[1] = 8 * 8;
      hub_info[hub].axis_size[1] = 16;
      hub_info[hub].axis_sign[1] = true;
      hub_info[hub].axis_polarity[1] = true;
      hub_info[hub].hat = 0xffff;
      hub_info[hub].dpad[0] = 16 + 0;
      hub_info[hub].dpad[1] = 16 + 1;
      hub_info[hub].dpad[2] = 16 + 2;
      hub_info[hub].dpad[3] = 16 + 3;
      hub_info[hub].button[0] = 24 + 6;
      hub_info[hub].button[1] = 24 + 4;
      hub_info[hub].button[2] = 24 + 5;
      hub_info[hub].button[3] = 24 + 7;
      hub_info[hub].button[4] = 24 + 0;
      hub_info[hub].button[5] = 24 + 1;
      hub_info[hub].button[6] = 39;
      hub_info[hub].button[7] = 47;
      hub_info[hub].button[8] = 16 + 5;
      hub_info[hub].button[9] = 16 + 4;
      hub_info[hub].button[10] = 16 + 6;
      hub_info[hub].button[11] = 16 + 7;
      hub_info[hub].report_id = 0;
    } else {
      // https://github.com/quantus/xbox-one-controller-protocol
      hub_info[hub].report_size = 18 * 8;
      hub_info[hub].axis[0] = 10 * 8;
      hub_info[hub].axis_size[0] = 16;
      hub_info[hub].axis_sign[0] = true;
      hub_info[hub].axis_polarity[0] = false;
      hub_info[hub].axis[1] = 12 * 8;
      hub_info[hub].axis_size[1] = 16;
      hub_info[hub].axis_sign[1] = true;
      hub_info[hub].axis_polarity[1] = true;
      hub_info[hub].hat = 0xffff;
      hub_info[hub].dpad[0] = 40 + 0;
      hub_info[hub].dpad[1] = 40 + 1;
      hub_info[hub].dpad[2] = 40 + 2;
      hub_info[hub].dpad[3] = 40 + 3;
      hub_info[hub].button[0] = 32 + 6;
      hub_info[hub].button[1] = 32 + 4;
      hub_info[hub].button[2] = 32 + 5;
      hub_info[hub].button[3] = 32 + 7;
      hub_info[hub].button[4] = 40 + 4;
      hub_info[hub].button[5] = 40 + 5;
      hub_info[hub].button[6] = 57;
      hub_info[hub].button[7] = 73;
      hub_info[hub].button[8] = 32 + 3;
      hub_info[hub].button[9] = 32 + 2;
      hub_info[hub].button[10] = 40 + 6;
      hub_info[hub].button[11] = 40 + 7;
      hub_info[hub].report_id = 0;
    }
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
  for (uint8_t button = 0; button < 2; ++button)
    hub_info[hub].axis[button] = 0xffff;
  hub_info[hub].hat = 0xffff;
  for (uint8_t button = 0; button < 4; ++button)
    hub_info[hub].dpad[button] = 0xffff;
  for (uint8_t button = 0; button < 12; ++button)
    hub_info[hub].button[button] = 0xffff;
  hub_info[hub].report_id = 0;
  hub_info[hub].type = HID_TYPE_UNKNOWN;
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
            for (uint8_t i = 0; i < report_count && button_index < 12; ++i) {
              hub_info[hub].button[button_index++] =
                  hub_info[hub].report_size + i;
            }
          } else if ((data[i + 1] & 1) == 0) {  // Analog buttons
            for (uint8_t i = 0; i < report_count && analog_index < 2; ++i) {
              if (usages[i] == 0x00010030)
                analog_index = 0;
              else if (usages[i] == 0x00010031)
                analog_index = 1;
              hub_info[hub].axis_size[analog_index] = report_size;
              hub_info[hub].axis_sign[analog_index] =
                  false;  // TODO: support signed.
              hub_info[hub].axis_polarity[analog_index] = false;
              hub_info[hub].axis[analog_index++] =
                  hub_info[hub].report_size + report_size * i;
              if (analog_index < 2 &&
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
          hub_info[hub].type = HID_TYPE_UNKNOWN;
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
  for (uint8_t i = 0; i < 2; ++i)
    Serial.printf("axis %d: %d, %d\n", i, hub_info[hub].axis[i],
                  hub_info[hub].axis_size[i]);
  Serial.printf("hat: %d\n", hub_info[hub].hat);
  for (uint8_t i = 0; i < 12; ++i)
    Serial.printf("button %d: %d\n", i, hub_info[hub].button[i]);
#endif
  hub_info[hub].state = HID_STATE_READY;
  if (hub_info[hub].type != HID_TYPE_UNKNOWN)
    led_oneshot(L_PULSE_ONCE);
}

static void hid_report(uint8_t hub, const uint8_t* data, uint16_t size) {
  if (!hid->report)
    return;
  if (hub_info[hub].type == HID_TYPE_XBOX_360 && size != 20 && data[0] != 0x00)
    return;
  if (hub_info[hub].type == HID_TYPE_XBOX_ONE && size != 18 && data[0] != 0x20)
    return;
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
        if (usb_info[hub].state == XBOX_CONNECTED) {
          xbox_360_initialize[2] = 0x02 + hub;
          usb_host_out(hub, usb_info[hub].ep, xbox_360_initialize,
                       sizeof(xbox_360_initialize));
          usb_info[hub].state = XBOX_INITIALIZED;
        } else if (usb_info[hub].state == XBOX_INITIALIZED) {
          usb_host_in(hub, hub_info[hub].ep, 20);
        }
        break;
      case HID_TYPE_XBOX_ONE:
        if (usb_info[hub].state == XBOX_CONNECTED) {
          xbox_one_initialize[2] = usb_info[hub].cmd_count++;
          usb_host_out(hub, usb_info[hub].ep, xbox_one_initialize,
                       sizeof(xbox_one_initialize));
          usb_info[hub].state = XBOX_INITIALIZED;
        } else if (usb_info[hub].state == XBOX_INITIALIZED) {
          usb_host_in(hub, hub_info[hub].ep, usb_info[hub].ep_max_packet_size);
        }
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