// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "client.h"

#include "ch559.h"
#include "io.h"
#include "pwm1.h"

#include "jvsio/JVSIO_c.h"
#include "settings.h"
#include "soft485.h"

static bool mode_in = true;

static void update_pulldown() {
  // Activate pull-down only if the serial I/O direction is input.
  bool activate = mode_in && settings_options_pulldown();
  pinMode(2, 0, activate ? OUTPUT : INPUT);
}

static int data_available(struct JVSIO_DataClient* client) {
  client;
  update_pulldown();
  return soft485_ready() ? 1 : 0;
}

static void data_setInput(struct JVSIO_DataClient* client) {
  client;
  soft485_input();
  mode_in = true;
  update_pulldown();
}

static void data_setOutput(struct JVSIO_DataClient* client) {
  client;
  mode_in = false;
  update_pulldown();
  soft485_output();
}

static void data_startTransaction(struct JVSIO_DataClient* client) {
  client;
}

static void data_endTransaction(struct JVSIO_DataClient* client) {
  client;
}

static uint8_t data_read(struct JVSIO_DataClient* client) {
  client;
  return soft485_recv();
}

static void data_write(struct JVSIO_DataClient* client, uint8_t data) {
  client;
  soft485_send(data);
}

static void data_write_1M(struct JVSIO_DataClient* client, uint8_t data) {
  client;
  data;
}

static void data_write_3M(struct JVSIO_DataClient* client, uint8_t data) {
  client;
  data;
  // clang-format off
  __asm
    ; prepare for sending loop (1)
    mov r1, #0x01  ; const r1 = 1
    mov r2, #0x02  ; const r2 = 2

    ; start bit
    mov _P4_OUT, r2  ; 2t

    ; prepare for sending loop (2)
    ; a = data
    mov a, _bp  ; 2t
    add a, #0xfd  ; 4t
    mov r0, a  ; 2t
    mov a, @r0  ; 2t
    ; r7 = 8
    mov r7, #0x08  ; 4t

  _3m_loop:
    rrc a  ; 1t
    jc _3m_send_1  ; 4t/2t
  _3m_send_0:
    nop
    nop
    mov _P4_OUT, r2  ; 2t
    nop
    nop
    nop
    nop
    djnz r7, _3m_loop  ; 6t/4t
    sjmp _3m_stop  ; 4t
  _3m_send_1:
    mov _P4_OUT, r1  ; 2t
    nop
    nop
    nop
    nop
    djnz r7, _3m_loop  ; 6t/4t
    sjmp _3m_stop  ; 4t

    ; stop bit
  _3m_stop:
    nop
    nop
    nop
    mov _P4_OUT, r1  ; 2t

  __endasm;
  // clang-format on
}

static void data_delayMicroseconds(struct JVSIO_DataClient* client,
                                   unsigned int usec) {
  client;
  delayMicroseconds(usec);
}

static void data_delay(struct JVSIO_DataClient* client, unsigned int msec) {
  client;
  delay(msec);
}

static bool data_setCommSupMode(struct JVSIO_DataClient* client,
                                enum JVSIO_CommSupMode mode,
                                bool dryrun) {
  if (mode != k115200 && mode != k3M)
    return false;
  if (!dryrun) {
    soft485_set_recv_speed(mode);
    switch (mode) {
      case k115200:
        client->write = data_write;
        break;
      case k1M:
        client->write = data_write_1M;
        break;
      case k3M:
        led_oneshot(L_PULSE_ONCE);
        client->write = data_write_3M;
        break;
    }
  }
  return true;
}

void data_client(struct JVSIO_DataClient* client) {
  client->available = data_available;
  client->setInput = data_setInput;
  client->setOutput = data_setOutput;
  client->startTransaction = data_startTransaction;
  client->endTransaction = data_endTransaction;
  client->read = data_read;
  client->write = data_write;
  client->delayMicroseconds = data_delayMicroseconds;
  client->delay = data_delay;
  client->setCommSupMode = data_setCommSupMode;

  soft485_init();

  // Additional D+ pull-down that is activated only on receiving.
  digitalWrite(2, 0, LOW);
}

static void sense_begin(struct JVSIO_SenseClient* client) {
  client;
  pwm1_init();
  pwm1_duty(3, 4);
}

static void sense_set(struct JVSIO_SenseClient* client, bool ready) {
  client;
  P5_IN |= bP4_DRV;
  pwm1_enable(!ready);
}

static bool sense_is_ready(struct JVSIO_SenseClient* client) {
  client;
  // can be true for single node.
  return true;
}

static bool sense_is_connected(struct JVSIO_SenseClient* client) {
  client;
  // can be true for client mode.
  return true;
}

void sense_client(struct JVSIO_SenseClient* client) {
  client->begin = sense_begin;
  client->set = sense_set;
  client->is_ready = sense_is_ready;
  client->is_connected = sense_is_connected;
}

static void led_begin(struct JVSIO_LedClient* client) {
  client;
}

static void led_set(struct JVSIO_LedClient* client, bool ready) {
  client;
  settings_led_mode(ready ? L_ON : L_FASTER_BLINK);
}

void led_client(struct JVSIO_LedClient* client) {
  client->begin = led_begin;
  client->set = led_set;
}