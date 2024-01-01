---
layout: default_en
title: Layout Settings
permalink: /setting_en
---
# Layout Settings
This setting page is still under experiument for v2 series firmwares.
It may still contain bugs. Bug reports, feature requests are welcomed.
Also, if you send a layout for a game that requires a special configuration,
I'm happy to add it to the presets in this page.

---
## Preparation
Please follow the steps explained in [Firmware Updates](firmware).
As you may frequently use the setting page than firmware updates, I recommend you to get a special Type-A to Type-A connector explained in the page. It makes all process easy.

Once you complete the preparation, press the 'Find IONA-US' button below.
You need to select WinChipHead device and push the connect button in the dialog.

The same button will be changed to show 'Save Changes' to memorize the modified settings into the IONA-US board.

<button id="button"></button>
<pre id="status"></pre>

---
## Settings
#### Select / Copy
- Select one of Set 1 through to 6, edit following parameters, then click 'Decice' button.
- Without clicking 'Decide', changes will be disposed on selecting other sets.
- You will need to press 'Decide' also after copying data from other sets or presetrs.
- Once you finish all edits, click 'Save Changes' to store all changes into the IONA-US device.

| | | |
|-|-|-:|
|<select id="select"><option>Set 1</option><option>Set 2</option><option>Set 3</option><option>Set 4</option><option>Set 5</option><option>Set 6</option></select><br><button id="store">Decide</button>|<button id="storeToFile">Export to a File</button><br><button id="loadFromFile">Import from a File</button>|Copy from Another Set<br><select id="copy"><option>-</option><option>Set 1</option><option>Set 2</option><option>Set 3</option><option>Set 4</option><option>Set 5</option><option>Set 6</option></select><br>Copy from a Preset<br><select id="preset"><option>-</option></select>|

#### Core Settings
You can customize declaring device name, and supporting features.
If you will use it with some specific boards, e.g. ones from namco, you may need to make the JVS data signal level adjustment ON to be recognized. Otherwise, OFF is recommended.
Setting 0 for unused special inputs will optimize performance, or latency.

| | | |
|-|-|-:|
||JVS ID|<select id="id"><option>SEGA ENTERPRISES,LTD.</option><option>namco ltd.;JYU-PCB</option><option>namco ltd.;NA-JV</option><option>namco ltd.;TSS-I/O</option></select>
||# of Analog Input|<select id="ainc"><option>0</option><option>2</option><option>4</option><option>6</option><option>8</option></select>
||Bit width of Analog Input|<select id="ainw"><option>Auto</option><option>16-bits</option></select>
||# of Rotary Input|<select id="rotc"><option>0</option><option>2</option></select>
||# of Screen Position Input|<select id="scrc"><option>0</option><option>1</option><option>2</option></select>
||Bit width of Screen Position Input|<select id="scrw"><option>10-bits</option><option>16-bits</option></select>
||# of Analog Output (Fake)|<select id="aout"><option>0</option><option>2</option></select>
||Character Display Size (Fake)|<select id="disp"><option>N/A</option><option>16 x 1</option></select>
||JVS Dash Support|<select id="jvsd"><option>OFF</option><option>ON</option></select>
||JVS Data Signal Level Adjustmnent|<select id="jvss"><option>OFF</option><option>ON</option></select>

#### Analog Map
IONA-US can handle 6 kinds of analog input per 1P and 2P controllers respectively.
Here you can assign each analog input to several kind of special inputs for JVS.
Lever (Digital) maps the input to digital lever, 0 is for up and down, 1 is for left and right. This reassigned lever inputs will be also expanded to follow the button maps configured below.
Racing game may use analog inputs as wheels or padals.
Some games may use rotary inputs as DJ controllers or paddles, screen position inputs as zapper controllers, and so on. Each game may be creative here to have their own maps.

<table id="analog_map"></table>

#### Rapid Fire
You can edit 7 kinds of rapid fire patterns. You can find None or one of these 7 patterns in the following section to select a pattern for each button.
You can edit each sequence as passthrough or mask per frame, left to right.
Each sequence will be looped within the loop length.
If you check all boxes, all controlls are pass through, that means no rapid fires. If you check and uncheck step by step, it will fire 30 times per second.
If Invert flag is set, the button state is the report will be inverted.

<table id="rapid_fire_map"></table>

#### Button Map
You can edit button assignments per each digital button for P1 and P2 controller as each button can fire any combination of JVS P1 and P2 outputs.
This allow you to assign P1 button for P2 button. This is useful to map your amepad to control P1 lever and P2 lever via left and right analog sticks, e.g. for Virtual-On.
Each button can have a rapid fire setting.

<table id="button_map"></table>

