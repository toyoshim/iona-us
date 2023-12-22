// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#ifndef __client_h__
#define __client_h__

#include "jvsio_client.h"

void client_init(void);
void client_poll(void);
enum JVSIO_CommSupMode client_get_current_speed(void);

#endif  // __client_h__