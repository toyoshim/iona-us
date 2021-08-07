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

![図](fw_fig.jpg)

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

うまくファームウェア更新モードで起動したら、JVS端子を使ってコンピュータとUSBで接続します。
ドライバや特殊なソフトウェアは不要です。

## ファームウェアの選択
ページ一番下のメニューから書き込みたいファームウェアのバージョンを選びます。
現在はVer 1.00のみ選択可能です。
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
- Ver 1.00 展示用サンプルに搭載されたバージョンでコインが減らないバグがある
- Ver 1.01 初期製品出荷版で現状の最新版です

## ファームウェアの互換性
Xbox 360、Xbox Oneシリーズの規格に対応したコントローラは基本的に安定して動くはずです。
それ以外のUSB HIDデバイスについては、別のコントローラに対応する際に、うまく動作しなくなる可能性がゼロではありません。
以下に挙げたデバイスについては、デスクリプタの内容を把握し、実機ではないですが自動テストで互換性を確認するようにしています。
報告していただいたデバイスについても随時追加予定ですので、ここにないデバイスを継続的に使いたい場合は報告して頂けると幸いです。

|デバイス名称|確認バージョン|備考|
|-|-|-|
|(Xbox 360規格コントローラ)|1.00||
|(Xbox Oneシリーズ規格コントローラ)|1.00||
|ホリパッドFPSプラス for PlayStation 4|1.00|PS3モードも対応|
|ホリパッドミニ for Nintendo Switch|1.00||

---
## ファームウェア更新
以下は実際にファームウェア更新を行うためのUIです。書き込みボタンにより実際に更新されます。

<script src="https://toyoshim.github.io/CH559Flasher.js/CH559Flasher.js"></script>
<script>
async function flash() {
  const firmwares = [
    'firmwares/us_v1_00.bin',  // Ver 1.00
    'firmwares/us_v1_01.bin',  // Ver 1.01
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
  const bin = await (await fetch(url)).arrayBuffer();
  await flasher.write(bin, rate => progressWrite.value = rate);
  await flasher.verify(bin, rate => progressVerify.value = rate);
  error.innerText = flasher.error ? flasher.error : '成功';
}
</script>

<select id="version">
<option>Ver 1.00</option>
<option selected>Ver 1.01</option>
</select>
<button onclick="flash();">書き込み</button>

| | |
|-|-|
|書き込み|0% <progress id="progress_write" max=1 value=0></progress> 100%|
|検証|0% <progress id="progress_verify" max=1 value=0></progress> 100%|

結果
<pre id="error"></pre>
