# GadgetSeed コマンドシェル

GadgetSeedは主にシステム及びアプリケーションのデバッグを目的としたコマンドシェルがあります。

## 利用方法

コマンドシェルは通常、シリアルインタフェースから利用することができます。
PCからはシリアルターミナルソフトを経由して接続して下さい。
以下は、STM32F769NIH6 のコマンドシェルの起動例です。

```
GadgetSeed Ver. 0.9.9
(c)2010-2020 Takashi SHUDO
CPU ARCH     : Cortex-M7
CPU NAME     : STM32F769NIH6
SYSTEM       : 32F769IDISCOVERY
Build date   : 16:10:10 Jan  3 2020
Compiler     : 8.2.1 20181213 (release) [gcc-8-branch revision 267074]
STM32Cube HAL: FW.F7.1.15.0
System Clock : 200 MHz
GS Memory Alloc API is newlib API
Heap area    : c099f204 - c0fffffc (6688248)
6531 K byte free
Graphics device "fb" Type : Frame buffer, Screen size 800x480(2), 16 bit color
Storage 0: "sd"
Set RTC Time = 946706755.007
RMII configuration Hardware Bug Version(0x1000)
Touch sensor found
MAC Address : 02 00 00 00 07 69
Link Down, 100Mb/s, Half
: 
```

コマンドプロンプトは": "です。

## 基本操作

キーボードからコマンドを入力し、エンターキー入力でコマンドを実行します。

カーソルキーの上下入力で、過去に入力したコマンドを再度、入力することができます。

実行可能なコマンド文字列は TAB キーで補完することができます。

## 主なコマンド

### help

実行可能なコマンド一覧を表示します。

```
: help  
help         : Print command help message
sys          : System operation commands
mem          : Memory operation commands
dev          : Device operation commands
task         : Task operation commands
graph        : Graphics operation commands
file         : File strage operation commands
sound        : Sound file play commands
: help sys
 -> sys
  sys          : System operation commands
    info         : Print system info
    timercheck   : Kerne timer & system time check
    kerneltime   : Print kernel time
    systime      : Print System time
    datetime     : Print datetime
    setdate      : Set datetime
    event        : Print sysevent
    setevent     : Set sysevent from key input
    sleep        : Sleep shell task
    console      : Set console device
    sysmem       : Print system memory status
    heap         : Print heap memory status
    reboot       : Reboot system
    interrupt    : Print interrupt count
:
```

### sys info

システム情報を表示します。

```
: sys info
Version    : 0.9.9
CPU ARCH   : Cortex-M7
CPU NAME   : STM32F769NIH6
SYSTEM     : 32F769IDISCOVERY
Build date : 16:10:10 Jan  3 2020
Compiler   : 8.2.1 20181213 (release) [gcc-8-branch revision 267074]
HAL Version: FW.F7.1.15.0
:
```

### dev list

システムに登録されているデバイスドライバの一覧を表示します。

```
: dev list 
timer     : Cortex-M SysTick Driver
debug     : Debug/Error Console
logout    : log output & buffer device
logbuf    : log buffer device
uart      : STM32F7 UART1
uart1     : STM32F7 UART6
input     : STM32F769I-Disc GPIO Button
null      : null device
video     : STM32F769I-Discovery LCD
fb        : Frame buffer(16 bit color)
ts        : STM32F7xxx-Disc Touch Sensor
qspi      : STM32F7xxx QSPI FLASH ROM
sd        : STM32F7xxx SD/MMC Storage
rtc       : STM32F7 RTC
eth       : STM32F7xxx-Discovery Ether
audio     : STM32F7xxx-Disc Audio Out
: 
```

### task list

実行中のタスク情報一覧を表示します。

```
: task list 
PID Name       Pri Status Entry    PC       Stack(size)    SP       SleepTime
  0 IDLE         7 READY  08074819 0807481A 20043150(0400) 20043500         0
  1 init         6 TIMER  08058d49 08075A8E 2003c970(1000) 2003d8c0      2419
  2 touch_sens   0 EVENT  08061a1d 08075A8E 20041980(1000) 200428b8        29
  3 ether_rmii   0 TIMER  080615b5 08075A8E 200414c0(0400) 20041860        29
  4 network      2 TIMER  080253a5 08075A8E 20056508(1000) 20057488        -1
  5 tcpip_thre   2 EVENT  080255a9 08075A8E 200333c8(2000) 20035318        75
  6 devif_thre   1 EVENT  080255a9 08075A8E 200353c8(2000) 20037320        69
  7 shell        3 RUN    08006b19 08075A8E 2002cc68(2000) 2002ebc8         0
  8 soundplay    1 TIMER  080002a9 08075A8E 20014520(C800) 20020c28       -31
  9 musicplay    5 EVENT  080028b9 08075A8E 20024ca8(2000) 20026be0       -21
 10 ir_stream    4 TIMER  08005e69 08075A8E 20027bd8(2800) 2002a358         9
 11 iradio       2 TIMER  08005c31 08075A8E 200273d8(0800) 20027b38        -1
: 
```

### task top

実行中タスクのCPU使用率を1秒間隔で表示し続けます。

```
PID TASK-NAME       PRI RUN-TIME(us)   %CPU
  0 IDLE              7       133055  13.05
  1 init              6            6   0.00
  2 touch_sensor      0          100   0.00
  3 ether_rmii        0           23   0.00
  4 network           2           80   0.00
  5 tcpip_thread      2           43   0.00
  6 devif_thread      1           61   0.00
  7 shell             3          118   0.01
  8 soundplay         1       228259  22.39
  9 musicplay         5       657714  64.51
```

なにかキーが入力されるまで、表示は更新され続けます。

### file dir

ストレージデバイスのファイル一覧を表示します。
ファイルシステムが有効な場合のみ、使用できます。

```
: file dir 
D 17/10/01         SPOTLI~1 .Spotlight-V100
D 18/08/05         FSEVEN~1 .fseventsd
D 14/01/04         ABBEYR~1 Abbey Road
D 15/05/06         AJA Aja
D 18/07/30         KINDOF~1 Kind Of Blue
D 18/07/30         KINGOF~1 King Of Pop [Japan Edition]
D 18/07/30         LIVING~1 Living Inside Your Love
D 18/07/30         PICTURES Pictures
- 18/08/05  39.7K  320X24~1.JPG 320x240-420.jpg
- 18/08/05  95.8K  480X270.JPG 480x270.jpg
- 18/08/05 194.6K  800X480.JPG 800x480.jpg
- 18/08/05   199B  AMAZON~1 .AmazonDriveConfig
D 18/08/05         MUSIC Music
- 00/01/08   1.0K  FWTEST.DAT fwtest.dat
D 18/07/30         TRASH-~1 .Trash-1000
D 18/07/30         MAIDEN~1 Maiden Voyage
D 18/01/06         MOUNTA~1 Mountain Dance
D 14/08/17         RIDEON~1 RIDE ON TIME (Remaster)
D 17/01/22         SAYITW~1 Say It with Silence
D 18/07/30         YELLOW~1.) Yellow Magic Orchestra(US Ver.)
D 10/05/15         ______~1 シンクロニシティーン
0:/ OK
   5 File  331.5K
  16 Dir   493.2M free
:
```
