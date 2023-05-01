// TODO
// - Store
// - Select Setting
// - Copy from another setting
// - Rapid fire template
// - Device support.
// - VirtualOn template
function select(id, index) {
  const select = document.getElementById(id);
  if (!select) {
    console.log(id, index);
    console.assert(false);
  }
  select.options.selectedIndex = index;
}

function check(id, checked) {
  const checkbox = document.getElementById(id);
  if (!checkbox) {
    console.log(id, checked);
    console.assert(false);
  }
  checkbox.checked = checked;
}

function enable(id, enabled) {
  const checkbox = document.getElementById(id);
  if (!checkbox) {
    console.log(id, enabled);
    console.assert(false);
  }
  checkbox.disabled = !enabled;
}

function applySequenceWidth(index, width) {
  for (let i = 1; i <= 8; ++i) {
    enable('p' + (index + 1) + i, i <= width);
  }
}

function applyData(data) {
  // Core
  const jvsDeviceId = (data[0] >> 5) & 7;
  const analogInputCount = (data[0] >> 2) & 7;
  const analogInputWidth = data[0] & 3;
  const rotaryInputCount = (data[1] >> 5) & 7;
  const screenPositionCount = (data[1] >> 2) & 7;
  const screenPositionWidth = data[1] & 3;
  const analogOutputCount = (data[2] >> 6) & 3;
  const characterDisplayWidthHeight = (data[2] >> 3) & 7;
  const JvsDashSupport = (data[2] >> 2) & 1;
  const JvsSignalAdjust = (data[2] >> 1) & 1;
  select('id', jvsDeviceId);
  select('ainc', analogInputCount);
  select('ainw', analogInputWidth);
  select('rotc', rotaryInputCount);
  select('scrc', screenPositionCount);
  select('scrw', screenPositionWidth);
  select('aout', analogOutputCount);
  select('disp', characterDisplayWidthHeight);
  select('jvsd', JvsDashSupport);
  select('jvss', JvsSignalAdjust);

  // Analog
  let offset = 3;
  for (let p of [1, 2]) {
    for (let i of [1, 2, 3, 4, 5, 6]) {
      const map = data[offset++];
      const type = (map >> 4) & 7;
      const index = map & 7;
      select('a' + p + i + 't', type);
      select('a' + p + i + 'i', index);
    }
  }

  // Digital
  const buttonMap = [
    'u', 'd', 'l', 'r', '1', '2', '3', '4',
    '5', '6', '7', '8', '9', 'a', 'b', 'c'];
  for (let p of [1, 2]) {
    for (let i = 0; i < 16; ++i) {
      for (let targetP of [1, 2]) {
        const d1 = data[offset + 0];
        const d2 = data[offset + 1];
        offset += 2;
        check('p' + p + buttonMap[i] + '_p' + targetP + 's', d1 & 0x80);
        check('p' + p + buttonMap[i] + '_p' + targetP + 'u', d1 & 0x20);
        check('p' + p + buttonMap[i] + '_p' + targetP + 'd', d1 & 0x10);
        check('p' + p + buttonMap[i] + '_p' + targetP + 'l', d1 & 0x08);
        check('p' + p + buttonMap[i] + '_p' + targetP + 'r', d1 & 0x04);
        check('p' + p + buttonMap[i] + '_p' + targetP + '1', d1 & 0x02);
        check('p' + p + buttonMap[i] + '_p' + targetP + '2', d1 & 0x01);
        check('p' + p + buttonMap[i] + '_p' + targetP + '3', d2 & 0x80);
        check('p' + p + buttonMap[i] + '_p' + targetP + '4', d2 & 0x40);
        check('p' + p + buttonMap[i] + '_p' + targetP + '5', d2 & 0x20);
        check('p' + p + buttonMap[i] + '_p' + targetP + '6', d2 & 0x10);
        check('p' + p + buttonMap[i] + '_p' + targetP + '7', d2 & 0x08);
        check('p' + p + buttonMap[i] + '_p' + targetP + '8', d2 & 0x04);
      }
    }
  }

  // Rapid Fire
  for (let p of [1, 2]) {
    for (let i = 0; i < 6; ++i) {
      const d1 = (data[offset] >> 4) & 15;
      const d2 = data[offset++] & 15;
      select('p' + p + buttonMap[4 + i * 2] + '_rp', d1);
      select('p' + p + buttonMap[5 + i * 2] + '_rp', d2);
    }
  }
  for (let i = 1; i < 8; ++i) {
    const pattern = data[offset++];
    const width = data[offset++];
    applySequenceWidth(i - 1, width + 1);
    for (let bit = 0; bit < 8; ++bit) {
      check('p' + i + (bit + 1), pattern & (1 << bit));
    }
  }
}

