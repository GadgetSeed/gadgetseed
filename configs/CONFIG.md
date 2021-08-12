# GadgetSeed Configuration

You can set the following configuration items in the configs/\*.conf and configs/systems/\*.conf files

| Configuration Name     | Description |
|------------------------|-------------|
| APPLICATION                        | Application directory                                        |
| APPLICATION2                       | Application directory (2)                                    |
| APPLICATION3                       | Application directory (3)                                    |
| APPLICATION4                       | Application directory (4)                                    |
| APPNAME                            | Executable file application field name                       |
| APP_STARTUP                        | Application startup function name                            |
| APP_STARTUP2                       | Application start function name (second start)               |
| APP_STARTUP3                       | Application start function name (third start)                |
| APP_STARTUP4                       | Application start function name (4th start)                  |
| ARCH                               | CPU architecture name                                        |
| COMP_ENABLE_FATFS                  | Enable FATFS                                                 |
| COMP_ENABLE_FONTS                  | Enable character font display                                |
| COMP_ENABLE_GRAPHICS               | Enable graphic drawing                                       |
| COMP_ENABLE_GSFFS                  | Enable GSFFS                                                 |
| COMP_ENABLE_PIPEFS                 | Enable PIPFS                                                 |
| COMP_ENABLE_SHELL                  | Enable command shell                                         |
| COMP_ENABLE_TCPIP                  | Enable TCP / IP network                                      |
| CPUNAME                            | CPU name                                                     |
| CPU_CLOCK_HZ                       | CPU clock frequency (Hz)                                     |
| DEFAULT_LOGPRIORITY                | Log priority of gslogn ()                                    |
| DEV_ENABLE_ADC                     | Enable A / D converter device                                |
| DEV_ENABLE_AUDIO                   | Enable audio device                                          |
| DEV_ENABLE_BUTTON                  | Enable button device                                         |
| DEV_ENABLE_BUZZER                  | Enable Piezoelectric Buzzer Device                           |
| DEV_ENABLE_ETHER                   | Enable Ether device                                          |
| DEV_ENABLE_GRCONSOLE               | Enable console output for graphics devices                   |
| DEV_ENABLE_I2C                     | Enable I2C host interface device                             |
| DEV_ENABLE_I2CEEPROM               | Enable the I2C EEPROM device                                 |
| DEV_ENABLE_KEYBOARD                | Enable keyboard device                                       |
| DEV_ENABLE_LCD_HX8357D             | Enable HX8357D LCD device                                    |
| DEV_ENABLE_LCD_ILI9325             | Enable the ILI9325 LCD device                                |
| DEV_ENABLE_LCD_ILI9341             | ILI9341 Enable LCD device                                    |
| DEV_ENABLE_LED                     | Enable LED device                                            |
| DEV_ENABLE_NULL                    | Enable null device                                           |
| DEV_ENABLE_QSPI                    | Enable QSPI ROM device                                       |
| DEV_ENABLE_RTC                     | Enable real-time clock                                       |
| DEV_ENABLE_SPI                     | Enable SPI master controller device                          |
| DEV_ENABLE_SPI2                    | Enable SPI (2) master controller device                      |
| DEV_ENABLE_STORAGE                 | Enable storage (such as SD card) devices                     |
| DEV_ENABLE_TEMPSENSOR_ADT7410      | Enable the ADT7410 temperature sensor device                 |
| DEV_ENABLE_TEMPSENSOR_BME280       | Enable BME280 temperature                                    |
| DEV_ENABLE_TOUCHSENSOR             | Enable touch sensor device                                   |
| DEV_QSPI_MEMORYMAP                 | Memory map QSPI-ROM                                          |
| DIFF_FROM_LOCAL_TIME_SEC           | Time difference (seconds) between UTC and Japan time (+9 hours) |
| ENABLE_MUSICPLAY_INTERNETRADIO     | Enable internet radio app                                    |
| ENABLE_UILIB                       | Enable user interface library                                |
| ENABLE_UI_DIALOG_NETSET            | Use the network settings dialog                              |
| ENABLE_UI_DIALOG_TIMESET           | Use the time setting dialog                                  |
| ETHERDEV_DEFAULT_MACADDRESS        | Ether device default MAC address                             |
| ETHERDEV_HARDWARE_CHECKSUM         | Enable hardware checksum for Ether devices                   |
| FATFS_ENABLE_CHMOD                 | Enable chmod API for FAT file system                         |
| FATFS_ENABLE_EXFAT                 | Enable exFAT file system                                     |
| FATFS_ENABLE_MKFS                  | Enable the mkfs API for FAT filesystems                      |
| FATFS_MAX_DIR_NUM                  | Maximum number of directories that FatFs can open            |
| FATFS_MAX_FILE_NUM                 | Maximum number of files that can be opened with FatFs        |
| FATFS_VOLUME_NUM                   | Number of FAT file system volumes                            |
| FONTS_DEFAULT_FONT                 | Default font name                                            |
| FONTS_ENABLE_FONT_12X16            | Enable 12X16 fonts                                           |
| FONTS_ENABLE_FONT_12X24            | Enable 12X24 fonts                                           |
| FONTS_ENABLE_FONT_16X24            | Enable 16X24 fonts                                           |
| FONTS_ENABLE_FONT_4X6              | Enable 4X6 fonts                                             |
| FONTS_ENABLE_FONT_4X8              | Enable 4X8 fonts                                             |
| FONTS_ENABLE_FONT_6X6              | Enable 6X6 fonts                                             |
| FONTS_ENABLE_FONT_8X16             | Enable 8X16 fonts                                            |
| FONTS_ENABLE_FONT_GENSHINGOTHIC    | Enable Genshin Gothic fonts                                  |
| FONTS_ENABLE_FONT_JISKAN16         | Enable jiskan16 font                                         |
| FONTS_ENABLE_FONT_JISKAN16GS       | Enable jiskan16gs font                                       |
| FONTS_ENABLE_FONT_JISKAN24         | Enable jiskan24 font                                         |
| FONTS_ENABLE_FONT_MISAKI           | Enable Misaki font                                           |
| FONTS_ENABLE_FONT_MPLUS            | Enable M + fonts                                             |
| FONTS_ENABLE_FONT_NAGA10           | Enable Naga 10 fonts                                         |
| FONTS_ENABLE_FONT_NUM24X32         | Enable NUM24X32 font                                         |
| FONTS_ENABLE_FONT_NUM24X40         | Enable NUM24X40 font                                         |
| FONTS_ENABLE_FONT_NUM24X48         | Enable NUM24X48 font                                         |
| FONTS_ENABLE_FONT_NUM32X48         | Enable NUM32X48 font                                         |
| FONTS_ENABLE_FONT_NUM48X64         | Enable NUM48X64 font                                         |
| FONTS_ENABLE_KANJI                 | Enable drawing of kanji font                                 |
| FONTS_MAP_BITMAPDATA_EXTROM        | Map font bitmap data to external ROM                         |
| FS_MAX_FILE_NUM                    | Maximum number of files that can be opened                   |
| FS_VOLUME_NUM                      | Maximum number of storage device volumes                     |
| GRAPHICS_COLOR_16BIT               | Graphic device is 16-bit color                               |
| GRAPHICS_COLOR_24BIT               | Graphic device is 24-bit color                               |
| GRAPHICS_COLOR_32BIT               | Graphic device is 32-bit color                               |
| GRAPHICS_DISPLAY_HEIGHT            | Graphic device display height                                |
| GRAPHICS_DISPLAY_WIDTH             | Graphic device display width                                 |
| GRAPHICS_DOTSIZE                   | 1-dot size of graphic device for emulator                    |
| GRAPHIC_ENABLE_DEV_MUTEX           | Enable MUTEX for graphics device                             |
| GSFFS_MAX_DIR_NUM                  | Maximum number of directories that can be opened by GSFFS    |
| GSFFS_MAX_FILE_NUM                 | Maximum number of files that can be opened by GSFFS          |
| GSFFS_USE_ERASESECTCOUNT           | Number of erased sectors used by GSFFS (minimum 2 or more)   |
| KERNEL_DRIVERS                     | Device drivers (timer                                        |
| KERNEL_ENABLE_CALLTRACE            | Enable kernel system call tracing                            |
| KERNEL_ENABLE_INTERRUPT_COUNT      | Enable kernel interrupt counter                              |
| KERNEL_ERROUT_DEVICE               | Error message output device                                  |
| KERNEL_IDLE_TASK_STACK_SIZE        | Kernel idle task stack size                                  |
| KERNEL_INITIALTASK_STACK_SIZE      | Stack size of kernel initialization task                     |
| KERNEL_MAX_CALLTRACE_RECORD        | Kernel system call trace record count                        |
| KERNEL_MAX_DEVICE_NUM              | Maximum number of kernel device drivers                      |
| KERNEL_MAX_KERNEL_TIMER_FUNC       | Maximum number of periodic processing functions that can be registered in the kernel timer |
| KERNEL_MAX_SYSTEMEVENT_COUNT       | Maximum number of buffers for system events                  |
| KERNEL_MAX_TASK_PRIORITY           | Kernel task priority stages                                  |
| KERNEL_MESSAGEOUT_DEVICE           | Kernel message output device                                 |
| KERNEL_MESSAGEOUT_LOG              | Enable logging of kernel messages                            |
| KERNEL_MESSAGEOUT_MEMORY_SIZE      | Kernel message output memory size                            |
| KERNEL_SYSTEMEVENT_LIFE_TIME       | System event lifetime (msec)                                 |
| KERNEL_TIMER_DEVICE                | Kernel timer device                                          |
| KERNEL_TIMER_INTERVAL_MSEC         | Kernel timer interrupt interval (ms)                         |
| KEY_REPEAT_INTERVAL_TIME           | Key repeat interval time (msec)                              |
| KEY_REPEAT_START_TIME              | Time to start key repeat (msec)                              |
| LIB_ENABLE_LIBFAAD2                | Enable the libfaad2 (MPEG-4 and MPEG-2 AAC decoder) library  |
| LIB_ENABLE_LIBMAD                  | Enable the libmad (MPEG audio decoder) library               |
| LIB_ENABLE_LIBPNG                  | Enable libpng library                                        |
| LIB_ENABLE_MT19937AR               | Enable random number library                                 |
| LIB_ENABLE_PICOJPEG                | Enable picojpeg (JPEG decoder) library                       |
| LIB_ENABLE_RANDOM                  | Enable the random number API                                 |
| LIB_ENABLE_ZLIB                    | Enable the zlib library                                      |
| LOG_DISPLOGPRI                     | Log priority to display                                      |
| LOG_RECORDLOGPRI                   | Log priority to record                                       |
| MAX_LOGBUF_SIZE                    | Log buffer size                                              |
| MAX_TASK_INFO_NUM                  | Maximum number of tasks that can be displayed with the top command |
| MEMORY_ENABLE_HEAP_MEMORY          | Enable heap memory                                           |
| MEMORY_HEAP_IS_NEWLIB              | Heap memory management with newlib                           |
| MEMORY_HEAP_SIZE                   | Heap memory size (without newlib)                            |
| PIPEFS_MAX_BUF_COUNT               | Pipe buffer size                                             |
| PIPEFS_MAX_DIR_NUM                 | Maximum number of directories that can be opened             |
| PIPEFS_MAX_PIPE_NUM                | Maximum number of pipes                                      |
| RTC_DATETIME_SYNC_CYCLE            | Synchronous calculation cycle of RTC and kernel time (msec)  |
| RTC_RESOLUTION_IS_NOT_SEC          | RTC resolution is finer than seconds                         |
| SHELL_MAX_COM_ARGV                 | shell command Maximum number of arguments                    |
| SHELL_MAX_COM_HIS                  | number of shell command histories                            |
| SHELL_MAX_LINE_COLUMS              | shell Maximum number of characters on the command line       |
| SHELL_USER_COMMAND_NUM             | number of shell user commands that can be registered         |
| SHELL_USE_FWTEST                   | Enable the file write test command (fwtest)                  |
| STM32CUBE_HAL_ARCH                 | STM32Cube targeting                                          |
| STM32CUBE_HAL_DIR                  | STM32 HAL installation directory specification               |
| SYSTEM                             | system-name                                                  |
| TARGET_SYSTEM_32F469IDISCOVERY     | Target system is 32F469 IDIS COVERY                          |
| TARGET_SYSTEM_EMU                  | The target system is an emulator                             |
| TARGET_SYSTEM_NUCLEO_F411RE        | Target system is NUCLEO_F411RE                               |
| TARGET_SYSTEM_STM32F746GDISCOVERY  | Target system is 32F746G DISCOVERY                           |
| TARGET_SYSTEM_STM32F769IDISCOVERY  | Target system is 32F769IDISCOVERY                            |
| TARGET_SYSTEM_STM32H747IDISCOVERY  | Target system is 32H747 IDIS COVERY                          |
| TCPIP_DEFAULT_DNSSERVER            | DNS server address                                           |
| TCPIP_DEFAULT_DNSSERVER2           | DNS server address 2                                         |
| TCPIP_DEFAULT_GATEWAY              | TCP / IP default gateway address                             |
| TCPIP_DEFAULT_IPADDR               | TCP / IP default IP address                                  |
| TCPIP_DEFAULT_NETMASK              | TCP / IP default netmask                                     |
| TCPIP_DEFAULT_NTP_SERVERNAME       | Default NTP server name                                      |
| TCPIP_ENABLE_DHCP                  | Enable DHCP                                                  |
| TCPIP_ENABLE_SNTP                  | Enable SNTP                                                  |
| TCPIP_ENABLE_START_SNTPINIT        | Enable SNTP at system startup                                |
| TCPIP_ENABLE_STATS                 | Enable TCP / IP statistics                                   |
| TIMEZONE_STR                       | A string indicating the time zone                            |
| UISTYLEDEF                         | UI style definition header file name                         |
| UI_BUTTON_REPEAT_TIME              | Time until repeat event occurs when UI button is pressed and held (ms) |
| UI_NETWORK_CONFFILE                | Network config configuration file name                       |
