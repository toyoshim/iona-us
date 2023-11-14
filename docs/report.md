---
layout: default
title: 報告
permalink: /report
---
# 報告
---

## 未対応のコントローラについて報告
このページから未対応のコントローラについて、デバイス情報を取得して要望として報告できます。
WebHID APIを用いてコントローラの情報を取得します。
ChromeまたはWebHIDの有効になったChromium系のブラウザにてご利用ください。
情報は製品固有の物であり、環境や個人の情報は含みません。
場合によってはここから取得できる情報だけでは不十分で、追加の作業をお願いする場合もあります。
また、未対応のコントローラ対応やファームウェアの更新は保証外の作業であり、
状況によっては時間を頂いたり、対応を諦める事もある事をご理解願います。

## 既存の報告
[こちらのページ](https://github.com/toyoshim/iona-us/issues)に未解決の報告をまとめてあります。
既に報告が上がっている場合は対応をお待ち下さい。
コメント欄に一言頂けると励みになったり修正時に通知が飛んだりします。

## 手順
該当するコントローラをコンピュータに接続し、デバイスの名前を確認してください。
下にある「コントローラと接続する」ボタンを押すと、下記のようなダイアログが現れます。
ここに該当デバイスの名前が含まれているはずですので、選択して「接続」ボタンを押して下さい。

![プロンプト](prompt.png)

正しく接続されると、下にある「コントローラ情報」にデバイス情報が表示されます。
ここに表示された内容を全てコピーして、作者の[Twitterアカウント](https://twitter.com/toyoshim)まで報告して下さい。
DMは解放してありますので、どなたでもDMが可能です。

---
<button onclick="connect();">コントローラと接続する</button>

---
## コントローラ情報
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

## さらに詳細な報告
Chrome越しで取得できる情報は限られているため、場合によっては充分なデバイス情報が得られません。
上記の方法でうまくいかず、さらなる調査に協力に頂ける場合、以下のIONA-USを使ったログ収集にご協力ください。

### ステップ1: 調査用ファームウェアの書き込み
ログ収集用の特殊なファームウェアを用います。そのため、以下の点にご注意ください。
- IONA-US上の設定データは全て削除され、自動で復元される事はありません。
- ログ収集後に再び[ファーム更新](firmware)ページからIONA-USのファームウェアを書き戻す必要があります。
- ログには、IONA-USとゲームパッド上でやり取りされたUSBプロトコル上の生パケットデータが含まれ、協力時にはこれらのデータを提出して頂く必要があります。
- ご提供頂いたデータをもとに、IONAのファームウェアを修正することにご同意ください。
- 同データを用いたテストケースが追加される事もあり、その場合はデータがGitHub上に公開されることにご同意ください。

[ファーム更新](firmware)ページの情報を参考にしてIONA-USをPCと接続してください。
P2側のUSBポートには何も接続しない状態で操作をしてください。

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
    error.innerText = '接続に失敗しました: ' + flasher.error;
    return;
  }
  await flasher.erase();
  const response = await fetch('firmwares/logger.bin');
  if (response.ok) {
    const bin = await response.arrayBuffer();
    await flasher.write(bin, rate => progressWrite.value = rate);
    await flasher.verify(bin, rate => progressVerify.value = rate);
    error.innerText = flasher.error ? flasher.error : '成功';
  } else {
    error.innerText = 'ファームウェアが見つかりません';
  }
  await flasher.boot();
}
</script>
<button onclick="flash();">上記の条件に同意して書き込み</button>

| | |
|-|-|
|書き込み|0% <progress id="flash_progress_write" max=1 value=0></progress> 100%|
|検証|0% <progress id="flash_progress_verify" max=1 value=0></progress> 100%|

結果
<pre id="flash_error"></pre>

### ステップ2: 目的のデバイスの情報取得
ここまで正常に進んだ場合、LEDが点滅しているはずです。
LEDの点滅が確認できたら、目的のデバイスをP2側のUSBポートに挿してください。
ログのバッファが埋め尽くされるとLEDが点灯したままになりますが、点滅したままでも充分なログが取得できている可能性が高いので心配不要です。

ログの取得は一瞬ですので、すぐに次の作業に移れます。
ここでSERVICEボタンを押してください。
LEDが消灯すれば成功です。
ログデータを保存し、PCとの通信ができる状態になったので、次のステップに進み、ログを吸い出します。

