// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "client.h"

#include "adc.h"
#include "ch559.h"
#include "io.h"
#include "pwm1.h"

#include "jvsio/JVSIO_c.h"
#include "serial.h"
#include "settings.h"
#include "soft485.h"

static bool v3 = false;
static bool mode_in = true;
static uint16_t sense = 0xffff;
static enum JVSIO_CommSupMode comm_mode;

static void update_direction(void) {
  // Activate pull-down only if the serial I/O direction is input.
  bool activate = mode_in && settings_options_pulldown();
  pinMode(2, 0, activate ? OUTPUT : INPUT);

  // External RS485 controller DE-/RE (for v3 board)
  if (v3) {
    digitalWrite(4, 1, mode_in ? LOW : HIGH);
  }
}

static int data_available(struct JVSIO_DataClient* client) {
  client;
  update_direction();
  return soft485_ready() ? 1 : 0;
}

static void data_setInput(struct JVSIO_DataClient* client) {
  client;
  if (v3) {
    while (0 == (SER1_LSR & bLSR_T_ALL_EMP))
      ;
  } else {
    soft485_input();
  }
  mode_in = true;
  update_direction();
}

static void data_setOutput(struct JVSIO_DataClient* client) {
  client;
  mode_in = false;
  update_direction();
  if (!v3) {
    soft485_output();
  }
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

static void data_write_txd1(struct JVSIO_DataClient* client, uint8_t data) {
  client;
  while (!(SER1_LSR & bLSR_T_FIFO_EMP))
    ;
  SER1_FIFO = data;
}

static void data_write(struct JVSIO_DataClient* client, uint8_t data) {
  client;
  soft485_send(data);
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

static bool data_setCommSupMode(struct JVSIO_DataClient* client,
                                enum JVSIO_CommSupMode mode,
                                bool dryrun) {
  if ((mode != k115200) && (!settings_options_dash() || mode != k3M))
    return false;
  if (!dryrun) {
    soft485_set_recv_speed(mode);
    if (v3) {
      // External RS485 controller setup restore
      update_direction();
      pinMode(4, 1, OUTPUT);
    }
    uint8_t led_mode = L_ON;
    switch (mode) {
      case k115200:
        if (!v3) {
          client->write = data_write;
        }
        break;
      case k3M:
        if (!v3) {
          client->write = data_write_3M;
        }
        led_mode = L_BLINK_TWICE;
        break;
    }
    settings_led_mode(led_mode);
    comm_mode = mode;
  }
  return true;
}

static void data_dump(struct JVSIO_DataClient* client,
                      const char* str,
                      uint8_t* data,
                      uint8_t len) {
  client;
  Serial.print(str);
  Serial.print(": ");
  for (uint8_t i = 0; i < len; ++i) {
    if (data[i] < 16)
      Serial.print("0");
    Serial.printc(data[i], HEX);
    Serial.print(" ");
  }
  Serial.println("");
}

void client_init(void) {
  // Check V3 board that P4_2 is connected to GND.
  pinMode(4, 2, INPUT_PULLUP);
  delayMicroseconds(1000);
  if (digitalRead(4, 2) == LOW) {
    v3 = true;
    pinMode(4, 2, INPUT);
  }
}

void client_poll(void) {
  if (v3 && adc_peek(&sense)) {
    // Extends to 16-bit range.
    sense <<= 5;
  }
}

void data_client(struct JVSIO_DataClient* client) {
  if (v3) {
    // Assign TXD1 to P4.4.
    SER1_IER |= bIER_PIN_MOD0;
    SER1_IER &= ~bIER_PIN_MOD1;

    soft485_init();
    pinMode(4, 4, OUTPUT);

    // External RS485 controller setup
    digitalWrite(4, 1, LOW);
    pinMode(4, 1, OUTPUT);
  } else {
    soft485_init();
  }

  client->available = data_available;
  client->setInput = data_setInput;
  client->setOutput = data_setOutput;
  client->startTransaction = data_startTransaction;
  client->endTransaction = data_endTransaction;
  client->read = data_read;
  client->write = v3 ? data_write_txd1 : data_write;
  client->setCommSupMode = data_setCommSupMode;
  client->dump = data_dump;

  // Additional D+ pull-down that is activated only on receiving.
  digitalWrite(2, 0, LOW);
}

static void sense_begin(struct JVSIO_SenseClient* client) {
  client;
  pwm1_init();
  pwm1_duty(3, 4);

  if (v3) {
    // Pull-up for downstream JVS sense
    adc_init();
    adc_select(7);
    pinMode(3, 0, INPUT_PULLUP);
  }
}

static void sense_set(struct JVSIO_SenseClient* client, bool ready) {
  client;
  P5_IN |= bP4_DRV;
  pwm1_enable(!ready);
}

static bool sense_isReady(struct JVSIO_SenseClient* client) {
  client;
  // can be true for single node.
  // TODO: support v3 board daisy chains.
  return true;
}

static bool sense_isConnected(struct JVSIO_SenseClient* client) {
  client;
  // can be true for client mode.
  return true;
}

void sense_client(struct JVSIO_SenseClient* client) {
  client->begin = sense_begin;
  client->set = sense_set;
  client->isReady = sense_isReady;
  client->isConnected = sense_isConnected;
}

static void led_set(struct JVSIO_LedClient* client, bool ready) {
  client;
  uint8_t mode = L_ON;
  if (ready) {
    switch (comm_mode) {
      case k115200:
        mode = L_ON;
        break;
      case k3M:
        mode = L_BLINK_THREE_TIMES;
        break;
    }
  } else {
    mode = settings_options_pulldown() ? L_FASTER_BLINK : L_FAST_BLINK;
  }
  settings_led_mode(mode);
}

static void led_begin(struct JVSIO_LedClient* client) {
  led_set(client, false);
}

void led_client(struct JVSIO_LedClient* client) {
  client->begin = led_begin;
  client->set = led_set;
}

static void time_delayMicroseconds(struct JVSIO_TimeClient* client,
                                   unsigned int usec) {
  client;
  delayMicroseconds(usec);
}

static void time_delay(struct JVSIO_TimeClient* client, unsigned int msec) {
  client;
  delay(msec);
}

static uint32_t time_getTick(struct JVSIO_TimeClient* client) {
  client;
  return 0;  // not impl and not used in I/O device mode.
}

void time_client(struct JVSIO_TimeClient* client) {
  client->delayMicroseconds = time_delayMicroseconds;
  client->delay = time_delay;
  client->getTick = time_getTick;
}