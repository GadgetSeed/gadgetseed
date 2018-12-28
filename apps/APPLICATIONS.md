# Gadgetseed Sample Application

## Music player

   It is a music player to operate with a touch panel.
   You can play MP3 files and AAC files.
   You can view information about a song, including album art.

   The AAC audio decoder uses [faad2](https://nt.com/dsvensson/fad2).

   The MP3 audio decoder uses Libmad (https://www.underbit.com/products/mad/).

   You can view album art in JPEG and PNG format.

   The JPEG image is decoded using [Picojpeg](https://code.google.com/archive/p/picojpeg/).

   The PNG image codec uses libpng (http://www.libpng.org/pub/png/) and [zlib](https://lib. net/).

   Random replay function is used for random number generation [Mt19937ar](http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/MT2002/emt19937ar.html).

   Works with 32F769IDISCOVERY, 32F746GDISCOVERY.

   ![musicplay](musicplay.png)

   The snapshot is 32F769IDISCOVERY.

## File manager

   A simple file Manager. You can play MP3 and AAC files. (32F769IDISCOVERY, 32F746GDISCOVERY only)

   You can view PNG image files. (32F769IDISCOVERY, 32F746GDISCOVERY only)

   You can view the JPEG image file.

   Works with 32F769IDISCOVERY, 32F746GDISCOVERY.

   Operates with NUCLEO-F411RE + LCD (kuman 2.8 inch TFT LCD Shield).

   Operates with NUCLEO-F411RE + LCD (Kuman 3.5 inch TFT LCD Shield).

   ![filemanager](filemanager.png)

   ![filemanager_2](filemanager_2.png)

   The snapshot is 32F769IDISCOVERY.

## Clock application

   This is a digital clock application using RTC.

   ![clock](clock.png)

   ![clock_2](clock_2.png)

   The snapshot is 32F746GDISCOVERY.

## Paint application

   It is a simple drawing software using a touch panel.

   ![paint](paint.png)

   The snapshot is 32F746GDISCOVERY.

## Graphics test

   A random graphics drawing test program.

   ![graphics_test](graphics_test.png)

   Snapshots are for NUCLEO-F411RE + LCD (kuman 2.8 inch TFT LCD Shield).

## LED brink

   It is an application to blink the LED.
   Flashes the LCD mounted on the board.

   Works with 32F769IDISCOVERY, NUCLEO-F411RE.

## Hello world

   Debug Console "Hello, Word!" On the screen.

   ```sh
   GadgetSeed Ver. 0.9.5
   (c)2010-2018 Takashi SHUDO
   CPU ARCH     : Cortex-M7
   CPU NAME     : STM32F746NGH6
   SYSTEM       : 32F746GDISCOVERY
   Build date   : 15:52:32 Jul 31 2018
   System Clock : 162 MHz
   : Hello, world!
   ```

## Network sample

   The lwIP sample, Httpserver, works. Also, telnetd works.
   You can log in with Telnet from outside and use the debug console of Gadgetseed.

   Works with 32F769IDISCOVERY, 32F746GDISCOVERY.