### ステップ3: ログを読み出す
下のボタンを押すと再びデバイス選択ダイアログが出るので、最初のステップと同じデバイスを選択してください。

<button onclick="read();">ログを読み出す</button>

<style>
.type {
  background-color: #466;
  padding: 4pt 8pt;
  border-radius: 8pt;
  margin: 32pt;
}
.type_error {
  background-color: #c66;
  color: #222;
  padding: 4pt 8pt;
  border-radius: 8pt;
  margin: 32pt;
}
.ep {
  display: inline;
  background-color: #686;
  padding: 1pt 4pt;
  border-radius: 3pt;
  margin-top: 10pt;
  margin-left: 50pt;
  border: 4pt;
  font-family: monospace;
}
.pid {
  display: inline;
  background-color: #668;
  padding: 1pt 4pt;
  border-radius: 3pt;
  margin-top: 10pt;
  margin-left: 5pt;
  border: 4pt;
  font-family: monospace;
}
.size {
  display: inline;
  background-color: #868;
  padding: 1pt 4pt;
  border-radius: 3pt;
  margin-top: 10pt;
  margin-left: 5pt;
  border: 4pt;
  font-family: monospace;
}
.data {
  background-color: #866;
  padding: 1pt 4pt;
  border-radius: 3pt;
  margin: 5pt 50pt;
  border: 4pt;
  font-family: monospace;
}
</style>

