---
layout: default
title: ファームウェア更新
permalink: /firmware
---
# ファームウェア更新
---
## 前置き
ファームウェア更新は、WebUSB APIを用いて、このページ上から行います。
ChromeまたはWebUSBの有効になったChromium系のブラウザにてご利用下さい。

## 準備
下の写真で白丸で囲まれた部分にスルーホールが4つあります。
まずは電源を入れていない状態で、黄色で結んだ対と赤で結んだ対（距離が近い対）を短絡します。
これによりJVS端子にUSB用のデータ線（D+/D-）を接続し、コンピュータとのUSB接続を可能にします。

![図 v1.xx](fw_fig.jpg)
![図 v2.xx](fw_fig_v2.jpg)

ここでは簡単な方法をいくつか紹介します。

まずは1枚目の写真も針金のような物をアーチ状に曲げて穴に刺す方法。
ホチキスの芯などでも作成可能ですが細すぎると接触が悪くなるかもしれません。

2枚目のピンセットを使った方法も、大胆に見えますが、作業も簡単でしっかり接続されるのでオススメです。
ただ、ピンセットを複数持っている人は少ないかも。

一番の正攻法は3枚目のピンヘッダを取り付けてジャンパーピンで繋ぐ方法です。
ただしハンダ付けが必要なので万人向けではありません。

![針金](fw_wire.jpg)
![ピンセット](fw_pinset.jpg)
![ジャンパーピン](fw_jump.jpg)

## コンピュータとの接続
まずは基板上のSERVICEボタン（中央付近のボタン）を押しながらmicro USBから給電開始します。
SERVICEボタンが正しく認識されると、LEDランプが消灯したままのファームウェア更新モードになります。
点滅してしまった場合は通常モードで起動しているため、もう一度電源の接続からやり直します。

これとは別にUSB Type AからType Aに繋ぐ特殊なケーブルを1P用端子に接続する方法もあります。
この場合は端子のショートや電源接続は不要です。SERVICEを押しながらPCに接続するだけで更新可能です。

うまくファームウェア更新モードで起動したら、JVS端子を使ってコンピュータとUSBで接続します。
この際、USB端子にはゲームコントローラ等を挿さないで下さい。特に1P側の端子はJVS端子の信号と衝突します。

## WinUSBの設定（Windowsで初回のみ）
デバイスを初めて接続した場合、デバイスマネージャーに不明なデバイスとして表示されます。
複数表示されている場合は、プロパティの詳細に`USB\VID_4348&PID_55E0\...`と表示されているデバイスが対象とするデバイスです。
このデバイスに対し、システム提供のWinUSBと呼ばれる標準ドライバを割り当てる必要があります。

