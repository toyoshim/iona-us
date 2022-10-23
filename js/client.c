// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "client.h"

#include "ch559.h"
#include "pwm1.h"
#include "serial.h"
#include "uart1.h"

#include "jvsio/JVSIO_c.h"

static int data_available(struct JVSIO_DataClient* client) {
  client;
  return uart1_ready() ? 1 : 0;
}

static void data_setInput(struct JVSIO_DataClient* client) {
  client;
  // Wait until all sending data go out.
  while (!uart1_sent())
    ;
  // Activate pull-down.
  pinMode(4, 6, OUTPUT);
}

static void data_setOutput(struct JVSIO_DataClient* client) {
  client;
  // Inactivate pull-down.
  pinMode(4, 6, INPUT);
}

static void data_startTransaction(struct JVSIO_DataClient* client) {
  client;
}

static void data_endTransaction(struct JVSIO_DataClient* client) {
  client;
}

static uint8_t data_read(struct JVSIO_DataClient* client) {
  client;
  return uart1_recv();
}

static void data_write(struct JVSIO_DataClient* client, uint8_t data) {
  client;
  uart1_send(data);
}

static bool data_setCommSupMode(struct JVSIO_DataClient* client,
                                enum JVSIO_CommSupMode mode,
                                bool dryrun) {
  client;
  dryrun;
  // No Dash support.
  return mode == k115200;
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

void data_client(struct JVSIO_DataClient* client) {
  client->available = data_available;
  client->setInput = data_setInput;
  client->setOutput = data_setOutput;
  client->startTransaction = data_startTransaction;
  client->endTransaction = data_endTransaction;
  client->read = data_read;
  client->write = data_write;
  client->setCommSupMode = data_setCommSupMode;
  client->dump = data_dump;

  uart1_init(UART1_RS485, UART1_115200);

  // Additional D+ pull-down that is activated only on receiving.
  digitalWrite(4, 6, LOW);
}

static void sense_begin(struct JVSIO_SenseClient* client) {
  client;
  pwm1_init();
}

static void sense_set(struct JVSIO_SenseClient* client, bool ready) {
  client;
  pwm1_enable(!ready);
}

static bool sense_isReady(struct JVSIO_SenseClient* client) {
  client;
  // can be true for single node.
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

static void led_begin(struct JVSIO_LedClient* client) {
  client;
  pinMode(1, 6, OUTPUT);
  digitalWrite(1, 6, LOW);
}

static void led_set(struct JVSIO_LedClient* client, bool ready) {
  client;
  digitalWrite(1, 6, ready ? HIGH : LOW);
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