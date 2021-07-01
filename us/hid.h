// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#ifndef __hid_h__
#define __hid_h__

#include "chlib/usb_host.h"

void hid_init();
void hid_poll();

#endif  // __hid_h__
