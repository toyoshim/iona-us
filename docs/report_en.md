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

## Adavanced Report
As information that can be obtained via Chrome is really relimited,
we cannot get enough hints to support the device.
If information obtained above wasn't enough, and you are happy to help me,
please follow this step to gather information via IONA-US itself.

### Step 1; Flash debug firmware for the investigation
We use a dedicated firmware that gather detailed USB transaction logs.
You need to understand following risks.
- All user data on IONA-US will disapper, and will not be restored automatically.
- You need to flash IONA-US firmware at [Firmware Update](firmware_en) page once all are done.
- Obtained log contains raw USB protocol transactions that are communicated between IONA-US and your target device, and you need to agree on providing it as a file.
- You also need to agree that we will use the data to develop IONA's firmware.
- You also need to agree that we will use the data in our unit tests, and the the test code will be published at GitHub with your device data.

Connect IONA-US to your PC by following the instruction at [Firmware Update)(firmware_en) page.
You should not connect anything to the IONA's P2 USB port at this point.

<script src="https://toyoshim.github.io/CH559Flasher.js/CH559Flasher.js"></script>
<script>
async function flash() {
  const progressWrite = document.getElementById('flash_progress_write');
  const progressVerify = document.getElementById('flash_progress_verify');
  const error = document.getElementById('flash_error');
  progressWrite.value = 0;
  progressVerify.value = 0;
  error.innerText = '';
  const flasher = new CH559Flasher();
  await flasher.connect();
  if (flasher.error) {
    error.innerText = 'Connection failed: ' + flasher.error;
    return;
  }
  await flasher.erase();
  const response = await fetch('firmwares/logger.bin');
  if (response.ok) {
    const bin = await response.arrayBuffer();
    await flasher.write(bin, rate => progressWrite.value = rate);
    await flasher.verify(bin, rate => progressVerify.value = rate);
    error.innerText = flasher.error ? flasher.error : 'Succeeded';
  } else {
    error.innerText = 'Cannot find the logging firmware';
  }
  await flasher.boot();
}
</script>
<button onclick="flash();">Agreed and flush the debug firmware</button>

| | |
|-|-|
|Write|0% <progress id="flash_progress_write" max=1 value=0></progress> 100%|
|Verify|0% <progress id="flash_progress_verify" max=1 value=0></progress> 100%|

Result
<pre id="flash_error"></pre>

### Step 2: Logging target device information
If everything went well, you will see the LED is blinking.
Once you ensure the LED blinking, connect your target device to the IONA's P2 USB port.
You may see the LED is turned OFF if the all logging buffer is fullfilled.
Even if it's still blinking, we may have enough logging data. So, never mind.

It will be finished immediately, and you can go to the next step now.
Press 'SERVICE' button here.
It will make the LED turned off.
This indicates the log data is stored to the persistent area,
and the device is ready to upload the data to your PC.
You can obtain the log at the next step.

### Step 3: Obtain the log
Press the button below, and you will see the device selection dialog.
Please choose the device you chose at the first step.

<button onclick="read();">Obtain the log</button>

<script>
async function read() {
  const progressRead = document.getElementById('progress_read');
  const error = document.getElementById('read_error');
  progressRead.value = 0;
  error.innerText = '';

  const flasher = new CH559Flasher();
  await flasher.connect();
  if (flasher.error) {
    error.innerText = 'Connection failed: ' + flasher.error;
    return;
  }
  const log = new Uint8Array(1024);
  for (let i = 0; i < 1024; i += 32) {
    let buffer = await flasher.readDataInRange(0xf000 + i, 32);
    if (!buffer) {
      setStatus('Read failed: ' + flasher.error);
      return;
    }
    let b8 = new Uint8Array(buffer)
    for (let j = 0; j < 32; ++j) {
      log[i + j] = b8[j];
      progressRead.value = (i + j) / 1023;
    }
  }
  const a = document.createElement('a');
  const blob = new Blob([log], { type: 'octet/stream' });
  const url = window.URL.createObjectURL(blob);
  a.href = url;
  a.download = 'iona-usb-log.bin';
  a.click();
}
</script>

| | |
|-|-|
|Read|0% <progress id="progress_read" max=1 value=0></progress> 100%|

Result
<pre id="read_error"></pre>

