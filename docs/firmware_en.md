---
layout: default_en
title: Firmware Updates
permalink: /firmware_en
---
# Firmware Updates
---
## Caution
You can update your device's firmware to support more devices or fix issues here.
But as this page uses WebUSB API, you need to visit by Google Chrome or Chromium based browser that enables WebUSB API support.

## Preparation
There are 4 through-hole on the IONA board as you can see in the white circle below.
Before supplying power, you need to shorten the yellow hole pair and the red hole pair respectively.
This allows the JVS port to use USB data line, D+ and D-, to communicate with PC over USB protocol.

![Figure v1.xx](fw_fig.jpg)
![Figure v2.xx](fw_fig_v2.jpg)

Here, we introduce some simple ways to shorten the holes.

The first approach is to use a thin wire arch.
You may be able to use a staple, but it might be too thin to provide a stable connection.

The second one is to use two pairs of tweezers.
It may look a wild way, but actually it's very easy to get a stable connection.
It's recommended if you have two pairs of tweezers.

The last one is the ideal approach.
You install pin headers, and connect them with jumpper pins.
As it needs soldering, it isn't a good way for end users.

![Wires](fw_wire.jpg)
![Tweezers](fw_pinset.jpg)
![Jumpper](fw_jump.jpg)

## Connect to PC
First, you need to keep the SERVICE button on the board pressed, then start supplying the power.
If the SERVICE button is correctly kept pressed until it boots, LED will not be on.
This is the signal that IONA boots to the firmware update mode.
If LED blinks, IONA runs in a normal mode, and you need to power off, and retry.

Alternatively, you can use a special USB cable from Type A to A.
You can connect your PC and 1P connector by such cable with pressing SERVICE to enter the firmware update mode.
You should not connect power supply and don't need jumpers.

Once it boots to the firmware update mode, connect IONA to your PC over USB via JVS port.
You should not connect a gamepad to the P1 USB port. It conflicts with the USB connection to the PC host, and makes communication unstable.

## WinUSB Settings (only for the first time on Windows)
When you connect IONA to your PC first time, it appears as an unknown device in the device manager.
If there are multiple unknown devices, IONA is the one that shows `USB\VID_4348&PID_55E0\...` at the property in details tab.
You need to install the system provinding default driver called as WinUSB for the device.

