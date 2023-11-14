// Copyright 2023 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

export class IONA {
  imports = {
    env: {
      emscripten_memcpy_js: () => console.log('emscripten_memcpy_js'),
      timer3_tick_raw: null,
      iona_usb_out: null,
      iona_usb_in: null,
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
  intf = {};

  constructor() {
    this.imports.env.timer3_tick_raw = this.timer3_tick_raw.bind(this);
    this.imports.env.iona_usb_out = this.usb_out.bind(this);
    this.imports.env.iona_usb_in = this.usb_in.bind(this);
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

  setInterface(intf) {
    this.intf = intf;
  }

  storeData(buffer) {
    const u8 = new Uint8Array(buffer);
    for (let i = 0; i < u8.byteLength; ++i) {
      this.u8[this.communication_buffer_address + i] = u8[i];
    }
  }

  loadData(address, size) {
    const u8 = new Uint8Array(size);
    for (let i = 0; i < size; ++i) {
      u8[i] = this.u8[address + i];
    }
    return u8.buffer;
  }

  checkDeviceDescriptor(buffer) {
    this.storeData(buffer);
    this.exports.iona_usb_host_check_device_desc(this.communication_buffer_address);
  }

  async checkConfigurationDescriptor(buffer) {
    this.storeData(buffer);
    const intf = this.exports.iona_usb_host_check_configuration_desc(this.communication_buffer_address);
    if (this.intf.select) {
      await this.intf.select(intf);
    }
  }

  checkHidReportDescriptor(buffer) {
    this.storeData(buffer);
    this.exports.iona_usb_host_check_hid_report_desc(this.communication_buffer_address);
  }

  checkHidReport(buffer) {
    this.storeData(buffer);
    this.exports.iona_usb_host_check_hid_report(this.communication_buffer_address, buffer.byteLength);
  }

  checkStatus() {
    const result = {
      ready: this.exports.iona_is_device_ready(),
      type: this.exports.iona_get_device_type(),
    };
    if (result.ready) {
      const digitals = this.exports.iona_get_digital_states();
      const buttons = [];
      for (let bit = 0x0200; bit != 2; bit >>= 1) {
        buttons.push((digitals & bit) != 0);
      }
      buttons.push((digitals & 0x4000) != 0);
      buttons.push((digitals & 0x8000) != 0);
      buttons.push((digitals & 0x0002) != 0);
      buttons.push((digitals & 0x0001) != 0);
      const analogs = [];
      for (let i = 0; i < 6; ++i) {
        analogs.push(this.exports.iona_get_analog_state(i));
      }
      result.up = (digitals & 0x2000) != 0;
      result.down = (digitals & 0x1000) != 0;
      result.left = (digitals & 0x0800) != 0;
      result.right = (digitals & 0x0400) != 0;
      result.buttons = buttons;
      result.analogs = analogs;
    }
    return result;
  }

  poll() {
    this.exports.iona_poll();
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

  usb_out(ep, data, size) {
    if (!this.intf.out) {
      return false;
    }
    this.intf.out(ep, this.loadData(data, size)).then(() => {
      this.exports.iona_transaction_complete(0);
    });
    return true;
  }

  usb_in(ep, size) {
    if (!this.intf.in) {
      return false;
    }
    this.intf.in(1, size).then(buffer => {
      this.storeData(buffer);
      this.exports.iona_transaction_complete(buffer.byteLength);
    });
    return true;
  }
}