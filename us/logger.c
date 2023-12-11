// Copyright 2023 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "ch559.h"
#include "flash.h"
#include "gpio.h"
#include "led.h"
#include "serial.h"
#include "usb/hid/hid.h"

#define LOG_SIZE 1024
uint8_t log[LOG_SIZE];
uint16_t log_wptr;

static void (*runBootloader)(void) = 0xf400;

static void log_init(void) {
  log[0] = 'I';
  log[1] = 'O';
  log[2] = 'N';
  log[3] = 'L';
  log_wptr = 4;
}

static void log_putc(uint8_t data) {
  if (log_wptr == LOG_SIZE) {
    return;
  }
  log[log_wptr++] = data;
  if (log_wptr == LOG_SIZE) {
    led_mode(L_ON);
  }
}

void usb_host_log_send(uint8_t ep, uint8_t pid, uint8_t size, uint8_t* buffer) {
  log_putc(1);
  log_putc(ep);
  log_putc(pid);
  log_putc(size);

  for (uint8_t i = 0; i < size; ++i) {
    log_putc(buffer[i]);
  }
}

void usb_host_log_recv(uint8_t ep, uint8_t pid, uint8_t size, uint8_t* buffer) {
  log_putc(2);
  log_putc(ep);
  log_putc(pid);
  log_putc(size);
  for (uint8_t i = 0; i < size; ++i) {
    log_putc(buffer[i]);
  }
}

void usb_host_log_stall(void) {
  log_putc(3);
}

void usb_host_log_nak(void) {
  log_putc(4);
}

static void update(uint8_t hub,
                   const struct hid_info* info,
                   const uint8_t* data,
                   uint16_t size) {
  hub;
  info;
  data;
  size;
}

static void detected(void) {
  led_oneshot(L_PULSE_ONCE);
}

static uint8_t get_flags(void) {
  return USE_HUB1;
}

void main(void) {
  initialize();

  led_init(1, 5, LOW);
  flash_init(*(uint32_t*)"IONL", true);

  log_init();

  pinMode(4, 6, INPUT_PULLUP);

  delay(30);

  struct hid hid;
  hid.report = update;
  hid.detected = detected;
  hid.get_flags = get_flags;
  hid_init(&hid);

  Serial.println("logging ready");
  led_mode(L_BLINK);

  for (;;) {
    hid_poll();
    led_poll();
    if (digitalRead(4, 6) == LOW) {
      break;
    }
  }

  log_putc(0);
  if (flash_write(4, &log[4], LOG_SIZE - 4)) {
    Serial.println("upload ready");
    led_mode(L_OFF);
    led_poll();
    runBootloader();
  }
}
