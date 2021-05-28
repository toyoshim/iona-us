// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

#include "ch559.h"
#include "client.h"
#include "jvsio/JVSIO_c.h"

#define VER "1.10g"

static const char id[] = "SEGA ENTERPRISES,LTD.compat;MP01-IONA-JS(CH559);ver" VER;
//static const char id[] = "SEGA ENTERPRISES,LTD.compat;MP07-IONA-US;ver" VER;

static struct JVSIO_DataClient data;
static struct JVSIO_SenseClient sense;
static struct JVSIO_LedClient led;

static int8_t coin_index_bias = 0;

static void loop(struct JVSIO_Lib* io) {
  uint8_t len;
  uint8_t* data = io->getNextCommand(io, &len, 0);
  if (!data) {
    //dipsw.Update();
    //jamma.Update(dipsw.GetSwapMode());
    return;
  }

  switch (*data) {
   case kCmdReset:
    coin_index_bias = 0;
    //jamma.Initialize();
    break;
   case kCmdIoId:
    io->pushReport(io, kReportOk);
    for (uint8_t i = 0; id[i]; ++i)
      io->pushReport(io, id[i]);
    io->pushReport(io, 0x00);
    break;
   case kCmdFunctionCheck:
    io->pushReport(io, kReportOk);

    io->pushReport(io, 0x01);  // sw
    io->pushReport(io, 0x02);  // players
    io->pushReport(io, 0x0C);  // buttons
    io->pushReport(io, 0x00);

    io->pushReport(io, 0x02);  // coin
    io->pushReport(io, 0x02);  // slots
    io->pushReport(io, 0x00);
    io->pushReport(io, 0x00);

    io->pushReport(io, 0x03);  // analog inputs
    io->pushReport(io, 0x08);  // channels
    io->pushReport(io, 0x00);  // bits
    io->pushReport(io, 0x00);

    io->pushReport(io, 0x12);  // general purpose driver
    io->pushReport(io, 0x08);  // slots
    io->pushReport(io, 0x00);
    io->pushReport(io, 0x00);

    io->pushReport(io, 0x00);
    break;
   case kCmdSwInput:
    //dipsw.Sync();
    //jamma.Sync();
    io->pushReport(io, kReportOk);
    if (data[1] == 2 && data[2] == 2) {
      //bool mode = dipsw.GetRapidMode();
      //bool mask = dipsw.GetRapidMask();
      //io.pushReport(jamma.GetSw(0, mode, mask));
      //io.pushReport(jamma.GetSw(1, mode, mask));
      //io.pushReport(jamma.GetSw(2, mode, mask));
      //io.pushReport(jamma.GetSw(3, mode, mask));
      //io.pushReport(jamma.GetSw(4, mode, mask));
      io->pushReport(io, 0);
      io->pushReport(io, 0);
      io->pushReport(io, 0);
      io->pushReport(io, 0);
      io->pushReport(io, 0);
    } else {
      Serial.println("Err CmdSwInput");
    }
    break;
   case kCmdCoinInput:
    io->pushReport(io, kReportOk);
    if (data[1] <= 2) {
      for (uint8_t i = 0; i < data[1]; ++i) {
        io->pushReport(io, (0 << 6) | 0);
        //io->pushReport(io, jamma.GetCoin(i));
        io->pushReport(io, 0);
      }
    } else {
      Serial.println("Err CmdCoinInput");
    }
    break;
   case kCmdAnalogInput:
    io->pushReport(io, kReportOk);
    for (uint8_t channel = 0; channel < data[1]; ++channel) {
      io->pushReport(io, 0x80);
      io->pushReport(io, 0x00);
    }
    break;
   case kCmdCoinSub:
   case kCmdCoinAdd:
    // Coin slot index should start with 1, but some PCB seem to expect starting
    // with 0. Following code detects the slot index 0 and sets the bias to 1
    // so that it offsets.
    if (data[1] == 0)
      coin_index_bias = 1;
    //if (*data == kCmdCoinSub)
    // jamma.SubCoin(data[1] + coin_index_bias - 1, data[3]);
    //else
    // jamma.AddCoin(data[1] + coin_index_bias - 1, data[3]);
    io->pushReport(io, kReportOk);
    break;
   case kCmdDriverOutput:
    //jamma.DriveOutput(data[2]);
    io->pushReport(io, kReportOk);
    break;
  }
}

void main() {
  initialize();
  Serial.println(id);

  data_client(&data);
  sense_client(&sense);
  led_client(&led);

  struct JVSIO_Lib* io = JVSIO_open(&data, &sense, &led, 1);
  io->begin(io);
  Serial.println("boot");

  for (;;)
    loop(io);
}