# GadgetSeed

---

![GadgetSeed](gadgetseed_logo.png)

GadgetSeedは組み込み機器向けのマルチタスクOSです。

## 特徴

* マルチタスクカーネル
* デバッグを支援するシェルコマンド
* 標準化されたデバイスドライバAPI
* [FatFS](http://elm-chan.org/fsw/ff/00index_e.html)ファイルシステムに対応
* [LwIP](https://savannah.nongnu.org/projects/lwip/) TCP/IPプロトコルスタックに対応
* グラフィックス描画、文字フォント描画

## サポートMCUアーキテクチャ

   GadgetSeed は以下のアーキテクチャで動作することができます。

* ARM Cortex-M7
* ARM Cortex-M4

## サポートハードウェア

   GadgetSeed は以下のハードウェアで動作することができます。

| ハードウェア                          | MCU           | アーキテクチャ |
|---------------------------------------|---------------|----------------|
| [32F769IDISCOVERY](#32F769IDISCOVERY) | STM32F769NIH6 | ARM Cortex-M7  |
| [32F746GDISCOVERY](#32F746GDISCOVERY) | STM32F746NGH6 | ARM Cortex-M7  |
| [NUCLEO-F411RE](#NUCLEO-F4x1RE)       | STM32F401RET6 | ARM Cortex-M4  |

<!--
| [NUCLEO-F401RE](#NUCLEO-F4x1RE)       | STM32F411RET6 | ARM Cortex-M4  |
-->

<a name="32F769IDISCOVERY"></a>
### 32F769IDISCOVERY

![32F769IDISCOVERY](http://www.st.com/content/ccc/fragment/product_related/rpn_information/board_photo/group0/5b/1e/e6/2e/d1/1b/45/44/32f769i-disco.jpg/files/stm32f769i-disco.jpg/_jcr_content/translations/en.stm32f769i-disco.jpg)

<http://www.st.com/en/evaluation-tools/32f769idiscovery.html>[English]  
<http://www.st.com/ja/evaluation-tools/32f769idiscovery.html>[Japanese]

<a name="32F746GDISCOVERY"></a>
### 32F746GDISCOVERY

![STM32F746G-Discovery](http://www.st.com/content/ccc/fragment/product_related/rpn_information/board_photo/group0/ea/c4/6d/73/c3/f5/46/e2/stm32f746g-disco/files/stm32f746g-disco.jpg/_jcr_content/translations/en.stm32f746g-disco.jpg)

<http://www.st.com/en/evaluation-tools/32f746gdiscovery.html>[English]  
<http://www.st.com/ja/evaluation-tools/32f746gdiscovery.html>[Japanese]

<a name="NUCLEO-F4x1RE"></a>
### NUCLEO-F411RE

![STM32F411RE](http://www.st.com/content/ccc/fragment/product_related/rpn_information/board_photo/68/fb/69/d3/eb/3d/47/5a/nucleo-F4.jpg/files/nucleo-F4.jpg/_jcr_content/translations/en.nucleo-F4.jpg)

NUCLEO-F411RE  
<http://www.st.com/en/evaluation-tools/nucleo-f411re.html>[English]  
<http://www.st.com/ja/evaluation-tools/nucleo-f411re.html>[Japanese]

<!--
NUCLEO-F401RE  
<http://www.st.com/en/evaluation-tools/nucleo-f401re.html>[English]  
<http://www.st.com/ja/evaluation-tools/nucleo-f401re.html>[Japanese]
-->

## サンプルアプリケーション

   サンプルアプリケーションについては、以下を参照してください。

   [apps/APPLICATIONS.jp.md](apps/APPLICATIONS.jp.md)

## 開発環境の構築

### 動作環境構成

   ![GadgetSeed](gadgetseed_devenv.png)

### Ubuntu 18.04の場合

1. STM32CubeMXのインストール

   <http://www.st.com/en/development-tools/stm32cubemx.html>[English]  
   <http://www.st.com/ja/development-tools/stm32cubemx.html>[Japanese]

   上記のサイトからSTM32CubeMXをダウンロードし、"en.stm32cubemx.zip"を解凍してください。
   解凍した"en.stm32cubemx.zip"から"SetupSTM32CubeMX-4.26.0.linux"を使用します。

   ```sh
   sudo apt install -y libc6-i386 default-jre openjfx  
   sudo ./SetupSTM32CubeMX-4.26.0.linux
   ```

1. STM32 HAL and LL Driversのインストール

   STM32CubeMXを起動しHALドライバ、LLドライバをインストールしてください。

   ```sh
   /usr/local/STMicroelectronics/STM32Cube/STM32CubeMX/STM32CubeMX &  
   ```

   MCU STM32F7用のシステムのために"STM32Cube MCU Package for STM32F7 Serias Version 1.11.0"をインストールしてください。  
   MCU STM32F4用のシステムのために"STM32Cube MCU Package for STM32F4 Serias Version 1.21.0"をインストールしてください。  

1. arm-gccのインストール

   <https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads>

   上記サイトから GNU Arm Embedded Toolchain("gcc-arm-none-eabi-7-2017-q4-major-linux.tar.bz2") をダウンロードしてください。

   インストール
   ```sh
   sudo tar xvfj gcc-arm-none-eabi-7-2017-q4-major-linux.tar.bz2 -C /opt
   ```

   実行パスの設定
   ```sh
   PATH=$PATH:/opt/gcc-arm-none-eabi-7-2017-q4-major/bin
   ```
   上記は .bash_aliases 等に追記することを推奨します。

1. make gcc openocd picocom等ツールのインストール

   ```sh
   sudo apt install -y git make gcc unzip openocd picocom otf2bdf p7zip-full
   ```

<!--
1. arm-gccのインストール
   <http://marksolters.com/programming/2016/06/22/arm-toolchain-16-04.html>

   ```sh
   sudo apt-get remove gcc-arm-none-eabi binutils
   sudo add-apt-repository ppa:team-gcc-arm-embedded/ppa
   sudo apt-get update
   sudo apt-get install gcc-arm-embedded
   ```
-->

<!--
1. STM32CubeProgrammer

   <http://www.st.com/en/development-tools/stm32cubeprog.html>[English]  
   <http://www.st.com/ja/development-tools/stm32cubeprog.html>[Japanese]

   必要に応じてインストールしてください。

   ```sh
   sudo apt-get -y install libusb-1.0.0-dev  
   sudo ./SetupSTM32CubeProgrammer-1.0.0.linux  
   sudo cp /usr/local/STMicroelectronics/STM32Cube/STM32CubeProgrammer/Drivers/rules/* /etc/udev/rules.d/
   ```
-->

## 実行ファイルのビルド

1. makeコマンドの実行

   ソースツリーを展開したディレクトリでmakeコマンドを実行してください。

   ```sh
   make
   ```

1. コンフィグレーションによるシステムの選択

   以下の内容が表示されたら、ビルドするシステムを選んでください。1から5の数値を入力してシステムを選択します。

   ```sh
   *** Select target system ***
     1 : 32F746GDISCOVERY               : STM 32F746GDISCOVERY                          : 32F746GDISCOVERY.conf
     2 : 32F769IDISCOVERY               : STM 32F769IDISCOVERY                          : 32F769IDISCOVERY.conf
     3 : NUCLEO-F411RE                  : STM NUCLEO-F411RE                             : NUCLEO-F411RE.conf
     4 : NUCLEO-F411RE_HVGA-LCD-HX8357D : STM NUCLEO-F411RE + MAR3520(HVGA LCD HX8357D) : NUCLEO-F411RE_HVGA-LCD-HX8357D.conf
     5 : NUCLEO-F411RE_QVGA-LCD-ILI9341 : STM NUCLEO-F411RE + K60(QVGA LCD ILI9341)     : NUCLEO-F411RE_QVGA-LCD-ILI9341.conf
     6 : emu                            : Emulator system with linux                    : emu.conf
   Input No. : 
   ```

1. コンフィグレーションによるアプリケーションの選択

   以下のような内容が表示されたら、ビルドするアプリケーションを選んでください。
   例はシステムとして"2"(32F769IDISCOVERY)を選択した場合です。
   数値を入力してアプリケーションを選択します。

   ```sh
   Select : 2
   Target system : 32F769IDISCOVERY (32F769IDISCOVERY.conf)
   *** Select target apprication ***
     1 : Clock application                        : clock.conf
     2 : File manager high resolution display     : filmanager_hr.conf
     3 : Graphics test                            : graphics_test.conf
     4 : Graphics test(many fonts)                : graphics_test_many_fonts.conf
     5 : LED brink                                : heartbeat.conf
     6 : Hello world                              : hello_world.conf
     7 : Music player high resolution display     : musicplay_hr.conf
     8 : Network sample                           : network.conf
     9 : Paint application                        : paint.conf
   Input No. : 
   ```

   例はアプリケーションとして"7"(Music player high resolution display)を選択した場合です。

   ```sh
   Select : 7
   Target apprication : Music player high resolution display (musicplay_hr.conf)
   awk -f tools/mksysconfig_mk.awk configs/systems/32F769IDISCOVERY.conf configs/musicplay_hr.conf > /home/shudo/develop/gadgetseed/targetconfig.mk
   cp /home/shudo/develop/gadgetseed/include/asm-Cortex-M7.h /home/shudo/develop/gadgetseed/include/asm.h
   awk -f tools/mksysconfig_h.awk configs/systems/32F769IDISCOVERY.conf configs/musicplay_hr.conf > /home/shudo/develop/gadgetseed/include/sysconfig.h
   arm-none-eabi-gcc -M -g -Wall -mthumb -mcpu=cortex-m7 -mtune=cortex-m7 -fipa-sra -mfpu=fpv5-d16 -mfloat-abi=hard -O2 -I/home/shudo/develop/gadgetseed/include main.c > .depend
   arm-none-eabi-gcc -g -Wall -mthumb -mcpu=cortex-m7 -mtune=cortex-m7 -fipa-sra -mfpu=fpv5-d16 -mfloat-abi=hard -O2 -I/home/shudo/develop/gadgetseed/include -c main.c
   make -C tools bmp2txt
   gcc -Wall -O2 -o bmp2txt bmp2txt.c
   tools/bmp2txt  gs_logo.bmp > gs_logo.txt
   make -C tools txt2bitmap
   gcc -Wall -O2 -o txt2bitmap txt2bitmap.c
    :
    :
   echo "const char os_version[] = \"0.9.5\";" > version.c
   echo "const char build_date[] = __DATE__;" >> version.c
   echo "const char build_time[] = __TIME__;" >> version.c
   arm-none-eabi-gcc -g -Wall -mthumb -mcpu=cortex-m7 -mtune=cortex-m7 -fipa-sra -mfpu=fpv5-d16 -mfloat-abi=hard -O2 -I/home/shudo/develop/gadgetseed/include -c version.c
   arm-none-eabi-gcc -g -Wall -mthumb -mcpu=cortex-m7 -mtune=cortex-m7 -fipa-sra -mfpu=fpv5-d16 -mfloat-abi=hard -O2 -I/home/shudo/develop/gadgetseed/include -Wl,-static -Wl,--gc-sections -nostartfiles -o gs-Cortex-M7-32F769IDISCOVERY-musicplay-0.9.5.elf -Tarch/Cortex-M7/systems/32F769IDISCOVERY.lds -Wl,-Map=gs-Cortex-M7-32F769IDISCOVERY-musicplay-0.9.5.map arch/Cortex-M7/start.o version.o \
   main.o gs_logo.o apps/soundplay/soundplay.a apps/musicplay/musicplay.a kernel/kernel.a arch/Cortex-M7/arch.a drivers/drivers.a libs/libs.a kernel/task/task.a extlibs/fatfs/libfatfs.a fs/fs.a uilib/uilib.a graphics/graphics.a font/font.a fontdata/fontdata.a shell/shell.a extlibs/libmad/libmad.a extlibs/faad2/libfaad2.a extlibs/picojpeg/libpicojpeg.a extlibs/libpng/libpng.a extlibs/zlib/libzlib.a arch/Cortex-M7/arch.a -lm kernel/kernel.a arch/Cortex-M7/arch.a drivers/drivers.a libs/libs.a kernel/task/task.a extlibs/fatfs/libfatfs.a fs/fs.a uilib/uilib.a graphics/graphics.a font/font.a fontdata/fontdata.a shell/shell.a extlibs/libmad/libmad.a extlibs/faad2/libfaad2.a extlibs/picojpeg/libpicojpeg.a extlibs/libpng/libpng.a extlibs/zlib/libzlib.a
   ln -f -s gs-Cortex-M7-32F769IDISCOVERY-musicplay-0.9.5.elf gadgetseed
   arm-none-eabi-objdump -h --section=.VECTORS --section=.text --section=.data \
   --section=.bss --section=.stack gs-Cortex-M7-32F769IDISCOVERY-musicplay-0.9.5.elf

   gs-Cortex-M7-32F769IDISCOVERY-musicplay-0.9.5.elf:     file format elf32-littlearm

   Sections:
   Idx Name          Size      VMA       LMA       File off  Algn
     0 .VECTORS      000001f8  08000000  08000000  00010000  2**2
                     CONTENTS, ALLOC, LOAD, READONLY, DATA
     1 .text         001b8ad5  08000200  08000200  00010200  2**6
                     CONTENTS, ALLOC, LOAD, READONLY, CODE
     3 .data         00003a54  20020000  081b8ce0  001d0000  2**3
                     CONTENTS, ALLOC, LOAD, DATA
     4 .bss          00032624  20023a58  081bc738  001d3a54  2**3
                     ALLOC
     5 .stack        00000000  20000000  20000000  001d3a58  2**3
                     CONTENTS
   ```

## ソフトウェアの書き込みと実行方法

   ビルドしたソフトウェアをハードウェアに書き込むためには openocd と GDB を使用します。
   デバッグコンソールを使用するにはシリアルターミナルを使用します。
   以下の例では、openocd と GDB とシリアルターミナル用にそれぞれターミナルを使用します。

1. openocd の起動

   openocdの為にターミナルを開き、ハードウェアに合わせて以下のコマンドを実行します。

   ### 32F7xxxDISCOVERY

   ```sh
   sudo openocd -f /usr/share/openocd/scripts/board/stm32f7discovery.cfg
   ```

   ### NUCLEO-F4xxxx

   ```sh
   sudo openocd -f /usr/share/openocd/scripts/board/st_nucleo_f4.cfg
   ```

   openocdは動作したままの状態になります。

1. シリアルターミナルの起動

   シリアルターミナルの為にターミナルを開き以下のコマンドを実行します。

   ### picocom を使用する場合

   ```sh
   sudo picocom -l /dev/ttyACM0 -b 115200
   ```

   ### cu を使用する場合

   ```sh
   sudo cu -l /dev/ttyACM0 -s 115200
   ```

1. GDB の起動

   GDBの為にターミナルを開き以下のコマンドを実行します。
   このコマンドは、 GadgetSeed をビルドしたディレクトリ以下で実行します。

   ```sh
   arm-none-eabi-gdb -x gdbinit-openocd gadgetseed
   ```

   上記のコマンドでハードウェアにビルドした GadgetSeed が書き込まれます。

   ```
   GNU gdb (GDB) 7.9
   Copyright (C) 2015 Free Software Foundation, Inc.
   License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
   This is free software: you are free to change and redistribute it.
   There is NO WARRANTY, to the extent permitted by law.  Type "show copying"
   and "show warranty" for details.
   This GDB was configured as "--host=x86_64-apple-darwin16.1.0 --target=arm-none-eabi".
   Type "show configuration" for configuration details.
   For bug reporting instructions, please see:
   http://www.gnu.org/software/gdb/bugs/>.
   Find the GDB manual and other documentation resources online at:
   <http://www.gnu.org/software/gdb/documentation/>.
   For help, type "help".
   Type "apropos word" to search for commands related to "word"...
   Reading symbols from gadgetseed...done.
    :
   stm32f7x.cpu: target state: halted
   target halted due to debug-request, current mode: Thread 
   xPSR: 0x01000000 pc: 0x08000200 msp: 0x20020000
   Loading section .VECTORS, size 0x1f8 lma 0x8000000
   Loading section .text, size 0x1b8a85 lma 0x8000200
   Loading section .ARM.excep, size 0x8 lma 0x81b8c88
   Loading section .data, size 0x3a54 lma 0x81b8c90
   Start address 0x8000200, load size 1820377
   Transfer rate: 48 KB/sec, 15297 bytes/write.
   (gdb) 
   ```

   上記のようなメッセージが出力されます。
   プロンプト(gdb)が表示されるまで、数十秒の時間がかかります。

   以下のコマンドを入力して GadgetSeed を実行します。

   ```
   (gdb) c
   Continuing.
   ```

   GadgetSeed が起動し、シリアルターミナルに以下の表示が出力されます。

   ```
   GadgetSeed Ver. 0.9.5
   (c)2010-2018 Takashi SHUDO
   CPU ARCH     : Cortex-M7
   CPU NAME     : STM32F769NIH6
   SYSTEM       : 32F769IDISCOVERY
   Build date   : 07:26:51 Aug  5 2018
   System Clock : 200 MHz
   GS Memory Alloc API is newlib API
   Heap area    : c0177000 - c0fffffc (15241212)
   14883 K byte free
   Graphics device "fb" Type : Frame buffer, Screen size 800x480(2), 16 bit color
   Storage 0: "sd"
   Set RTC Time = 946684953.003
   Touch sensor found
   : Add user shell command "sound"
   AUDIO Buffer Size : 9216
   Audio File searching...
   ```

   シリアルターミナルはコマンドシェルとして各コマンドを入力することができます。
   コマンドシェルについては、以下を参照してください。

   [shell/SHELL.jp.md](shell/SHELL.jp.md)

# PC Linux(Ubuntu)上での実行

PC Linux上で、GadgetSeed のエミュレータを実行することができます。
GadgeSeedのエミュレータは実験的な実装です。動作に不安定なところがあります。
以下のコマンドで、エミュレータのビルドに必要なパッケージをインストールします。

```sh
sudo apt install -y ncurses-dev libasound2-dev aptitude
sudo aptitude install -y libgtk2.0-dev
```

## コンフィグレーションとビルド

エミュレータを実行すにはコンフィグレーションでシステムを選択するときに時に "emu" を選択します。
以下のコマンドで再度コンフィグレーションから GadgetSeed をビルドすることができます。

```sh
make reset
make
```

## 実行

以下のコマンドで、GadgetSeed のエミュレータを実行することができます。

```sh
./gadgetseed
```

# ドキュメント

以下のコマンドで GadgetSeed のドキュメントを作成することができます。

```sh
make docs
```

以下に Doxygen ドキュメントファイルが生成されます。

[docs/html/index.html](docs/html/index.html)

システムの構築や、アプリケーションの開発の為のコンフィグレーション項目は以下を参照して下さい。

[configs/CONFIG.jp.md](configs/CONFIG.jp.md)

# ライセンス

[MIT](LICENSE.txt)

# 著者

Takashi SHUDO([takashi.shudo@nifty.com](mailto:takashi.shudo@nifty.com))
