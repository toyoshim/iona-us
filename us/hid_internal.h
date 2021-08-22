// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#ifndef __hid_internal_h__
#define __hid_internal_h__

#include <stdint.h>

struct usb_info {
  uint8_t class;
  uint16_t pid;
  uint16_t ep_max_packet_size;
  uint8_t ep;  // interrupt out
  uint8_t state;
  uint8_t cmd_count;
};

#endif  // __hid_internal_h__