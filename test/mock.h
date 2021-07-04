// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#ifndef __mock_h__
#define __mock_h__

extern "C" {
#include "chlib/usb_host.h"
}

extern struct usb_host* usb_host;

#endif  // __mock_h__