const presets = [];

function appendPreset(name, data) {
  console.assert(data.length == 169);
  const select = document.getElementById('preset');
  const option = document.createElement('option');
  option.innerText = name;
  select.appendChild(option);
  presets.push(data);
}

function applyPreset(index) {
  applyData(presets[index]);
}

// Setup
appendPreset('SEGA Basic', [
  // Core : JVS Dash support
  0x00, 0x00, 0x04,
  // Analog Map: Left X-Axis => Lever X, Left Y-Axis => Lever Y
  0x10, 0x11, 0x00, 0x00, 0x00, 0x00, 0x12, 0x13, 0x00, 0x00, 0x00, 0x00,
  // Digital Map
  0x20, 0x00, 0x00, 0x00,  // 1P Up
  0x10, 0x00, 0x00, 0x00,  // 1P Down
  0x08, 0x00, 0x00, 0x00,  // 1P Left
  0x04, 0x00, 0x00, 0x00,  // 1P Right
  0x02, 0x00, 0x00, 0x00,  // 1P B1
  0x01, 0x00, 0x00, 0x00,  // 1P B2
  0x00, 0x80, 0x00, 0x00,  // 1P B3
  0x00, 0x40, 0x00, 0x00,  // 1P B4
  0x00, 0x20, 0x00, 0x00,  // 1P B5
  0x00, 0x10, 0x00, 0x00,  // 1P B6
  0x00, 0x08, 0x00, 0x00,  // 1P B7
  0x00, 0x04, 0x00, 0x00,  // 1P B8
  0x00, 0x00, 0x00, 0x00,  // 1P SHARE
  0x80, 0x00, 0x00, 0x00,  // 1P OPTION
  0x00, 0x00, 0x00, 0x00,  // 1P L3
  0x00, 0x00, 0x00, 0x00,  // 1P R3
  0x00, 0x00, 0x20, 0x00,  // 2P Up
  0x00, 0x00, 0x10, 0x00,  // 2P Down
  0x00, 0x00, 0x08, 0x00,  // 2P Left
  0x00, 0x00, 0x04, 0x00,  // 2P Right
  0x00, 0x00, 0x02, 0x00,  // 2P B1
  0x00, 0x00, 0x01, 0x00,  // 2P B2
  0x00, 0x00, 0x00, 0x80,  // 2P B3
  0x00, 0x00, 0x00, 0x40,  // 2P B4
  0x00, 0x00, 0x00, 0x20,  // 2P B5
  0x00, 0x00, 0x00, 0x10,  // 2P B6
  0x00, 0x00, 0x00, 0x08,  // 2P B7
  0x00, 0x00, 0x00, 0x04,  // 2P B8
  0x00, 0x00, 0x00, 0x00,  // 2P SHARE
  0x00, 0x00, 0x80, 0x00,  // 2P OPTION
  0x00, 0x00, 0x00, 0x00,  // 2P L3
  0x00, 0x00, 0x00, 0x00,  // 2P R3
  // Rapid Fire
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0x07,
  0x55, 0x07,
  0x33, 0x07,
  0x03, 0x04,
  0x07, 0x05,
  0xaa, 0x07,
  0xcc, 0x07,
]);

// Hooks
document.getElementById('preset').addEventListener('change', e => {
  if (e.target.options.selectedIndex > 0) {
    applyPreset(e.target.options.selectedIndex - 1);
  }
});

for (let i = 1; i < 8; ++i) {
  document.getElementById('rm' + i).addEventListener('change', e => {
    applySequenceWidth(i - 1, e.target.options.selectedIndex + 1);
  });
}