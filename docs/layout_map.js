function createTr(...args) {
  const tr = document.createElement('tr');
  for (const arg of args) {
    const td = document.createElement('td');
    if (typeof arg === 'string') {
      td.appendChild(document.createTextNode(arg));
    } else {
      td.appendChild(arg);
    }
    tr.appendChild(td);
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

const analog_map = document.getElementById('analog_map');
const analog_tbody = document.createElement('tbody');
analog_tbody.appendChild(createTr(
  uiMessages.layoutAnalogInput,
  uiMessages.layoutAnalogOutputType,
  uiMessages.layoutAnalogOutputNumber,
  uiMessages.layoutAnalogInvert,
));
const players = ['1', '2'];
const ids = ['1 (LX)', '2 (LY)', '3 (RX)', '4 (RY)', '5 (LT)', '6 (RT)'];
for (let player = 0; player < players.length; ++player) {
  for (let id = 0; id < ids.length; ++id) {
    analog_tbody.appendChild(createTr(
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
    ));
  }
}
analog_map.appendChild(analog_tbody);

const rapid_fire_map = document.getElementById('rapid_fire_map');
const rapid_fire_tbody = document.createElement('tbody');
rapid_fire_tbody.appendChild(createTr(
  uiMessages.layoutRapidFireSet,
  uiMessages.layoutRapidFireSequence,
  uiMessages.layoutRapidFireCycle,
  uiMessages.layoutRapidFireInvert,
));
for (let set = 0; set < 7; ++set) {
  rapid_fire_tbody.appendChild(createTr(
    uiMessages.layoutRapidFireSetPrefix + ' ' + (set + 1),
    createSequence('p' + (set + 1)),
    createSelect('rm' + (set + 1), [1, 2, 3, 4, 5, 6, 7, 8]),
    createCheckbox('inv' + (set + 1)),
  ));
}
rapid_fire_map.appendChild(rapid_fire_tbody);