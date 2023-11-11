// Copyright 2023 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

export class IONA {
  imports = {
    env: {
      emscripten_memcpy_js: () => console.log('emscripten_memcpy_js'),
      timer3_tick_raw: null,
    },
    wasi_snapshot_preview1: {
      fd_write: null,
    },
  };

  u8 = null;
  u32 = null;
  communication_buffer_address = 0;
  wasm = null;
  exports = null;

  constructor() {
    this.imports.env.timer3_tick_raw = this.timer3_tick_raw.bind(this);
    this.imports.wasi_snapshot_preview1.fd_write = this.fd_write.bind(this);
  }

  async initialize() {
    WebAssembly.instantiateStreaming(fetch("iona.wasm"), this.imports).then((obj => {
      this.wasm = obj;
      const memory = obj.instance.exports.memory.buffer;
      this.u8 = new Uint8Array(memory);
      this.u32 = new Uint32Array(memory);
      this.exports = obj.instance.exports;

      this.communication_buffer_address = this.exports.iona_get_communication_buffer();
      this.exports.iona_init();
    }).bind(this));
  }

  storeData(buffer) {
    const u8 = new Uint8Array(buffer);
    for (let i = 0; i < u8.byteLength; ++i) {
      this.u8[this.communication_buffer_address + i] = u8[i];
    }
  }

  checkDeviceDescriptor(buffer) {
    this.storeData(buffer);
    this.exports.iona_usb_host_check_device_desc(this.communication_buffer_address);
  }

  checkConfigurationDescriptor(buffer) {
    this.storeData(buffer);
    this.exports.iona_usb_host_check_configuration_desc(this.communication_buffer_address);
  }

  checkHidReportDescriptor(buffer) {
    this.storeData(buffer);
    this.exports.iona_usb_host_check_hid_report_desc(this.communication_buffer_address);
  }

  checkHidReport(buffer) {
    this.storeData(buffer);
    this.exports.iona_usb_host_check_hid_report(this.communication_buffer_address, buffer.byteLength);
  }

  createPseudoDeviceDescriptor(pid, vid) {
    const desc = new Uint8Array(18);
    desc[0] = 0x12;  // bLength
    desc[1] = 0x01;  // bDescriptorType
    desc[2] = 0x01;  // bcdUSB
    desc[3] = 0x01;  //   v1.1
    desc[4] = 0x03;  // bDeviceClass HID
    desc[5] = 0x00;  // bDeviceSubClass
    desc[6] = 0x00;  // bDeviceProtocol
    desc[7] = 0x40;  // bMaxPacketSize0
    desc[8] = vid >> 8;
    desc[9] = vid & 0xff;
    desc[10] = pid >> 8;
    desc[11] = pid & 0xff;
    desc[12] = 0x00;  // bcdDevice L
    desc[13] = 0x00;  // bcdDevice H
    desc[14] = 0x00;  // iManufacturer
    desc[15] = 0x00;  // iProduct
    desc[16] = 0x00;  // iSerialNumber
    desc[17] = 0x00;  // bNumConfigurations
    return desc.buffer;
  }

  createPseudoConfigurationDescriptor(length) {
    const desc = new Uint8Array(9 + 9 + 7 + 9 + 1);
    desc[0] = 0x09;  // bLength
    desc[1] = 0x02;  // bDescriptorType: CONFIGURATIO)N
    desc[2] = 0x22 + 1;  // wTotalLength L
    desc[3] = 0x00;  // wTotalLength H
    // ...

    desc[9] = 0x09;  // bLength
    desc[10] = 0x04;  // bDescriptorType: INTERFACE
    desc[11] = 0x01;  // bInterfaceNumber
    desc[12] = 0x00;  // bAlternateSetrting
    desc[13] = 0x01;  // bNumEndpoints
    desc[14] = 0x03;  // bInterfaceClass: HID
    desc[15] = 0x00;  // bInterfaceSubClass
    desc[16] = 0x00;  // bInterfaceProtocol
    desc[17] = 0x01;  // iInterface

    desc[18] = 0x07;  // bLenbth
    desc[19] = 0x05;  // bDesciptorType: ENDPOINT
    desc[20] = 0x81;  // bEndpointAddress
    desc[21] = 0x03;  // bmAttributes
    // ...

    desc[25] = 0x09 + 1;  // bLength
    desc[26] = 0x21;  // bDesciptorType: HID
    desc[27] = 0x01;  // bcdHID
    desc[28] = 0x01;  //   v1.1
    desc[29] = 0x00;  // bCountryCode
    desc[30] = 0x01;  // bNumDescriptors
    desc[31] = 0x22;  // bReportDescriptorType: HID_REPORT
    // Note: need a padding byte here to access uint16 value correctly.
    desc[33] = length & 0xff;
    desc[34] = length >> 8;
    return desc.buffer;
  }

  timer3_tick_raw(result) {
    const msec_x16 = (performance.now() * 16) | 0;
    this.u32[result >> 2] = msec_x16;
    return 0;
  }

  fd_write(fd, iov, iovcnt, pnum) {
    let n = 0;
    const chars = [];
    for (let i = 0; i < iovcnt; ++i) {
      const ptr = this.u32[(iov >> 2) + i * 2 + 0];
      const len = this.u32[(iov >> 2) + i * 2 + 1];
      for (let j = 0; j < len; ++j) {
        chars.push(String.fromCharCode(this.u8[ptr + j]));
      }
      n += len;
    }
    this.u32[pnum >> 2] = n;
    console.log(chars.join(''));
    return 0;
  }
}