<script>
async function read() {
  const progressRead = document.getElementById('progress_read');
  const error = document.getElementById('read_error');
  progressRead.value = 0;
  error.innerText = '';

  const flasher = new CH559Flasher();
  await flasher.connect();
  if (flasher.error) {
    error.innerText = '接続に失敗しました: ' + flasher.error;
    return;
  }
  const log = new Uint8Array(1024);
  for (let i = 0; i < 1024; i += 32) {
    let buffer = await flasher.readDataInRange(0xf000 + i, 32);
    if (!buffer) {
      setStatus('読み出しに失敗しました: ' + flasher.error);
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
|読み出し|0% <progress id="progress_read" max=1 value=0></progress> 100%|

結果
<pre id="read_error"></pre>

<button onclick="analyze();">ログを解析する</button>
<div id="dump"></div>

<script>
function createDiv(text, className) {
  const div = document.createElement('div');
  if (text) {
    div.innerText = text;
  }
  if (className) {
    div.className = className;
  }
  return div;
}

function dumpLog(text) {
  const dump = document.getElementById('dump');
  if (!dump.firstChild) {
    dump.appendChild(createDiv());
  }
  dump.firstChild.innerText = text;
}

function dumpObject(object) {
  const dump = document.getElementById('dump');
  if (!dump.firstChild) {
    dump.appendChild(createDiv());
  }
  dump.firstChild.appendChild(object);
}

function octetToHex(octet) {
  const hex = '0' + octet.toString(16);
  return hex.substring(hex.length - 2);
}

function epToString(ep) {
  return 'EP:0x' + octetToHex(ep);
}

function pidToString(pid) {
  switch (pid) {
    case 0x01:
      return 'PID:OUT';
    case 0x02:
      return 'PID:ACK';
    case 0x03:
      return 'PID:DATA0';
    case 0x09:
      return 'PID:IN';
    case 0x0a:
      return 'PID:NAK';
    case 0x0b:
      return 'PID:DATA1';
    case 0x0d:
      return 'PID:SETUP';
    case 0x0e:
      return 'PID:STALL';
    default:
      return 'PID:0x' + octetToHex(pid);
  }
}

function sizeToString(size) {
  return 'SIZE:0x' + octetToHex(size);
}

function dataToString(data) {
  let string = '';
  for (let i in data) {
    if (i != 0) {
      string += ', ';
    }
    string += '0x' + octetToHex(data[i]);
  }
  return string;
}

function decodeSetup(data) {
  const setup = {
    bmRequestType: data[0],
    bRequest: data[1],
    wValue: data[2] | (data[3] << 8),
    wIndex: data[4] | (data[5] << 8),
    wLength: data[6] | (data[7] << 8),
  };
  setup.direction = (data[0] & 0x80) ? 'Device to Host' : 'Host to Device';
  const type = [
    'Standard', 'Class', 'Vendor', 'Reserved',
  ];
  setup.type = type[(data[0] & 0x60) >> 5];
  const receipt = [
    'Device', 'Interface', 'Endpoint', 'Other'
  ];
  const receiptId = data[0] & 0x1f;
  setup.receipt = (receiptId < 4) ? receipt[receiptId] : 'Reserved';
  const request = [
    'Get Status', 'Clear Feature', '0x02 unknown', 'Set Feature',
    '0x04 unknown', 'Set Address', 'Get Descriptor', 'Set Descriptor',
    'Get Configuration', 'Set Configuration', 'Get Interface', 'Set Interface',
    'Sync Frame',
  ];
  if (setup.receipt == 'Device') {
    setup.request =
       (setup.bRequest < 0x0d) ? request[setup.bRequest] : 'Unknown';
  }
  if (setup.request == 'Get Descriptor') {
    switch (data[3]) {
      case 0x01:
        setup.descriptor = 'Device Descriptor';
        break;
      case 0x02:
        setup.descriptor = 'Configuration Descriptor';
        break;
      case 0x03:
        setup.descriptor = 'String Descriptor';
        break;
      case 0x04:
        setup.descriptor = 'Interface Descriptor';
        break;
      case 0x05:
        setup.descriptor = 'Endpoint Descriptor';
        break;
      case 0x06:
        setup.descriptor = 'Device Qualifier Descriptor';
        break;
      case 0x0b:
        setup.descriptor = 'Interface Association Descriptor';
        break;
      case 0x21:
        setup.descriptor = 'HID Descriptor';
        break;
      case 0x22:
        setup.descriptor = 'HID Report Descriptor';
        break;
      case 0x23:
        setup.descriptor = 'HUB Descriptor';
        break;
      default:
        setup.descriptor = 'Unknown Descriptor';
        break;
    }
  }
  return setup;
}

function dumpSend(ep, pid, data) {
  const root = createDiv('HOST to DEVICE', 'type');
  root.appendChild(document.createElement('br'));
  root.appendChild(createDiv(epToString(ep), 'ep'));
  root.appendChild(createDiv(pidToString(pid), 'pid'));
  root.appendChild(createDiv(sizeToString(data.length), 'size'));
  if (data.length) {
    root.appendChild(createDiv(dataToString(data), 'data'));
    if (pid == 0x0d) {
      console.log(decodeSetup(data));
    }
  }
  dumpObject(root);
}

function dumpRecv(ep, pid, data) {
  const root = createDiv('DEVICE to HOST', 'type');
  root.appendChild(document.createElement('br'));
  root.appendChild(createDiv(epToString(ep), 'ep'));
  root.appendChild(createDiv(pidToString(pid), 'pid'));
  root.appendChild(createDiv(sizeToString(data.length), 'size'));
  if (data.length) {
    root.appendChild(createDiv(dataToString(data), 'data'));
  }
  dumpObject(root);
}

function dumpStall() {
  dumpObject(createDiv('STALL', 'type_error'));
}

function dumpNak() {
  dumpObject(createDiv('NAK', 'type_error'));
}

async function analyze() {
  const handle = (await window.showOpenFilePicker({
    types: [
      {
        description: 'IONA USB Protocol Dump File',
      }
    ]
  }))[0];
  const dump = document.getElementById('dump');
  if (dump.firstChild) {
    dump.removeChild(dump.firstChild);
  }
  const file = await handle.getFile();
  const data = new Uint8Array(await file.arrayBuffer());
  if (data[0] != 'I'.charCodeAt(0) ||
      data[1] != 'O'.charCodeAt(0) ||
      data[2] != 'N'.charCodeAt(0) ||
      data[3] != 'L'.charCodeAt(0)) {
    dumpLog('IONAのログファイルではありません。');
    return;
  }
  for (let i = 4; i < data.length; ++i) {
    switch (data[i]) {
      case 1:
        dumpSend(data[i + 1], data[i + 2],
            data.subarray(i + 4, i + 4 + data[i + 3]));
        i += data[i + 3] + 3;
        break;
      case 2:
        dumpRecv(data[i + 1], data[i + 2],
            data.subarray(i + 4, i + 4 + data[i + 3]));
        i += data[i + 3] + 3;
        break;
      case 3:
        dumpStall();
        break;
      case 4:
        dumpNak();
        break;
      default:
        return;
    }
  }
}
</script>