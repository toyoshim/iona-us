---
layout: default_en
title: Report
permalink: /report_en
---
# Report
---

## Request for Unsupported Devices
Here, you can report detailed information on a device that does not work with IONA to request support.
As this page uses WebHID API to obtain device information, you need to visit this page with Google Chrome or Chromium based browsers that enables WebHID support.
Obtained information is only about the product itself, and does not contain any privacy information such as PC environment specific parameters, serial numbers, or something about you.
These information might not be enough to support the device. In such case, author may ask additional information to support the device.
We don't ensure that all reported devices will be supported though we try. It may take a long time, or we may give up to support some devices if it's difficult.

## Known Issues, or Unresolved Reports
[This site](https://github.com/toyoshim/iona-us/issues) contains unresolved reported issues.
If there is already a request for the device, please wait until the issue is resolved.
It's good to add some comments in the thread to encourage us to prioritize the issue, or you can get a notification when the issue is fixed.

## How to Report
Please connect the device under the issue to the PC. You need to know the device name that appears on the PC.
Please push the connect button below. You will see a dialog as follows.
This dialog should contain the device name in the list. You need to choose the one you want to report, then press connect button.

![Prompt](prompt.png)

Once the device is connected correctly, device information appears in the controller information section below.
Please copy all the information, then report it to author's [Twitter account](https://twitter.com/toyoshim).
DM is open for everyone.

---
<button onclick="connect();">Connect</button>

---
## Controller Information
<pre id="info"></pre>

<script>
function to2x(v) {
  return '0x' + ('0' + v.toString(16)).substr(-2, 2);
}
async function connect() {
  const devices = await navigator.hid.requestDevice({filters: []});
  const device = devices[0];
  const info = [];
  info.push('"' + device.productName + '"');
  info.push(' VID: 0x' + ('000' + device.vendorId.toString(16)).substr(-4, 4));
  info.push(' PID: 0x' + ('000' + device.productId.toString(16)).substr(-4, 4));
  info.push('[Top Collections]');
  info.push(' #: ' + device.collections.length);
  for (let i = 0; i < device.collections.length; ++i) {
    info.push(' [Collection ' + i + ']');
    info.push('  children#: ' + device.collections[i].children.length);
    info.push('  feature#: ' + device.collections[i].featureReports.length);
    info.push('  input#: ' + device.collections[i].inputReports.length);
    info.push('  output#: ' + device.collections[i].outputReports.length);
    const input = device.collections[i].inputReports;
    let data = [];
    for (let j = 0; j < input.length; ++j) {
      data.push('0x85');
      data.push(to2x(input[j].reportId));
      for (const item of input[j].items) {
        data.push('0x95');
        data.push(to2x(item.reportCount));
        data.push('0x75');
        data.push(to2x(item.reportSize));
        if (item.usageMaximum) {
          if (item.usageMaximum < 256) {
            data.push('0x29');
            data.push(to2x(item.usageMaximum));
          } else {
            data.push('0x2a');
            data.push(to2x(item.usageMaximum & 0xff));
            data.push(to2x(item.usageMaximum >> 8));
          }
        }
        if (item.usageMinimum) {
          if (item.usageMinimum < 256) {
            data.push('0x19');
            data.push(to2x(item.usageMinimum));
          } else {
            data.push('0x1a');
            data.push(to2x(item.usageMinimum & 0xff));
            data.push(to2x(item.usageMinimum >> 8));
          }
        }
        // TODO: {logical|physical}{Maximum|Minimum}, unit*
        if (!item.isRange) for (let usage of item.usages) {
          if (usage > 65535) {
            // Usage Page is encoded as an upper 16-bits
            const usagePage = (usage >> 16) & 0xffff;
            usage &= 0xffff;
            if (usagePage < 256) {
              data.push('0x05');
              data.push(to2x(usagePage));
            } else {
              data.push('0x06');
              data.push(to2x(usagePage & 0xff));
              data.push(to2x(usagePage >> 8));
            }
          }
          if (usage < 256) {
            data.push('0x09');
            data.push(to2x(usage));
          } else {
            data.push('0x0a');
            data.push(to2x(usage & 0xff));
            data.push(to2x(usage >> 8));
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
          data.push('0x81');
          data.push(to2x(bits));
        } else {
          data.push('0x82');
          data.push(to2x(bits & 0xff));
          data.push(to2x(bits >> 8));
        }
      }
      info.push('  input' + j + ': ' + data.join(', '));
      data = [];
    }
  }
  info.push('END');
  document.getElementById('info').innerText = info.join('\n');
}
</script>