You can follow the instruction explained at the Microsoft official site, [Installing WinUSB by specifying the system-provided device class](https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/winusb-installation#installing-winusb-by-specifying-the-system-provided-device-class).

Some users said retry will help if installation failed.
As this firmware update mode is provided by the chip vendor, I have no other ideas to mitigate this problem.

![Device Manager](fw_devman.png)

## Firmware Selection
You can pick up any firmware version you install.
When you push the flash button after selecting the firmware version, you will see a prompt as below.

You will see only devices that have a specified vendor ID and product ID. Thus, you will see only one choice here usually.
If you see multiple choices, another device that uses the same chip by chance would be connected in a firmware update mode to your PC. It should rarely happen, but just in case.

If you can see no choice, please check if there are following errors.

- Through-holls are not shortened correctly, or connection is unstable.
- Does not supply power over micro USB.
- Power is supplied, but doesn't boot to the firmware update mode with the SERVICE button.

As the initial firmware is also flashed at this site, you can expect it just works unless it is damanged after shipping.
So, probably you can find one of these issue, and will solve it.

If you can see the prompt as expected, select the device and press the connect button.
It starts flashing, and following UIs show it's progress.
Even if it fails during flashing, device won't be broken, and you can just retry safely.
Boot mode should not be broken by flashing.

If you see a persistent issue, please contact the author.

![Prompt](fw_prompt.png)

## Confirmation
One the firmware update finishes, please disconnect IONA from your PC, and stop supplying power to reboot.
If it bookts and starts blinking after supplying power again, you succeeded the update.
Please ensure the shortened pins are open again before connecting it to JVS systems.
It may be ok if it does not long, but it may damage the device to connect over JVS with through-holls shortened.

## Firmware History
- Ver 1.00 Sample for KVClab., there is a bug that coin could not be decreased.
- Ver 1.01 Initial firmware for the first lot.
- Ver 1.02 Fix USB host behaviors' spec violation, and add some more device supports.
- Ver 1.02a Fix Xbox controller unstability issue, and add some more device supports.
- Ver 1.03 Improve composite device and REMOTE WAKEUP support, and add some more device supports.
- Ver 1.04 Add some more device supports.
- Ver 1.10 Add twinstick mode.
- Ver 1.20 Add NAOMI Mahjong mode.
- Ver 1.21 Improve JVS electrical characteristics.
- Ver 1.22 Requre buttons pressed over 0.5 seconds to enter the layout mode.
- Ver 1.23 Wait more time before start a reset sequence on detecting device.
- Ver 1.24 Stability update to backport Ver 1.33 changes.
- Ver 1.30 P1 analog X/Y, P2 analog X/Y are assigned to analog 0-3.
- Ver 1.31 Guncon3 support.
- Ver 1.32 P1 analog X/Y are also assigned to newly added 2ch Rotary inputs.
- Ver 1.33 Improved JVS compatibility for v1.20+ PCB.
- Ver 1.34 Reduce Analog channel to 4ch to avoid I/O error on Guilty Gear series, and fix the issue D/H/L/Pon are not responsible for Usagi.
- Ver 1.35 Allow to send inputs even while pressing SERVICE+TEST. You can enter the settings mode iff you keep them pressed for 0.5-5.0 sec.
- Ver 1.40 Add analog layout, option config, and screen position input support.
- Ver 1.41 Add namco NA-JV compatible mode and several option conmfigs.
- Ver 1.42b Changed to ack with successful status for the main ID command.

## Firmware Compatibility
All controllers that conform Xbox 360, or Xbox One series protocols are expected to work fine.
Other USB HID devices support might get to be broken unexpectedly on supporting other new devices.
We know HID descriptors on following devices, and have some automated unit tests. So, it's expected to keep better compatibility. We can add more automated tests when you report device information.
If you find a device that isn't in the list, but works, it's a good idea to report information to get stable supports.

Also, the internal format to hold the user settings is changed at firmware Ver 1.40. As a reuslt, if you write a firmware those format is different from one for current firmware, all your configurations will be reset.

|Device Name|Confirmed Version|Note|
|-|-|-|
|(Xbox 360 protocol controllers)|1.00|1.02a and later are recommended|
|(Xbox One series protocol controllers)|1.00|1.02a and later are recommended|
|(Keyboard supporting boot mode)|1.20|supported by NAOMI Mahjong mode|
|Guncon3|1.31||designed for DeathCrimson OX|
|Horipad FPS plus for PlayStation 4|1.00|PS3 mode also works|
|Wireless Controller（DUALSHOCK 4 - CUH-ZCT1J）|1.02||
|Wireless Controller（DUALSHOCK 4 - CUH-ZCT2J）|1.03||
|Horipad mini for Nintendo Switch|1.00||
|Nintendo Switch Pro Controller|1.04||
|Nintendo Switch Joy-Con Charger Glip|1.04||
|CYBER Arcade Stick|1.23||
|6B Controller (MEGADRIVE mini)|1.02a||
|Xin-Mo Controller (*1)|1.20||

(*1) Controller used by the Pasocade full HD table cab. PS3 dock model

---
## Firmware Update
This is the real UI to update firmware. Flash button will actually flash the chosen firmware.
Ver 1.03  is hidden as it has many known issues.

Now 1.4x series are experimental versions, and the latest 1.3x series, selected by default, is expected to be the latest stable version.

<script src="https://toyoshim.github.io/CH559Flasher.js/CH559Flasher.js"></script>
<script>
async function flash() {
  const firmwares = [
    'firmwares/us_v1_00.bin',  // Ver 1.00
    'firmwares/us_v1_01.bin',  // Ver 1.01
    'firmwares/us_v1_02.bin',  // Ver 1.02
    'firmwares/us_v1_02a.bin',  // Ver 1.02a
    'firmwares/us_v1_04.bin',  // Ver 1.04
    'firmwares/us_v1_10.bin',  // Ver 1.10
    'firmwares/us_v1_20.bin',  // Ver 1.20
    'firmwares/us_v1_21.bin',  // Ver 1.21
    'firmwares/us_v1_22.bin',  // Ver 1.22
    'firmwares/us_v1_23.bin',  // Ver 1.23
    'firmwares/us_v1_24.bin',  // Ver 1.24
    'firmwares/us_v1_30.bin',  // Ver 1.30
    'firmwares/us_v1_31.bin',  // Ver 1.31
    'firmwares/us_v1_32.bin',  // Ver 1.32
    'firmwares/us_v1_33.bin',  // Ver 1.33
    'firmwares/us_v1_34.bin',  // Ver 1.34
    'firmwares/us_v1_35.bin',  // Ver 1.35
    'firmwares/us_v1_40.bin',  // Ver 1.40
    'firmwares/us_v1_41.bin',  // Ver 1.41
    'firmwares/us_v1_42b.bin',  // Ver 1.42b
  ];
  const progressWrite = document.getElementById('progress_write');
  const progressVerify = document.getElementById('progress_verify');
  const error = document.getElementById('error');
  progressWrite.value = 0;
  progressVerify.value = 0;
  error.innerText = '';

  const flasher = new CH559Flasher();
  await flasher.connect();
  await flasher.erase();
  const url = firmwares[document.getElementById('version').selectedIndex];
  const response = await fetch(url);
  if (response.ok) {
    const bin = await response.arrayBuffer();
    await flasher.write(bin, rate => progressWrite.value = rate);
    await flasher.verify(bin, rate => progressVerify.value = rate);
    error.innerText = flasher.error ? flasher.error : 'Succeeded';
  } else {
    error.innerText = 'Firmware not found';
  }
}
</script>

<select id="version">
<option>Ver 1.00</option>
<option>Ver 1.01</option>
<option>Ver 1.02</option>
<option>Ver 1.02a</option>
<option>Ver 1.04</option>
<option>Ver 1.10</option>
<option>Ver 1.20</option>
<option>Ver 1.21</option>
<option>Ver 1.22</option>
<option>Ver 1.23</option>
<option>Ver 1.24</option>
<option>Ver 1.30</option>
<option>Ver 1.31</option>
<option>Ver 1.32</option>
<option>Ver 1.33</option>
<option>Ver 1.34</option>
<option selected>Ver 1.35</option>
<option>Ver 1.40</option>
<option>Ver 1.41</option>
<option>Ver 1.42b</option>
</select>
<button onclick="flash();">Flash</button>

| | |
|-|-|
|Write|0% <progress id="progress_write" max=1 value=0></progress> 100%|
|Verify|0% <progress id="progress_verify" max=1 value=0></progress> 100%|

Result
<pre id="error"></pre>
