// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "client.h"

#include "ch559.h"
#include "pwm1.h"
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

static void data_delayMicroseconds(struct JVSIO_DataClient* client,
                                   unsigned int usec) {
  client;
  delayMicroseconds(usec);
}

static void data_delay(struct JVSIO_DataClient* client, unsigned int msec) {
  client;
  delay(msec);
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
