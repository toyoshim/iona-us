// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#ifndef __client_h__
#define __client_h__

struct JVSIO_DataClient;
struct JVSIO_SenseClient;
struct JVSIO_LedClient;
struct JVSIO_TimeClient;

// JVS#1 - P4.3 (PWM1_) SENSE
// JVS#2 - P4.1 D- (proto: P1.1)
// JVS#3 - P4.0 (RXD1_) D+
// JVS#4 - GND
void client_init(void);
void client_poll(void);
void data_client(struct JVSIO_DataClient* client);
void sense_client(struct JVSIO_SenseClient* client);
void led_client(struct JVSIO_LedClient* client);
void time_client(struct JVSIO_TimeClient* client);

#endif  // __client_h__