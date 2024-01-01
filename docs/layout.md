---
layout: default
title: レイアウト設定
permalink: /setting
---
# レイアウト設定
v2系ファームウェアに対応した設定ページを実験的に公開しています。
十分な動作確認はできていないため、不具合の報告や機能の要望は歓迎します。
また特定ゲーム向けの設定をご報告いただければ、プリセット追加させて頂きます。

---
## 接続準備
[ファーム更新ページ](https://toyoshim.github.io/iona-us/firmware)の説明に従いファームウェア更新モードにします。頻繁に利用する場合は更新ページに書かれているType-AからType-Aに繋ぐ特殊なケーブルを利用すると楽です。

準備ができたら以下の「デバイスを探す」ボタンを押し、一覧に表示されたWinChipHead製のデバイスを選択し、接続を押して下さい。

同じボタンが「変更を保存する」に変化するので再度ボタンを押すと設定が保存されます。

<button id="button"></button>
<pre id="status"></pre>

---
## 設定
#### 設定の選択・コピー
- 変更したい設定１〜６のうち１つを選び、編集した後に「変更を確定」ボタンを押して下さい。
- 確定していない変更は、設定番号を選び直した際に破棄されます。
- 他の設定やプリセットからコピーした場合にも確定は必要です。
- 全ての変更が確定したら「変更を保存する」ボタンでデバイスに保存されます。

| | | |
|-|-|-|
|<select id="select"><option>設定 1</option><option>設定 2</option><option>設定 3</option><option>設定 4</option><option>設定 5</option><option>設定 6</option></select><br><button id="store">変更を確定</button>|<button id="storeToFile">ファイルに保存</button><br><button id="loadFromFile">ファイルから読込</button>|他の設定からコピー<br><select id="copy"><option>-</option><option>設定 1</option><option>設定 2</option><option>設定 3</option><option>設定 4</option><option>設定 5</option><option>設定 6</option></select><br>プリセットからコピー<br><select id="preset"><option>-</option></select>

#### 基本設定
宣言するデバイス名やサポートする機能をカスタマイズします。
JVSデータ信号レベル補正はLOW信号の電圧が高めのnamco系基板でONにしないと認識されない事があります。それ以外ではOFFが推奨です。
特殊入力は不要な際は0を指定した方が、動作時の遅延や負荷は小さくなります。

| | | |
|-|-|-:|
||JVS ID|<select id="id"><option>SEGA ENTERPRISES,LTD.</option><option>namco ltd.;JYU-PCB</option><option>namco ltd.;NA-JV</option><option>namco ltd.;TSS-I/O</option></select>
||アナログ入力数|<select id="ainc"><option>0</option><option>2</option><option>4</option><option>6</option><option>8</option></select>
||アナログ入力幅|<select id="ainw"><option>自動</option><option>16-bits</option></select>
||ロータリー入力数|<select id="rotc"><option>0</option><option>2</option></select>
||画面座標入力数|<select id="scrc"><option>0</option><option>1</option><option>2</option></select>
||画面座標入力幅|<select id="scrw"><option>10-bits</option><option>16-bits</option></select>
||アナログ出力数（ダミー）|<select id="aout"><option>0</option><option>2</option></select>
||文字表示ディスプレイサイズ（ダミー）|<select id="disp"><option>なし</option><option>16 x 1</option></select>
||JVS Dash サポート|<select id="jvsd"><option>OFF</option><option>ON</option></select>
||JVSデータ信号レベル補正|<select id="jvss"><option>OFF</option><option>ON</option></select>

#### アナログ設定
1Pと2Pそれぞれ6種類のアナログ入力の役割を設定します。
レバー（デジタル）の0と1を選ぶことで、アナログスティックを上下、左右のレバーに割り当てられます。ここで割り当てたレバー操作も、後段のボタン配置により展開されます。
アナログはレースゲームのハンドルやペダル、ロータリーはDJコントローラやパドル、画面座標入力はガンコントローラなどがアサインされますが、実際の使われ方はゲームによって様々です。

<table id="analog_map"></table>

#### 連射設定
連射のパタンを7種類設定できます。次のセクションで各ボタンごとに連射を使わないか、ここで定義されたパタンのどれかをアサインするか選べまます。
シーケンスは左から右に１フレームごとに入力を通すかマスクするかをチェックボックスで指定します。
指定した周期でシーケンスがループします。
全てチェックすれば実質連射はオフになり、１つ飛ばしでチェックすれば30連射になります。
反転を有効にすると、最終的な入力状態が反転して出力されます。

<table id="rapid_fire_map"></table>

#### ボタン配置
P1、P2それぞれの上下左右入力とボタン入力に対し、押された際にゲーム側に押されたと通達するボタンを指定します。任意の組み合わせで指定でき、P1側の入力をP2側の入力として配置することもできます。
ボタンはそれぞれ連射の設定を持つことができます。

<table id="button_map"></table>

<script src="https://toyoshim.github.io/CH559Flasher.js/CH559Flasher.js"></script>
<script>
window.uiMessages = {
  abort: '中断しました',
  connected: '接続しました（ブートローダー： ', 
  connectedInformation: ' / 設定フォーマット: v',
  error: 'エラーが発生しました: ',
  errorOnRead: '設定読込中にエラーが発生しました: ',
  findDevice: 'デバイスを探す',
  idle: '待機中',
  modifiedOnStore: '未確定のデータが存在しますが、確定前のデータをファイルに保存しますか？',
  modifiedOnSave: '未確定のデータが存在しますが、確定前のデータを保存しますか？',
  noDevice: '例外が発生しました、デバイスが接続されているか確認して下さい',
  save: '変更を保存する',
  saved: '保存しました',
  unknownContinue: '接続しましたが、対応するバージョンのIONA-USの設定データが確認できません。このまま継続して最新データで上書きしますか？',
  unknownFileFormat: '未知のファイルフォーマットです',

  layoutAnalogInput: '入力',
  layoutAnalogOutputType: '出力タイプ',
  layoutAnalogOutputNumber: '出力番号',
  layoutAnalogInvert: '反転',
  layoutAnalogAnalog: 'アナログ',
  layoutAnalogOutputTypeNone: 'なし',
  layoutAnalogOutputTypeLever: 'レバー（デジタル）',
  layoutAnalogOutputTypeAnalog: 'アナログ入力',
  layoutAnalogOutputTypeRotary: 'ロータリー入力',
  layoutAnalogOutputTypeScreenPosition: '画面座標入力',

  layoutRapidFireSet: '連射設定',
  layoutRapidFireSequence: 'シーケンス',
  layoutRapidFireCycle: '周期',
  layoutRapidFireInvert: '反転',
  layoutRapidFireSetPrefix: 'パタン',

  layoutButtonInput: '入力',
  layoutButtonOutput: '出力',
  layoutButtonRapidFire: '連射',
  layoutButtonRapidFireNone: 'なし',
  layoutButtonRapidFirePrefix: 'パタン',
  layoutButtonRapidFireGearUp: 'ギアアップ',
  layoutButtonRapidFireGearDown: 'ギアダウン',
};
</script>
<script src="layout_map.js"></script>
<script src="layout.js"></script>
<script src="layout_presets.js"></script>

#### 配置の確認
現在PCに繋がっているコントローラについてIONAが認識するボタン配置を確認できます。
IONAのファームウェアを元にしたコードがブラウザ上で走りますが、デバイスへのアクセスは
WebUSBやWebHIDを経由するため、100%同じになる保証はできません。
以下のボタンからアクセスするAPIを選び、ダイアログが表示されたら確認したいデバイスの名前を選びます。
通常のコントローラはHIDを、Xbox向け非HIDのコントローラはUSBを選択してください。

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