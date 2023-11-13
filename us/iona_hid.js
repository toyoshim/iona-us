// Copyright 2023 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

export class HID {
  type = 'hid';
  iona = null;
  device = null;
  report = null;

  constructor(iona) {
    this.iona = iona;
  }

  async initialize() {
    this.device = (await navigator.hid.requestDevice({ filters: [] }))[0];
  }

  getDeviceDescriptor() {
    return this.iona.createPseudoDeviceDescriptor(
      this.device.productId, this.device.vendorId);
  }

  getConfigurationDescriptor() {
    this.prepareReportDescriptor();
    return this.iona.createPseudoConfigurationDescriptor(this.report.length);
  }

  getHidReportDescriptor() {
    return (new Uint8Array(this.report)).buffer;
  }

  listen(callback) {
    this.device.addEventListener('inputreport', e => {
      callback(e.data.buffer);
    });
    this.device.open();
  }

  prepareReportDescriptor() {
    if (this.report) {
      return;
    }
    const report = [];
    for (let collection of this.device.collections) {
      for (let input of collection.inputReports) {
        report.push(0x85);
        report.push(input.reportId);
        for (const item of input.items) {
          report.push(0x95);
          report.push(item.reportCount);
          report.push(0x75);
          report.push(item.reportSize);
          if (item.usageMaximum) {
            if (item.usageMaximum < 256) {
              report.push(0x29);
              report.push(item.usageMaximum);
            } else {
              report.push(0x2a);
              report.push(item.usageMaximum & 0xff);
              report.push(item.usageMaximum >> 8);
            }
          }
          if (item.usageMinimum) {
            if (item.usageMinimum < 256) {
              report.push(0x19);
              report.push(item.usageMinimum);
            } else {
              report.push(0x1a);
              report.push(item.usageMinimum & 0xff);
              report.push(item.usageMinimum >> 8);
            }
          }
          // TODO: {logical|physical}{Maximum|Minimum}, unit*
          if (!item.isRange) for (let usage of item.usages) {
            if (usage > 65535) {
              // Usage Page is encoded as an upper 16-bits
              const usagePage = (usage >> 16) & 0xffff;
              usage &= 0xffff;
              if (usagePage < 256) {
                report.push(0x05);
                report.push(usagePage);
              } else {
                report.push(0x06);
                report.push(usagePage & 0xff);
                report.push(usagePage >> 8);
              }
            }
            if (usage < 256) {
              report.push(0x09);
              report.push(usage);
            } else {
              report.push(0x0a);
              report.push(usage & 0xff);
              report.push(usage >> 8);
            }
          }
          let bits = 0;
          if (item.isConstant) bits |= 1;
          if (!item.isArray) bits |= 2;
          if (!item.isAbsolute) bits |= 4;
          if (item.wrap) bits |= 8;
          if (!item.isLinear) bits |= 16;
          if (!item.hasPreferredState) bits |= 32;
          if (item.hasNull) bits |= 64;
          if (item.isBufferredBytes) bits |= 256;
          // TODO: isRange, isVolatile
          if (bits < 256) {
            report.push(0x81);
            report.push(bits);
          } else {
            report.push(0x82);
            report.push(bits & 0xff);
            report.push(bits >> 8);
          }
        }
      }
    }
    this.report = report;
  }
}