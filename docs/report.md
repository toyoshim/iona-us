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
    const data = [];
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
        for (let usage of item.usages) {
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
    }
  }
  info.push('END');
  document.getElementById('info').innerText = info.join('\n');
}
</script>