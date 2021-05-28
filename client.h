// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#ifndef __client_h__
#define __client_h__

struct JVSIO_DataClient;
struct JVSIO_SenseClient;
struct JVSIO_LedClient;

// JVS#1 - P4.3 (PWM1_) SENSE
// JVS#2 - P5.4 (XB) D-
// JVS#3 - P5.5 (XA) D+
// JVS#4 - GND
void data_client(struct JVSIO_DataClient* client);
void sense_client(struct JVSIO_SenseClient* client);
void led_client(struct JVSIO_LedClient* client);

#endif  // __client_h__