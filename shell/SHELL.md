# Gadgetseed Command shell

Gadgetseed has a command shell that is primarily designed to debug systems and applications.

## How to use

The command shell is usually available from the serial interface.
Please connect via serial terminal software from PC.
The following is an example of launching the STM32F746NGH6 command shell:

```
GadgetSeed Ver. 0.9.5
(c)2010-2018 Takashi SHUDO
CPU ARCH     : Cortex-M7
CPU NAME     : STM32F746NGH6
SYSTEM       : 32F746GDISCOVERY
Build date   : 11:07:47 Aug  5 2018
System Clock : 162 MHz
GS Memory Alloc API is newlib API
Heap area    : c007f800 - c07ffffc (7866364)
7681 K byte free
Graphics device "fb" Type : Frame buffer, Screen size 480x272(2), 16 bit color
Storage 0: "sd"
Touch sensor found
: Add user shell command "sound"
AUDIO Buffer Size : 9216

: 
:
```

The command prompt is ":".

## Basic operation

Enter the command from the keyboard and run the command with enter key.

You can enter the command you entered in the past again by typing the cursor key up or down.

You can use the TAB key to complete an executable command string.

## Main commands

### help

Displays a list of commands that can be executed.

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

Displays system information.

```
: sys info 
Version    : 0.9.5
CPU ARCH   : Cortex-M7
CPU NAME   : STM32F746NGH6
SYSTEM     : 32F746GDISCOVERY
Build date : 11:07:47 Aug  5 2018
:
```

### dev list

Displays a list of device drivers that are registered with the system.

```
: dev list 
timer     : Cortex-M SysTick Driver
debug     : Debug/Error Console
uart      : STM32F7 UART1
uart1     : STM32F7 UART6
video     : STM32F746G-Discovery LCD
fb        : Frame buffer(16 bit color)
ts        : STM32F7xxx-Disc Touch Sensor
sd        : STM32F7xxx SD/MMC Strage
audio     : STM32F769I-Disc Audio Out
: 
```

### task list

Displays a list of running task information.

```
: task list 
PID Name       Pri Status Entry    PC       Stack(size)    SP       SleepTime
  0 IDLE         3 READY  08015779 0801577A 2001E6D8(0400) 2001EA88         0
  2 touch_sens   1 EVENT  08009275 080168DA 2001CE74(1000) 2001DD90        34
  3 shell        0 RUN    0801B011 080168DA 2002094C(2000) 200228B0         0
  4 soundplay    1 TIMER  08000465 080168DA 20007CC0(C000) 20013C60         4
  5 filemanage   2 EVENT  08002245 080168DA 20016AA4(2000) 200189C8         4
:
```

### task top

Continues to display the CPU usage of the running task at a one-second interval.

```
PID TASK-NAME       PRI RUN-TIME(us)   %CPU
  0 IDLE              3      1005366  97.60
  2 touch_sensor      1          153   0.01
  3 shell             0        23936   2.32
  4 soundplay         1          377   0.03
  5 filemanager       2          168   0.01
```

The display will continue to be updated until you enter a key.

### file dir

Displays a list of files for the storage device.
Available only when the file system is enabled.

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
D 10/05/15 ______ ~ 1 synchronicity Teen
0:/ OK
   5 File  331.5K
  16 Dir   493.2M free
:
```
