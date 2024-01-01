function createTr(header, args) {
  const tr = document.createElement('tr');
  for (const arg of args) {
    const holder = document.createElement(header ? 'th' : 'td');
    if (typeof arg === 'string') {
      holder.appendChild(document.createTextNode(arg));
    } else {
      holder.appendChild(arg);
    }
    tr.appendChild(holder);
  }
  return tr;
}

function createSelect(id, options) {
  const select = document.createElement('select');
  select.id = id;
  for (const option of options) {
    const opt = document.createElement('option');
    opt.appendChild(document.createTextNode(option));
    select.appendChild(opt);
  }
  return select;
}

function createCheckbox(id) {
  const checkbox = document.createElement('input');
  checkbox.type = 'checkbox';
  checkbox.id = id;
  return checkbox;
}

function createSequence(prefix) {
  const span = document.createElement('span');
  for (let i = 1; i <= 8; ++i) {
    const input = document.createElement('input');
    input.type = 'checkbox';
    input.id = prefix + i;
    span.appendChild(input);
  }
  return span;
}

function createPair(a, b) {
  const span = document.createElement('span');
  span.appendChild(document.createTextNode(a));
  span.appendChild(document.createElement('br'));
  span.appendChild(document.createTextNode(b));
  return span;
}

function createCheckboxPair(i, o) {
  const span = document.createElement('span');
  span.appendChild(createCheckbox(i + '_p1' + o));
  span.appendChild(document.createElement('br'));
  span.appendChild(createCheckbox(i + '_p2' + o));
  return span;
}

function createButtonLabel(label, testClass) {
  const span = document.createElement('span');
  span.className = 'test_' + testClass;
  span.appendChild(document.createTextNode(label));
  return span;
}

const analog_map = document.getElementById('analog_map');
const analog_thead = document.createElement('thead');
analog_thead.appendChild(createTr(true, [
  uiMessages.layoutAnalogInput,
  uiMessages.layoutAnalogOutputType,
  uiMessages.layoutAnalogOutputNumber,
  uiMessages.layoutAnalogInvert
]));
analog_map.appendChild(analog_thead);
const analog_tbody = document.createElement('tbody');
const players = ['1', '2'];
const ids = ['1 (LX)', '2 (LY)', '3 (RX)', '4 (RY)', '5 (LT)', '6 (RT)'];
for (let player = 0; player < players.length; ++player) {
  for (let id = 0; id < ids.length; ++id) {
    analog_tbody.appendChild(createTr(false, [
      'P' + players[player] + ' ' + uiMessages.layoutAnalogAnalog + ' ' + ids[id],
      createSelect('a' + (player + 1) + (id + 1) + 't', [
        uiMessages.layoutAnalogOutputTypeNone,
        uiMessages.layoutAnalogOutputTypeLever,
        uiMessages.layoutAnalogOutputTypeAnalog,
        uiMessages.layoutAnalogOutputTypeRotary,
        uiMessages.layoutAnalogOutputTypeScreenPosition
      ]),
      createSelect('a' + (player + 1) + (id + 1) + 'i', [0, 1, 2, 3, 4, 5, 6, 7]),
      createCheckbox('a' + (player + 1) + (id + 1) + 'r'),
    ]));
  }
}
analog_map.appendChild(analog_tbody);

const rapid_fire_map = document.getElementById('rapid_fire_map');
const rapid_fire_thead = document.createElement('thead');
rapid_fire_thead.appendChild(createTr(true, [
  uiMessages.layoutRapidFireSet,
  uiMessages.layoutRapidFireSequence,
  uiMessages.layoutRapidFireCycle,
  uiMessages.layoutRapidFireInvert,
]));
rapid_fire_map.appendChild(rapid_fire_thead);
const rapid_fire_tbody = document.createElement('tbody');
for (let set = 0; set < 7; ++set) {
  rapid_fire_tbody.appendChild(createTr(false, [
    uiMessages.layoutRapidFireSetPrefix + ' ' + (set + 1),
    createSequence('p' + (set + 1)),
    createSelect('rm' + (set + 1), [1, 2, 3, 4, 5, 6, 7, 8]),
    createCheckbox('inv' + (set + 1)),
  ]));
}
rapid_fire_map.appendChild(rapid_fire_tbody);

const button_map = document.getElementById('button_map');
const buttons = ['↑', '↓', '←', '→', '□', '✕', '○', '△', 'L1', 'R1', 'L2', 'R2', 'Share', 'Option', 'L3', 'R3'];
const classes = ['up', 'down', 'left', 'right', 'b1', 'b2', 'b3', 'b4', 'b5', 'b6', 'b7', 'b8', 'b9', 'b10', 'b11', 'b12'];
const buttonIds = ['u', 'd', 'l', 'r', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c'];
for (let player = 0; player < 2; ++player) {
  const button_thead = document.createElement('thead');
  const inout = createPair(
    ' \\' + uiMessages.layoutButtonOutput,
    uiMessages.layoutButtonInput);
  button_thead.appendChild(createTr(true, [
    inout,
    '',
    'Start', 'Coin', '↑', '↓', '←', '→', '1', '2', '3', '4', '5', '6', '7', '8', '9', '10',
    uiMessages.layoutButtonRapidFire,
  ]));
  button_map.appendChild(button_thead);
  const button_tbody = document.createElement('tbody');
  for (let button = 0; button < 16; ++button) {
    const prefix = 'p' + players[player] + buttonIds[button];
    button_tbody.appendChild(createTr(false, [
      createButtonLabel('P' + players[player] + ' ' + buttons[button], classes[button]),
      createPair('P1', 'P2'),
      createCheckboxPair(prefix, 's'),
      createCheckboxPair(prefix, 'c'),
      createCheckboxPair(prefix, 'u'),
      createCheckboxPair(prefix, 'd'),
      createCheckboxPair(prefix, 'l'),
      createCheckboxPair(prefix, 'r'),
      createCheckboxPair(prefix, '1'),
      createCheckboxPair(prefix, '2'),
      createCheckboxPair(prefix, '3'),
      createCheckboxPair(prefix, '4'),
      createCheckboxPair(prefix, '5'),
      createCheckboxPair(prefix, '6'),
      createCheckboxPair(prefix, '7'),
      createCheckboxPair(prefix, '8'),
      createCheckboxPair(prefix, '9'),
      createCheckboxPair(prefix, 'a'),
      (button < 4) ? '' : createSelect(prefix + '_rp', [
        uiMessages.layoutButtonRapidFireNone,
        uiMessages.layoutButtonRapidFirePrefix + '1',
        uiMessages.layoutButtonRapidFirePrefix + '2',
        uiMessages.layoutButtonRapidFirePrefix + '3',
        uiMessages.layoutButtonRapidFirePrefix + '4',
        uiMessages.layoutButtonRapidFirePrefix + '5',
        uiMessages.layoutButtonRapidFirePrefix + '6',
        uiMessages.layoutButtonRapidFirePrefix + '7',
        uiMessages.layoutButtonRapidFireGearUp,
        uiMessages.layoutButtonRapidFireGearDown
      ]),
    ]));
  }
  button_map.appendChild(button_tbody);
}