<script src="https://toyoshim.github.io/CH559Flasher.js/CH559Flasher.js"></script>
<script>
window.uiMessages = {
  abort: 'Aborted',
  connected: 'Connected (Bootloader: ', 
  connectedInformation: ' / Setting format: v',
  error: 'Error: ',
  errorOnRead: 'Error on reading settings: ',
  findDevice: 'Find IONA-US',
  idle: 'Waiting for connecting to IONA-US',
  modifiedOnStore: 'Undecided changes exist. Do you save to a file without these changes?',
  modifiedOnSave: 'Undecided changes exist. Do you save without these changes?',
  noDevice: 'Unexpected error. Make sure your IONA-US is still connected',
  save: 'Save Changes',
  saved: 'Saved',
  unknownContinue: 'Connected, but cannot confirm known versions of IONA-US setting data. Do you agree to override it with the latest setting data?',
  unknownFileFormat: 'Unknown file format.',

  layoutAnalogInput: 'Controller Input',
  layoutAnalogOutputType: 'Assign Direction',
  layoutAnalogOutputNumber: 'Assign Index',
  layoutAnalogInvert: 'Invert',
  layoutAnalogAnalog: 'Analog',
  layoutAnalogOutputTypeNone: 'None',
  layoutAnalogOutputTypeLever: 'Lever (Digital)',
  layoutAnalogOutputTypeAnalog: 'Analog Input',
  layoutAnalogOutputTypeRotary: 'Rotary Input',
  layoutAnalogOutputTypeScreenPosition: 'Screen Position Input',

  layoutRapidFireSet: 'Sequence Set',
  layoutRapidFireSequence: 'Sequence Patterns',
  layoutRapidFireCycle: 'Length',
  layoutRapidFireInvert: 'Invert',
  layoutRapidFireSetPrefix: 'Set',

  layoutButtonInput: 'Input',
  layoutButtonOutput: 'Assign',
  layoutButtonRapidFire: 'Rapid Fire',
  layoutButtonRapidFireNone: 'None',
  layoutButtonRapidFirePrefix: 'Set',
  layoutButtonRapidFireGearUp: 'Gear Up',
  layoutButtonRapidFireGearDown: 'Gear Down',
};
</script>
<script src="layout_map.js"></script>
<script src="layout.js"></script>
<script src="layout_presets.js"></script>

#### Check raw layout
You can check a raw layout for a controller that is connected to your PC.
Code based on IONA's firmware runs on your browser, but via WebUSB and WebHID.
As it cannot access to the device directly, it might not be 100% compatible with actual IONA's behavior, but could be useful.
You can chooce the API to access to the device, and select your device name in the dialog.
Usual gamepad should be accessed via HID, and non-HID devices such as Xbox controllers should be done via USB.

<button id="hid">HID</button>
<button id="usb">USB</button>
<style id="style">
.test_up {}
.test_down {}
.test_left {}
.test_right {}
.test_b1 {}
.test_b2 {}
.test_b3 {}
.test_b4 {}
.test_b5 {}
.test_b6 {}
.test_b7 {}
.test_b8 {}
.test_b9 {}
.test_b10 {}
.test_b11 {}
.test_b12 {}
</style>

|<span class="test_up"> ↑ </span>|<span class="test_down"> ↓ </span>|<span class="test_left"> ← </span>|<span class="test_right"> → </span>|<span class="test_b1"> □ </span>|<span class="test_b2"> × </span>|<span class="test_b3"> ○ </span>|<span class="test_b4"> △ </span>|<span class="test_b5"> L1 </span>|<span class="test_b6"> R1 </span>|<span class="test_b7"> L2 </span>|<span class="test_b8"> R2 </span>|<span class="test_b9"> Share </span>|<span class="test_b10"> Option </span>|<span class="test_b11"> L3 </span>|<span class="test_b12"> R3 </span>|

| Analog 1 (LX) | <span id="test_a1" style="font-family: monospace">0x0000</span> |
| Analog 2 (LY) | <span id="test_a2" style="font-family: monospace">0x0000</span> |
| Analog 3 (RX) | <span id="test_a3" style="font-family: monospace">0x0000</span> |
| Analog 4 (RY) | <span id="test_a4" style="font-family: monospace">0x0000</span> |
| Analog 5 (LT) | <span id="test_a5" style="font-family: monospace">0x0000</span> |
| Analog 6 (RT) | <span id="test_a6" style="font-family: monospace">0x0000</span> |

<script type="module">
  import { IONA } from './iona_stub.js';
  import { HID } from './iona_hid.js';
  import { USB } from './iona_usb.js';

  let iona = null;
  let phy = null;

  function to04x(n) {
    const s = '000' + n.toString(16);
    return '0x' + s.substring(s.length - 4);
  }

  function loop() {
    const status = iona.checkStatus();
    if (!status.ready) {
      return;
    }
    const sheet = document.styleSheets[document.styleSheets.length - 1];
    for (let dir of ['up', 'down', 'left', 'right']) {
      const selector = '.test_' + dir;
      for (let rule of sheet.rules) {
        if (rule.selectorText == selector) {
          rule.style.color = status[dir] ? '#c00' : '#f0e7d5';
        }
      }
    }
    for (let i = 0; i < 12; ++i) {
      const selector = '.test_b' + (i + 1);
      for (let rule of sheet.rules) {
        if (rule.selectorText == selector) {
          rule.style.color = status.buttons[i] ? '#c00' : '#f0e7d5';
        }
      }
    }
    for (let i = 0; i < 6; ++i) {
      const id = 'test_a' + (i + 1);
      const element = document.getElementById(id);
      if (element) {
        element.innerText = to04x(status.analogs[i]);
      }
    }
    requestAnimationFrame(loop);
  }

  async function detect(ctor, e) {
    iona = new IONA();
    await iona.initialize();
    phy = new ctor(iona);
    await phy.initialize();
    iona.checkDeviceDescriptor(await phy.getDeviceDescriptor());
    await iona.checkConfigurationDescriptor(await phy.getConfigurationDescriptor());
    if (phy.type == 'hid') {
      iona.checkHidReportDescriptor(await phy.getHidReportDescriptor());
    }
    const status = iona.checkStatus();
    if (status.ready) {
      phy.listen(iona.checkHidReport.bind(iona));
      requestAnimationFrame(loop);
    }
  }

  document.getElementById('hid').addEventListener('click', detect.bind(this, HID));
  document.getElementById('usb').addEventListener('click', detect.bind(this, USB));
</script>