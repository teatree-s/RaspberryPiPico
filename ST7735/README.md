<a name="readme-top"></a>

<!-- ABOUT THE PROJECT -->

# 1. プロジェクトについて

Raspberry Pi Pico で TFT 液晶ディスプレイの ST7735 へ表示するプロジェクトです。

<p align="right">(<a href="#readme-top">back to top</a>)</p>

# 2. 環境構築

Raspberry Pi の公式ドキュメントを参考に環境を作成してください。

1. [Pico-series Microcontrollers](https://www.raspberrypi.com/documentation/microcontrollers/pico-series.html)
1. [Getting started with Raspberry Pi Pico](https://projects.raspberrypi.org/en/projects/getting-started-with-the-pico)
1. [What is MicroPython?](https://www.raspberrypi.com/documentation/microcontrollers/micropython.html)

Pico に書き込む UF2 ファイルは 3 つ目のページにあるリンクをクリックしてダウンロードしてください。

Download the correct MicroPython UF2 file for your board:

- Pico
- Pico W
- Pico 2
- Pico 2 W

また Thonny のインストールなどは、このフォルダの上位にある HowToUse も参考にしてください。

<p align="right">(<a href="#readme-top">back to top</a>)</p>

# 3. Pin connections

| Pico | TFT |
| ---- | --- |
| VBUS | VCC |
| GND  | GND |
| GP2  | CS  |
| GP1  | RST |
| GP0  | A0  |
| GP7  | SDA |
| GP6  | SCK |
| 3V3  | LED |

<img src="./docs/ST7735.jpg" width="480">

<p align="right">(<a href="#readme-top">back to top</a>)</p>

# 4. プログラム

1. Thonny を開いて Pico と接続する
2. 「ST7735.py, main.py, sdcard.py」と bmp フォルダを Pico へコピーする
3. Thonny の実行ボタンを押す
4. bmp フォルダの「logo.bmp」が表示されます

<p align="right">(<a href="#readme-top">back to top</a>)</p>
