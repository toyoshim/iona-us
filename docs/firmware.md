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
下記のメニューから書き込みたいファームウェアのバージョンを選びます。
現在はVer 1.00のみ選択可能です。
選択後に書き込みボタンを押すと、以下のようなポップアップが現れます。

<script src="https://toyoshim.github.io/CH559Flasher.js/CH559Flasher.js"></script>
<script>
async function flash() {
  const firmwares = [
    'firmwares/us_v1_00.bin',  // Ver 1.00
  ];
  const flasher = new CH559Flasher();
  await flasher.connect();
  //await flasher.erase();
  //const bin = await (await fetch('firmwares/us_v1_00.bin')).arrayBuffer();
  //await flasher.write(bin, rate => console.log(rate));
  //await flasher.verify(bin, rate => console.log(rate));
  //console.log(flasher);
}
</script>

<select id="version">
<option selected>Ver 1.00</option>
</select>
<button onclick="flash();">書き込み</button>