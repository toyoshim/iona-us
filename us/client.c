// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "client.h"

#include "adc.h"
#include "ch559.h"
#include "io.h"
#include "jvsio_client.h"
#include "jvsio_node.h"
#include "pwm1.h"

#include "serial.h"
#include "settings.h"
#include "soft485.h"

// #define FORCE_V3

#if defined(FORCE_V3)
static bool v3 = true;
#else
static bool v3 = false;
#endif
static bool mode_in = true;
static uint16_t sense = 0xffff;
static enum JVSIO_CommSupMode comm_mode = k115200;
static struct settings* settings = 0;
static void (*data_write)(uint8_t data) = 0;

static void update_direction() {
  // Activate pull-down only if the serial I/O direction is input.
  bool activate = mode_in && settings->data_signal_adjustment;
  pinMode(2, 0, activate ? OUTPUT : INPUT);

  // External RS485 controller DE-/RE (for v3 board)
  if (v3) {
    digitalWrite(4, 1, mode_in ? LOW : HIGH);
  }
}

static void txd1_send(uint8_t data) {
  while (!(SER1_LSR & bLSR_T_FIFO_EMP))
    ;
  SER1_FIFO = data;
}

static void data_write_3M(uint8_t data) {
  data;
  // clang-format off
  __asm
    ; start bit
    mov _P4_OUT, r2  ; 2t

    ; prepare for sending loop
    ; r1 = 1, r2 = 2
    mov r1, #0x01  ; 4t
    mov r2, #0x02  ; 4t
    ; a = data
    mov a, dpl  ; 2t
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

int JVSIO_Client_isDataAvailable() {
  update_direction();
  return soft485_ready() ? 1 : 0;
}

void JVSIO_Client_willSend() {
  mode_in = false;
  update_direction();
  if (!v3) {
    soft485_output();
  }
}

void JVSIO_Client_willReceive() {
  if (v3) {
    while (0 == (SER1_LSR & bLSR_T_ALL_EMP))
      ;
  } else {
    soft485_input();
  }
  mode_in = true;
  update_direction();
}

void JVSIO_Client_send(uint8_t data) {
  data_write(data);
}

uint8_t JVSIO_Client_receive() {
  return soft485_recv();
}

void JVSIO_Client_dump(const char* str, uint8_t* data, uint8_t len) {
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

bool JVSIO_Client_isSenseReady() {
  if (!v3) {
    return true;
  }

  client_poll();
  // The downstream SENSE seems > 3V, maybe pull-up 5V, in disconnected state.
  if (sense > 55000) {
    return true;
  }
  // The downstream SENSE seems < 1V, in readystate.
  if (sense < 20000) {
    return true;
  }
  // Otherwise, it's still under address configuration.
  return false;
}

bool JVSIO_Client_setCommSupMode(enum JVSIO_CommSupMode mode, bool dryrun) {
  if ((mode != k115200) && (!settings->jvs_dash_support || mode != k3M))
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
          data_write = soft485_send;
        }
        break;
      case k3M:
        led_mode = L_BLINK_TWICE;
        if (!v3) {
          data_write = data_write_3M;
        }
        break;
    }
    settings_led_mode(led_mode);
    comm_mode = mode;
  }
  return true;
}

void JVSIO_Client_setSense(bool ready) {
  P5_IN |= bP4_DRV;
  pwm1_enable(!ready);
}

void JVSIO_Client_setLed(bool ready) {
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
    mode = settings->data_signal_adjustment ? L_FASTER_BLINK : L_FAST_BLINK;
  }
  settings_led_mode(mode);
}

void JVSIO_Client_delayMicroseconds(unsigned int usec) {
  usec;
  // Omit delay as it isn't needed practically.
  // delayMicroseconds(usec);
}

void client_init() {
  settings = settings_get();
#if !defined(FORCE_V3)
  // Check V3 board that P4_2 is connected to GND.
  pinMode(4, 2, INPUT_PULLUP);
  delayMicroseconds(1000);
  if (digitalRead(4, 2) == LOW) {
    v3 = true;
  }
  pinMode(4, 2, INPUT);
#endif
  if (v3) {
    // Assign TXD1 to P4.4.
    SER1_IER |= bIER_PIN_MOD0;
    SER1_IER &= ~bIER_PIN_MOD1;

    soft485_init();
    data_write = txd1_send;
    pinMode(4, 4, OUTPUT);

    // External RS485 controller setup
    digitalWrite(4, 1, LOW);
    pinMode(4, 1, OUTPUT);

    // Pull-up for downstream JVS sense (for v3 board)
    adc_init();
    adc_select(7);
    pinMode(3, 0, INPUT_PULLUP);
  } else {
    data_write = soft485_send;
    soft485_init();
  }

  // Additional D+ pull-down that is activated only on receiving.
  digitalWrite(2, 0, LOW);

  // Drive sense signal.
  pwm1_init();
  pwm1_duty(3, 4);

  JVSIO_Client_setLed(false);

  JVSIO_Node_init(1);
}

void client_poll() {
  if (v3 && adc_peek(&sense)) {
    // Extends to 16-bit range.
    sense <<= 5;
  }
}