設定方法の詳細についてはMicrosoft公式のドキュメント
「[システム提供のデバイス クラスを指定して WinUSB をインストールする](https://docs.microsoft.com/ja-jp/windows-hardware/drivers/usbcon/winusb-installation#installing-winusb-by-specifying-the-system-provided-device-class)」
に書かれていますので、この手順に従って設定して下さい。
Windows 11でのインストールの様子をキャプチャーした動画があるので、[こちら](https://www.youtube.com/watch?v=5yzpc2vI_94)も参考にしてください。
公式ページの説明が分かりにくいと感じる場合、役に立つかもしれません。10でも基本的には同じです。

インストール時にエラーが発生する場合、何回か試すと成功するとの報告があります。
またUSB3.0ポートに接続した場合に特に不安定との報告もありますので、USB2.0等のポートに接続するか、USB2.0のUSB Hubなどを通して接続してみてください。
この不具合に関してはチップ内蔵のブートローダ、あるいはWindows側の問題のため、改善策は今の所ありません。

![デバイスマネージャー](fw_devman.png)

## ファームウェアの選択
ページ一番下のメニューから書き込みたいファームウェアのバージョンを選びます。
選択後に書き込みボタンを押すと、以下のようなプロンプトが現れます。

USBのベンダーIDと製品IDで絞って選択画面を出していますので、基本ここには1つの選択肢しか現れません。
複数出た場合は、同じチップを搭載したデバイスがファームウェア更新モードで接続されていないか確認して下さい。
この状況はまず無いかとは思いますが……。
一方で、何も表示されない場合には以下の点を確認してみて下さい。

- 準備で行ったスルーホールの短絡がうまくいっていない、または接続が不安定
- micro USBから電源が供給されていない
- 電源は供給されているが、SERVICEボタンを押しながらファームウェア更新モードにしていない

出荷時のファームウェア書き込みも同等の方法を使っているため、初期不良の可能性はありません。
かならず上記のいずれかの問題が起きているはずですので、ゆっくりと確認してみましょう。

プロンプトが正しく表示されていたら、表示されているデバイスを選択し「接続」ボタンを押します。
書き込みが始まり、プログレスバーが更新されます。
書き込み中のトラブルで更新が失敗しても壊れませんので安心して下さい。
継続して再書き込みを行えば問題ありません。
もし、繰返しエラーが発生する場合には連絡を頂けたら調査します。

![プロンプト](fw_prompt.png)

## 動作確認
ファームウェア更新が正常終了したら、コンピュータから切断し、micro USBからの電源を供給し直して下さい。
通常モードで起動した際にLEDが正しく点滅するようなら、更新は成功です。
JVS I/Oとして利用する前、特にジャンパーピンを実装した人は、忘れずにスルーホールの短絡を解除して下さい。
うっかり短時間の間ならJVSとして認識されない程度で済むと想います。
気づかずに長時間通電した場合、USB用の信号とJVS用の信号が衝突し、システムボードやIONA-USの故障に繋がります。

## ファームウェア更新履歴
- Ver 1.00 展示用サンプルに搭載されたバージョンでコインが減らないバグがあります
- Ver 1.01 初期製品出荷版
- Ver 1.02 USBホストの動作で仕様違反があったので修正し、対応コントローラを追加しました
- Ver 1.02a 他のコントローラ利用後にXbox系のコントローラを接続した際の動作不良を修正し、対応コントローラを追加しました
- Ver 1.03 複合デバイスとREMOTE WAKEUPの対応を改善し、対応コントローラを追加しました
- Ver 1.04 対応コントローラを追加しました
- Ver 1.10 ツインスティックモードを追加しました
- Ver 1.20 NAOMI麻雀モードを追加しました
- Ver 1.21 JVS電気特性の向上
- Ver 1.22 レイアウト設定モードに入るには0.5秒以上の同時押しが必要になりました
- Ver 1.23 コントローラ接続時に電源が安定するまで少し長く待つようにしました
- Ver 1.24 Ver 1.33の修正を取り込んだ安定版
- Ver 1.30 アナログ０−３に対し、P1アナログX/Y、P2アナログX/Yを割り当てました
- Ver 1.31 ガンコン3に対応しました
- Ver 1.32 ロータリーコントローラ2ch追加、P1アナログX/Yを割り当てました
- Ver 1.33 v1.20基板以降でJVSシステムとの相性問題が改善します
- Ver 1.34 アナログを4chに減らしGuilty GearシリーズでI/Oエラーが出ないよう修正、兎でD/H/L/ポンが反応しない問題に対応しました
- Ver 1.35 SERVICE+TEST長押し中も入力が効くように修正、設定モードには0.5〜5秒押し続けた時のみ移行
- Ver 1.40 アナログレイアウト、オプション設定を追加し、画面ポジション入力に対応しました
- Ver 1.41 namco NA-JV互換モードと各種オプション設定を追加しました
- Ver 1.42d メインIDコマンドや一部のnamco固有コマンドに正常応答を返すように変更しました
- Ver 1.43 TESTボタンを押しながら起動する事でJVSデータ信号レベル補正の有無を反転するよう変更しました
- Ver 1.44 Brook XB Fighting BoardのX-Oneモードに対応、ほかXbox系で再接続時に起きる認識問題を修正しました
- Ver 1.45 アナログレバー設定がマニュアルと反対になっていたのを修正、またアナログ入力を無効にするオプションを追加しました
- Ver 1.46 exA-ArcadiaのJVS Dash（高速モード）に対応、合わせてオプションの追加とLEDの調整をしました
- Ver 1.47 GP2040に対応

## ファームウェアの互換性
Xbox 360、Xbox Oneシリーズの規格に対応したコントローラは基本的に安定して動くはずです。
それ以外のUSB HIDデバイスについては、別のコントローラに対応する際に、うまく動作しなくなる可能性がゼロではありません。
以下に挙げたデバイスについては、デスクリプタの内容を把握し、実機ではないですが自動テストで互換性を確認するようにしています。
報告していただいたデバイスについても随時追加予定ですので、ここにないデバイスを継続的に使いたい場合は報告して頂けると幸いです。

またファームウェア1.40から保存する設定のフォーマットが変更されたため、異なるフォーマットを使用するファームウェアに更新した場合、設定は全て初期化されます。

|デバイス名称|確認バージョン|備考|
|-|-|-|
|(Xbox 360規格コントローラ)|1.00|1.02a以降推奨|
|(XInput規格コントローラ)|1.00|1.47以降推奨|
|GP2040 (*2)|1.47|Xbox 360互換モードで確認|
|(Xbox Oneシリーズ規格コントローラ)|1.00|1.02a以降推奨|
|Brook XB Fighting Board|1.44|標準のX-Oneモードで動作、X-360モード指定でも動作可能|
|Brook Universal Fighting Board|1.47|PS4モードで動作|
|(Bootモード対応キーボード)|1.20|NAOMI麻雀モードに対応|
|REAL ARCADE PRO V3.SA|1.47||
|ガンコン3|1.31||DeathCrimson OX対応|
|ホリパッドFPSプラス for PlayStation 4|1.00|PS3モードも対応|
|ワイヤレスコントローラー（DUALSHOCK 4 - CUH-ZCT1J）|1.02||
|ワイヤレスコントローラー（DUALSHOCK 4 - CUH-ZCT2J）|1.03||
|ホリパッドミニ for Nintendo Switch|1.00||
|Nintendo Switch Proコントローラー|1.04||
|Nintendo Switch Joy-Con 充電グリップ|1.04||
|Brook ZERO-PI Fighting Board|1.44|標準のSwitchモードで動作|
|CYBER・アーケードスティック|1.23||
|6B Controller (メガドライブミニ)|1.02a||
|Xin-Mo Controller (*1)|1.20||

(*1) パソケード フルHDテーブル筐体 PS3ドッキングモデル にて仕様されているコントローラ
(*2) Raspberry Pi Pico based open source firmware; https://gp2040.info/

---
## ファームウェア更新
以下は実際にファームウェア更新を行うためのUIです。書き込みボタンにより実際に更新されます。

Ver 2.0x系列は実験用のバージョンで十分なテストは実施されていません。また設定ページから必ず設定の初期化を行う必要があります。
最も新しいVer 1.4x系のバージョン（デフォルトで選択済み）が最新の安定バージョンになります。

<script src="https://toyoshim.github.io/CH559Flasher.js/CH559Flasher.js"></script>
<script>
async function flash() {
  const firmwares = [
    'firmwares/us_v1_04.bin',  // Ver 1.04
    'firmwares/us_v1_10.bin',  // Ver 1.10
    'firmwares/us_v1_24.bin',  // Ver 1.24
    'firmwares/us_v1_35.bin',  // Ver 1.35
    'firmwares/us_v1_40.bin',  // Ver 1.40
    'firmwares/us_v1_41.bin',  // Ver 1.41
    'firmwares/us_v1_42d.bin',  // Ver 1.42d
    'firmwares/us_v1_43.bin',  // Ver 1.43
    'firmwares/us_v1_44.bin',  // Ver 1.44
    'firmwares/us_v1_45.bin',  // Ver 1.45
    'firmwares/us_v1_46.bin',  // Ver 1.46
    'firmwares/us_v1_47.bin',  // Ver 1.47
    'firmwares/us_v2_00.bin',  // Ver 2.00
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
    error.innerText = flasher.error ? flasher.error : '成功';
  } else {
    error.innerText = 'ファームウェアが見つかりません';
  }
}
</script>

<select id="version">
<option>Ver 1.04</option>
<option>Ver 1.10</option>
<option>Ver 1.24</option>
<option>Ver 1.35</option>
<option>Ver 1.40</option>
<option>Ver 1.41</option>
<option>Ver 1.42d</option>
<option>Ver 1.43</option>
<option>Ver 1.44</option>
<option>Ver 1.45</option>
<option>Ver 1.46</option>
<option selected>Ver 1.47</option>
<option>Ver 2.00</option>
</select>
<button onclick="flash();">書き込み</button>

| | |
|-|-|
|書き込み|0% <progress id="progress_write" max=1 value=0></progress> 100%|
|検証|0% <progress id="progress_verify" max=1 value=0></progress> 100%|

結果
<pre id="error"></pre>
