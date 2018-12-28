# GadgetSeed

---

![GadgetSeed](gadgetseed_logo.png)

Gadgetseed is a multitasking OS for embedded devices.

## Features

* Multitasking kernel
* Shell commands to help debug
* Standardized device driver APIs
* [Fatfs](http://elm-chan.org/fsw/ff/00index_e.html) file system supported
* [lwIP](https://savannah.nongnu.org/projects/lwip/) TCP/IP protocol stack supported
* Graphics drawing, character font drawing

## Support MCU architecture

   Gadgetseed can operate in the following architectures:

* ARM Cortex-M7
* ARM Cortex-M4
* ARM Cortex-M3

## Support Hardware

   Gadgetseed can operate on the following hardware:

| Hardware | MCU | Architecture |
|---------------------------------------|---------------|----------------|
| [32F769IDISCOVERY](#32F769IDISCOVERY) | STM32F769NIH6 | ARM Cortex-M7  |
| [32F746GDISCOVERY](#32F746GDISCOVERY) | STM32F746NGH6 | ARM Cortex-M7  |
| [32F469IDISCOVERY](#32F469IDISCOVERY) | STM32F469NIH6 | ARM Cortex-M4  |
| [NUCLEO-F411RE](#NUCLEO-F4x1RE)       | STM32F401RET6 | ARM Cortex-M4  |
| [NUCLEO-L152RE](#NUCLEO-L152RE)       | STM32L152RET6 | ARM Cortex-M3  |

<!--
| [NUCLEO-F401RE](#NUCLEO-F4x1RE)       | STM32F411RET6 | ARM Cortex-M4  |
-->

<a name="32F769IDISCOVERY"></a>
### 32F769IDISCOVERY

![32F769IDISCOVERY](https://www.st.com/content/ccc/fragment/product_related/rpn_information/board_photo/group0/5b/1e/e6/2e/d1/1b/45/44/32f769i-disco.jpg/files/stm32f769i-disco.jpg/_jcr_content/translations/en.stm32f769i-disco.jpg)

<https://www.st.com/en/evaluation-tools/32f769idiscovery.html>[English]  
<https://www.st.com/ja/evaluation-tools/32f769idiscovery.html>[Japanese]

<a name="32F746GDISCOVERY"></a>
### 32F746GDISCOVERY

![STM32F746G-Discovery](https://www.st.com/content/ccc/fragment/product_related/rpn_information/board_photo/group0/ea/c4/6d/73/c3/f5/46/e2/stm32f746g-disco/files/stm32f746g-disco.jpg/_jcr_content/translations/en.stm32f746g-disco.jpg)

<https://www.st.com/en/evaluation-tools/32f746gdiscovery.html>[English]  
<https://www.st.com/ja/evaluation-tools/32f746gdiscovery.html>[Japanese]

<a name="32F469IDISCOVERY"></a>
### 32F469IDISCOVERY

![STM32F469-Discovery](https://www.st.com/content/ccc/fragment/product_related/rpn_information/board_photo/group0/e0/b4/a0/64/2f/ff/40/c7/stm32f469i-disco.jpg/files/stm32f469i-disco.jpg/_jcr_content/translations/en.stm32f469i-disco.jpg)

<https://www.st.com/en/evaluation-tools/32f469idiscovery.html>[English]  
<https://www.st.com/ja/evaluation-tools/32f469idiscovery.html>[Japanese]

<a name="NUCLEO-F4x1RE"></a>
### NUCLEO-F411RE

![STM32F411RE](https://www.st.com/content/ccc/fragment/product_related/rpn_information/board_photo/68/fb/69/d3/eb/3d/47/5a/nucleo-F4.jpg/files/nucleo-F4.jpg/_jcr_content/translations/en.nucleo-F4.jpg)

<https://www.st.com/en/evaluation-tools/nucleo-f411re.html>[English]  
<https://www.st.com/ja/evaluation-tools/nucleo-f411re.html>[Japanese]

<!-- NUCLEO-F401RE

<https://www.st.com/en/evaluation-tools/nucleo-f401re.html>[English]  
<https://www.st.com/ja/evaluation-tools/nucleo-f401re.html>[Japanese]
-->

<a name="NUCLEO-L152RE"></a>
### NUCLEO-L152RE

![STM32L152RE](https://www.st.com/content/ccc/fragment/product_related/rpn_information/board_photo/9e/75/14/86/ee/4a/43/78/nucleo-Lx.jpg/files/nucleo-Lx.jpg/_jcr_content/translations/en.nucleo-Lx.jpg)

<https://www.st.com/en/evaluation-tools/nucleo-l152re.html>[English]  
<https://www.st.com/ja/evaluation-tools/nucleo-l152re.html>[Japanese]


## Sample Application

   For a sample application, see:

   [apps/APPLICATIONS.md](apps/APPLICATIONS.md)

## Building the development environment

### Operating Environment configuration

   ![GadgetSeed](gadgetseed_devenv.png)

### For Ubuntu 18.04

1. Install Stm32cubemx

   <https://www.st.com/en/development-tools/stm32cubemx.html>[English]  
   <https://www.st.com/ja/development-tools/stm32cubemx.html>[Japanese]

   Download stm32cubemx. exe from the site above and unzip "En.stm32cubemx.zip".
   Use "SetupSTM32CubeMX-4.26.1.linux" from the unzipped "En.stm32cubemx.zip".

   ```sh
   sudo apt install -y libc6-i386 default-jre openjfx
   sudo ./SetupSTM32CubeMX-5.0.0.linux
   ```

1. Install STM32 HAL and LL drivers

   Start Stm32cubemx and install the HAL driver and the LL driver.

   ```sh
   /usr/local/STMicroelectronics/STM32Cube/STM32CubeMX/STM32CubeMX &
   ```

   Install "STM32Cube MCU Package for STM32F7 Serias Version 1.14.0" for the MCU STM32F7 system.  
   Install "STM32Cube MCU Package for STM32F4 Serias Version 1.23.0" for the MCU STM32F4 system.  
   Install "STM32Cube MCU Package for STM32L1 Serias Version 1.8.1" for the MCU STM32L1 system.

1. Install ARM-GCC

   <https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads>

   Download the GNU Arm Embedded toolchain ("Gcc-arm-none-eabi-8-2018-q4-major-linux.tar.bz2") from the site above.

   Install
   ```sh
   sudo tar xvfj gcc-arm-none-eabi-8-2018-q4-major-linux.tar.bz2 -C /opt
   ```

   Set execution path
   ```sh
   PATH=$PATH:/opt/gcc-arm-none-eabi-8-2018-q4-major/bin
   ```
   It is recommended that the above be appended to the. Bash_aalies, etc.

1. Install the make GCC openocd picocom and other tools

   ```sh
   sudo apt install -y git make gcc unzip openocd picocom otf2bdf p7zip-full
   ```

<!--
1. Install ARM-GCC
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

   If necessary, install it.

   ```sh
   sudo apt-get -y install libusb-1.0.0-dev
   sudo ./SetupSTM32CubeProgrammer-1.0.0.linux
   sudo cp /usr/local/STMicroelectronics/STM32Cube/STM32CubeProgrammer/Drivers/rules/* /etc/udev/rules.d/
   ```
-->

## Build an executable file

1. Run the Make command

   Run the make command in the directory where you extracted the source tree.

   ```sh
   make
   ```

1. Select the system by configuration

   When you see the following, select the system you want to build. Enter a number from 1 to 5 to select the system.

   ```sh
   *** Select target system ***
     1 : 32F469IDISCOVERY               : STM 32F469IDISCOVERY                          : 32F469IDISCOVERY.conf
     2 : 32F746GDISCOVERY               : STM 32F746GDISCOVERY                          : 32F746GDISCOVERY.conf
     3 : 32F769IDISCOVERY               : STM 32F769IDISCOVERY                          : 32F769IDISCOVERY.conf
     4 : NUCLEO-F411RE                  : STM NUCLEO-F411RE                             : NUCLEO-F411RE.conf
     5 : NUCLEO-F411RE_HVGA-LCD-HX8357D : STM NUCLEO-F411RE + MAR3520(HVGA LCD HX8357D) : NUCLEO-F411RE_HVGA-LCD-HX8357D.conf
     6 : NUCLEO-F411RE_QVGA-LCD-ILI9341 : STM NUCLEO-F411RE + K60(QVGA LCD ILI9341)     : NUCLEO-F411RE_QVGA-LCD-ILI9341.conf
     7 : NUCLEO-L152RE                  : STM NUCLEO-L152RE                             : NUCLEO-L152RE.conf
     8 : emu                            : Emulator system with linux                    : emu.conf
   Input No. : 
   ```

1. Selecting Applications by configuration

   When you see the following, select the application you want to build.
   The example is when you select "2" (32F769IDISCOVERY) as the system.
   Enter a number to select the application.

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

   An example is when you select "7" (Music Player High resolution Display) as an application.

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
   echo "const char os_version[] = \"0.9.8\";" > version.c
   echo "const char build_date[] = __DATE__;" >> version.c
   echo "const char build_time[] = __TIME__;" >> version.c
   arm-none-eabi-gcc -g -Wall -mthumb -mcpu=cortex-m7 -mtune=cortex-m7 -fipa-sra -mfpu=fpv5-d16 -mfloat-abi=hard -O2 -I/home/shudo/develop/gadgetseed/include -c version.c
   arm-none-eabi-gcc -g -Wall -mthumb -mcpu=cortex-m7 -mtune=cortex-m7 -fipa-sra -mfpu=fpv5-d16 -mfloat-abi=hard -O2 -I/home/shudo/develop/gadgetseed/include -Wl,-static -Wl,--gc-sections -nostartfiles -o gs-Cortex-M7-32F769IDISCOVERY-musicplay-0.9.8.elf -Tarch/Cortex-M7/systems/32F769IDISCOVERY.lds -Wl,-Map=gs-Cortex-M7-32F769IDISCOVERY-musicplay-0.9.8.map arch/Cortex-M7/start.o version.o \
   main.o gs_logo.o apps/soundplay/soundplay.a apps/musicplay/musicplay.a kernel/kernel.a arch/Cortex-M7/arch.a drivers/drivers.a libs/libs.a kernel/task/task.a extlibs/fatfs/libfatfs.a fs/fs.a uilib/uilib.a graphics/graphics.a font/font.a fontdata/fontdata.a shell/shell.a extlibs/libmad/libmad.a extlibs/faad2/libfaad2.a extlibs/picojpeg/libpicojpeg.a extlibs/libpng/libpng.a extlibs/zlib/libzlib.a arch/Cortex-M7/arch.a -lm kernel/kernel.a arch/Cortex-M7/arch.a drivers/drivers.a libs/libs.a kernel/task/task.a extlibs/fatfs/libfatfs.a fs/fs.a uilib/uilib.a graphics/graphics.a font/font.a fontdata/fontdata.a shell/shell.a extlibs/libmad/libmad.a extlibs/faad2/libfaad2.a extlibs/picojpeg/libpicojpeg.a extlibs/libpng/libpng.a extlibs/zlib/libzlib.a
   ln -f -s gs-Cortex-M7-32F769IDISCOVERY-musicplay-0.9.8.elf gadgetseed
   arm-none-eabi-objdump -h --section=.VECTORS --section=.text --section=.data \
   --section=.bss --section=.stack gs-Cortex-M7-32F769IDISCOVERY-musicplay-0.9.8.elf

   gs-Cortex-M7-32F769IDISCOVERY-musicplay-0.9.8.elf:     file format elf32-littlearm

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

## How to write and run the software

   Use OPENOCD and GDB to write the built software to the hardware.
   Use a serial terminal to use the debug console.
   The following example uses terminals for OPENOCD and GDB and serial terminals.

1. Launching Openocd

   Open the terminal for OPENOCD and run the following command to fit your hardware:

   ### 32F7xxxDISCOVERY

   ```sh
   sudo openocd -f /usr/share/openocd/scripts/board/stm32f7discovery.cfg
   ```

   ### 32F469IDISCOVERY

   ```sh
   sudo openocd -f /usr/share/openocd/scripts/board/stm32f469discovery.cfg
   ```

   ### NUCLEO-F4xxxx

   ```sh
   sudo openocd -f /usr/share/openocd/scripts/board/st_nucleo_f4.cfg
   ```

   ### NUCLEO-L1xxxx

   ```sh
   sudo openocd -f /usr/share/openocd/scripts/board/st_nucleo_l1.cfg
   ```

   OPENOCD will remain running.

1. Launching the serial terminal

   Open the terminal for the serial terminal and run the following command:

   ### When to use picocom

   ```sh
   sudo picocom -l /dev/ttyACM0 -b 115200
   ```

   ### When to use CU

   ```sh
   sudo cu -l /dev/ttyACM0 -s 115200
   ```

1. Starting GDB

   Open the terminal for GDB. Run the following command:
   Run this command under the directory where you built the gadgetseed.

   ```sh
   arm-none-eabi-gdb -x gdbinit-openocd gadgetseed
   ```

   The above command will write the gadgetseed built into the hardware.

   ```
   GNU gdb (GNU Tools for Arm Embedded Processors 8-2018-q4-major) 8.2.50.20181213-git
   Copyright (C) 2018 Free Software Foundation, Inc.
   License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
   This is free software: you are free to change and redistribute it.
   There is NO WARRANTY, to the extent permitted by law.
   Type "show copying" and "show warranty" for details.
   This GDB was configured as "--host=x86_64-apple-darwin10 --target=arm-none-eabi".
   Type "show configuration" for configuration details.
   For bug reporting instructions, please see:
   <http://www.gnu.org/software/gdb/bugs/>.
   Find the GDB manual and other documentation resources online at:
    <http://www.gnu.org/software/gdb/documentation/>.

   For help, type "help".
   Type "apropos word" to search for commands related to "word"...
   Reading symbols from gadgetseed...
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

   A message similar to the one above is printed.
   It takes a few tens of seconds for the prompt (GDB) to be displayed.

   Run Gadgetseed by typing the following command:

   ```
   (gdb) c
   Continuing.
   ```

   Gadgetseed starts and outputs the following display to the serial terminal:

   ```
   GadgetSeed Ver. 0.9.8
   (c)2010-2018 Takashi SHUDO
   CPU ARCH     : Cortex-M7
   CPU NAME     : STM32F769NIH6
   SYSTEM       : 32F769IDISCOVERY
   Build date   : 20:41:11 Dec 28 2018
   Compiler     : 8.2.1 20181213 (release) [gcc-8-branch revision 267074]
   System Clock : 200 MHz
   GS Memory Alloc API is newlib API
   Heap area    : c099709c - c0fffffc (6721376)
   6563 K byte free
   Graphics device "fb" Type : Frame buffer, Screen size 800x480(2), 16 bit color
   Storage 0: "sd"
   Set RTC Time = 946684953.003
   Touch sensor found
   : Add user shell command "sound"
   AUDIO Buffer Size : 9216
   Audio File searching...
   ```

   The serial terminal can enter each command as a command shell.
   For more information about command shells, see:

   [shell/SHELL.md](shell/SHELL.md)

# Running on PC Linux (Ubuntu)

On PC Linux, you can run the Gadgetseed emulator.
Gadgeseed Emulator is an experimental implementation. There is an unstable point in the operation.
Install the packages required to build the emulator with the following command:

```sh
sudo apt install -y ncurses-dev libasound2-dev aptitude
sudo aptitude install -y libgtk2.0-dev
```

## Configuration and build

To run the emulator, select "EMU" When you select the system in the configuration.
You can build Gadgetseed from the configuration again with the following command:

```sh
make reset
make
```

## Run

You can run the Gadgetseed emulator with the following command:

```sh
./gadgetseed
```

# Document

You can create a document for Gadgetseed with the following command:

```sh
make docs
```

The following Doxygen document files are generated:

[docs/html/index.html](docs/html/index.html)

The configuration items for the construction of the system and the development of the application refer to the following.

[configs/CONFIG.md](configs/CONFIG.md)

# License

[MIT](LICENSE.txt)

# Author

Takashi SHUDO([takashi.shudo@nifty.com](mailto:takashi.shudo@nifty.com))
