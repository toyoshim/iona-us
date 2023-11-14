// Copyright 2023 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

export class USB {
  type = 'usb';
  iona = null;
  device = null;

  constructor(iona) {
    this.iona = iona;
    this.iona.setInterface({
      select: this.select.bind(this),
      out: this.out.bind(this),
      in: this.in.bind(this)
    });
  }

  async initialize() {
    this.device = (await navigator.usb.requestDevice({ filters: [] }));
  }

  async getDeviceDescriptor() {
    if (!this.device.opened) {
      await this.device.open();
    }
    const desc = await this.device.controlTransferIn({
      requestType: 'standard',
      recipient: 'device',
      request: 6,  // GET_DESCRIPTOR
      value: 0x100,  // Device descriptor
      index: 0,
    }, 18);
    return desc.data.buffer;
  }

  async getConfigurationDescriptor() {
    const setup = {
      requestType: 'standard',
      recipient: 'device',
      request: 6,  // GET_DESCRIPTOR
      value: 0x200,  // Configuration descriptor
      index: 0,
    };
    const desc_header = await this.device.controlTransferIn(setup, 9);
    const u8 = new Uint8Array(desc_header.data.buffer);
    const desc_size = u8[2] | (u8[3] << 8);
    const desc = await this.device.controlTransferIn(setup, desc_size);
    return desc.data.buffer;
  }

  listen(callback) {
    this.requestAnimationFrame();
  }

  async select(intf) {
    await this.device.selectConfiguration(this.device.configurations[0].configurationValue);
    await this.device.claimInterface(this.device.configurations[0].interfaces[intf].interfaceNumber);
  }

  async out(ep, data) {
    this.device.transferOut(ep, data);
  }

  async in(ep, length) {
    const result = await this.device.transferIn(ep, length);
    return result.data.buffer;
  }

  requestAnimationFrame() {
    this.iona.poll();
    requestAnimationFrame(this.requestAnimationFrame.bind(this));
  }

}