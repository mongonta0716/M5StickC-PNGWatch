# M5StickC-PNGWatch
![image](https://github.com/mongonta0716/M5StickC-PNGwatch/blob/master/jpg/image.jpg)


 PNGファイルから作成したデータを背景にして利用できるアナログ時計です。

 Analog clock for M5StickC whose background can be changed with PNG file data

## ●環境(Environment)
- Arduino IDE 1.8.10
- M5StickC Library 0.1.0

## ●設定とコンパイル(Settings and Compile)
 PNGWatchC.inoのInitialSettingsを書き換えてコンパイルしてください。

 Set the PNGWatchC.ino InitialSettings area according to your environment and compile.
### 必須項目(Required Settings)
- WiFi SSID
- WiFi Password
- URL of NTP Server

## ●使い方(Usage)
 ButtonAを押すとWiFiに接続してNTPと時刻を同期します。

 Press the buttonA to connect to the configured WiFi and synchronize with NTP Server.

## ●画像の入れ替え方法(How to change image)
 160x80のPNGファイルを自分で作成し、コンパイルすると背景画像を入れ替えることができます。

1. clockC.pngを作成する。
2. clockC.pngをclockC.png.hに変換
3. ソースファイルを置き換える。
4. コンパイル

 If you create a 160x80 PNG file yourself and compile it, you can replace the background image.

1. Create clockC.png
2. Convert from clockC.png to clockC.png.h
3. Replace clockC.png.h
4. Compile

### 画像変換 `png2code.py`
[M5Stack_Test_tools](https://github.com/mongonta0716/M5Stack_Test_tools)のpng2code.pyを使って、変換してください。

Refer to [M5Stack_Test_tools](https://github.com/mongonta0716/M5Stack_Test_tools) and convert it using `png2code.py`.

# Licence
[MIT](https://github.com/mongonta0716/M5StickC-PNGwatch/blob/master/LICENSE)

# Author
[Takao Akaki](https://twitter.com/mongonta555)
