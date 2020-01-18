# GadgetSeed Configuration

You can set the following configuration items in the configs/\*.conf and configs/systems/\*.conf files

| Configuration Name     | Description |
|------------------------|-------------|
| APPLICATION                        | Application directory                                        |
| APPLICATION2                       | Application Directory (2)                                    |
| APPLICATION3                       | Application Directory (3)                                    |
| APPLICATION4                       | Application Directory (4)                                    |
| APPNAME                            | Application field name in the executable file                |
| APP_STARTUP                        | Application launch function name                             |
| APP_STARTUP2                       | Application start function name (start to second)            |
| APP_STARTUP3                       | Application start function name (start to third)             |
| APP_STARTUP4                       | Application start function name (start to 4th)               |
| ARCH                               | CPU architecture name                                        |
| COMP_ENABLE_FATFS                  | Enable FATFS                                                 |
| COMP_ENABLE_FONTS                  | Enable text font display                                     |
| COMP_ENABLE_GRAPHICS               | Enable graphic drawing                                       |
| COMP_ENABLE_GSFFS                  | Enable GSFFS                                                 |
| COMP_ENABLE_PIPEFS                 | Enable PIPFS                                                 |
| COMP_ENABLE_SHELL                  | Enable the command shell                                     |
| COMP_ENABLE_TCPIP                  | Enable TCP/IP networks                                       |
| CPUNAME                            | CPU name                                                     |
| CPU_CLOCK_HZ                       | CPU clock frequency (Hz)                                     |
| DEFAULT_LOGPRIORITY                | Log Priority for gslogn()                                    |
| DEV_ENABLE_ADC                     | Enable a A/D converter device                                |
| DEV_ENABLE_AUDIO                   | Enable an audio device                                       |
| DEV_ENABLE_BUTTON                  | Enable a button device                                       |
| DEV_ENABLE_BUZZER                  | Enable piezoelectric buzzer devices                          |
| DEV_ENABLE_ETHER                   | Enable Ether devices                                         |
| DEV_ENABLE_GRCONSOLE               | Enable console output for graphics devices                   |
| DEV_ENABLE_I2C                     | Enable i2C host interface devices                            |
| DEV_ENABLE_I2CEEPROM               | Enable I2C EEPROM devices                                    |
| DEV_ENABLE_KEYBOARD                | Enable a keyboard device                                     |
| DEV_ENABLE_LCD_HX8357D             | Enable the HX8357D LCD device                                |
| DEV_ENABLE_LCD_ILI9325             | Enable ILI9325 LCD device                                    |
| DEV_ENABLE_LCD_ILI9341             | Enable ILI9341 LCD device                                    |
| DEV_ENABLE_LED                     | Enable LED devices                                           |
| DEV_ENABLE_NULL                    | Enable null devices                                          |
| DEV_ENABLE_QSPI                    | Enable QSPI ROM devices                                      |
| DEV_ENABLE_RTC                     | Enable real-time clocking                                    |
| DEV_ENABLE_SPI                     | Enable SPI Master Controller Devices                         |
| DEV_ENABLE_SPI2                    | Enable SPI(2) Master Controller Device                       |
| DEV_ENABLE_STORAGE                 | Enable storage (such as An SD card) device                   |
| DEV_ENABLE_TEMPSENSOR_ADT7410      | Enable the ADT7410 temperature sensor device                 |
| DEV_ENABLE_TEMPSENSOR_BME280       | Enable BME280 temperature                                    |
| DEV_ENABLE_TOUCHSENSOR             | Enable touch sensor devices                                  |
| DEV_QSPI_MEMORYMAP                 | Memory Map QSPI-ROM                                          |
| DIFF_FROM_LOCAL_TIME_SEC           | Time difference between UTC and Japan time (+9 hours) (seconds) |
| ENABLE_MUSICPLAY_INTERNETRADIO     | Enable the Internet Radio app                                |
| ENABLE_UILIB                       | Enable the user interface library                            |
| ENABLE_UI_DIALOG_NETSET            | Use the Network Settings dialog                              |
| ENABLE_UI_DIALOG_TIMESET           | Use the time setting dialog                                  |
| ETHERDEV_DEFAULT_MACADDRESS        | Ether Device Default MAC Address                             |
| ETHERDEV_HARDWARE_CHECKSUM         | Enable hardware checksums for Ether devices                  |
| FATFS_ENABLE_CHMOD                 | Enable the chmod API for FAT file systems                    |
| FATFS_ENABLE_EXFAT                 | Enable the exFAT file system                                 |
| FATFS_ENABLE_MKFS                  | Enable the mkfs API for FAT file systems                     |
| FATFS_MAX_DIR_NUM                  | Maximum number of directories that can be opened in FatFs    |
| FATFS_MAX_FILE_NUM                 | Maximum number of files that can be opened in FatFs          |
| FATFS_VOLUME_NUM                   | NUMBER OF FAT FILE SYSTEM (file system) volumes              |
| FONTS_DEFAULT_FONT                 | Default font name                                            |
| FONTS_ENABLE_FONT_12X16            | Enable 12X16 fonts                                           |
| FONTS_ENABLE_FONT_12X24            | Enable 12X24 fonts                                           |
| FONTS_ENABLE_FONT_16X24            | Enable 16X24 fonts                                           |
| FONTS_ENABLE_FONT_4X6              | Enable 4X6 fonts                                             |
| FONTS_ENABLE_FONT_4X8              | Enable 4X8 fonts                                             |
| FONTS_ENABLE_FONT_6X6              | Enable 6X6 fonts                                             |
| FONTS_ENABLE_FONT_8X16             | Enable 8X16 fonts                                            |
| FONTS_ENABLE_FONT_GENSHINGOTHIC    | Enable Genshin Gothic font                                   |
| FONTS_ENABLE_FONT_JISKAN16         | Enable jiskan16 fonts                                        |
| FONTS_ENABLE_FONT_JISKAN16GS       | Enable jiskan16gs fonts                                      |
| FONTS_ENABLE_FONT_JISKAN24         | Enable jiskan24 fonts                                        |
| FONTS_ENABLE_FONT_MISAKI           | Enable Misaki font                                           |
| FONTS_ENABLE_FONT_MPLUS            | Enable M+ fonts                                              |
| FONTS_ENABLE_FONT_NAGA10           | Enable Naga 10 font                                          |
| FONTS_ENABLE_FONT_NUM24X32         | Enable NUM24X32 fonts                                        |
| FONTS_ENABLE_FONT_NUM24X40         | Enable NUM24X40 fonts                                        |
| FONTS_ENABLE_FONT_NUM24X48         | Enable NUM24X48 fonts                                        |
| FONTS_ENABLE_FONT_NUM32X48         | Enable NUM32X48 fonts                                        |
| FONTS_ENABLE_FONT_NUM48X64         | Enable NUM48X64 fonts                                        |
| FONTS_ENABLE_KANJI                 | Enable kanji font drawing                                    |
| FONTS_MAP_BITMAPDATA_EXTROM        | Map font bitmap data to external ROMs                        |
| FS_MAX_FILE_NUM                    | Maximum number of files that can be opened                   |
| FS_VOLUME_NUM                      | Maximum storage device volumes                               |
| GRAPHICS_COLOR_16BIT               | Graphic devices are 16-bit in color                          |
| GRAPHICS_COLOR_24BIT               | Graphic devices are available in 24-bit color                |
| GRAPHICS_COLOR_32BIT               | Graphic devices are 32-bit in color                          |
| GRAPHICS_DISPLAY_HEIGHT            | Graphic device display height                                |
| GRAPHICS_DISPLAY_WIDTH             | Graphic device display width                                 |
| GRAPHICS_DOTSIZE                   | 1 dot size of graphics device for emulator                   |
| GRAPHIC_ENABLE_DEV_MUTEX           | Enable MUTEX for graphics devices                            |
| GSFFS_MAX_DIR_NUM                  | Maximum number of directories that can be opened in GSFFS    |
| GSFFS_MAX_FILE_NUM                 | Maximum number of files that can be opened in GSFFS          |
| GSFFS_USE_ERASESECTCOUNT           | Number of erased sectors used by GSFFS (at least 2 times)    |
| KERNEL_DRIVERS                     | Device drivers required for kernel operation (timers         |
| KERNEL_ENABLE_CALLTRACE            | Enable kernel system call tracing                            |
| KERNEL_ENABLE_INTERRUPT_COUNT      | Enable kernel interrupt counters                             |
| KERNEL_ERROUT_DEVICE               | Error message output device                                  |
| KERNEL_IDLE_TASK_STACK_SIZE        | Stack size of kernel idle tasks                              |
| KERNEL_INITIALTASK_STACK_SIZE      | Stack size of kernel initialization tasks                    |
| KERNEL_MAX_CALLTRACE_RECORD        | Number of kernel system call trace recordings                |
| KERNEL_MAX_DEVICE_NUM              | Maximum number of kernel device drivers                      |
| KERNEL_MAX_KERNEL_TIMER_FUNC       | Maximum number of periodic functions that can be registered with the kernel timer |
| KERNEL_MAX_SYSTEMEVENT_COUNT       | Maximum number of buffers for system events                  |
| KERNEL_MAX_TASK_PRIORITY           | Number of kernel task priority stages                        |
| KERNEL_MESSAGEOUT_DEVICE           | Kernel message output device                                 |
| KERNEL_MESSAGEOUT_LOG              | Enable kernel message logging                                |
| KERNEL_MESSAGEOUT_MEMORY_SIZE      | Kernel message output memory size                            |
| KERNEL_SYSTEMEVENT_LIFE_TIME       | System event life (msec)                                     |
| KERNEL_TIMER_DEVICE                | Kernel timer device                                          |
| KERNEL_TIMER_INTERVAL_MSEC         | Kernel Timer Interrupt Interval (ms)                         |
| KEY_REPEAT_INTERVAL_TIME           | Key repeat interval time (msec)                              |
| KEY_REPEAT_START_TIME              | Time to key repeat start (msec)                              |
| LIB_ENABLE_LIBFAAD2                | Enable the libfaad2 (MPEG-4 and MPEG-2 AAC decoder) library  |
| LIB_ENABLE_LIBMAD                  | Enable the libmad (MPEG audio decoder) library               |
| LIB_ENABLE_LIBPNG                  | Enable the libpng library                                    |
| LIB_ENABLE_MT19937AR               | Enable random number libraries                               |
| LIB_ENABLE_PICOJPEG                | Enable the picojpeg (JPEG decoder) library                   |
| LIB_ENABLE_RANDOM                  | Enable Random Number API                                     |
| LIB_ENABLE_ZLIB                    | Enable the zlib library                                      |
| LOG_DISPLOGPRI                     | Log priority to display                                      |
| LOG_RECORDLOGPRI                   | Log priority to record                                       |
| MAX_LOGBUF_SIZE                    | Log buffer size                                              |
| MAX_TASK_INFO_NUM                  | Maximum number of tasks that can be displayed with the top command |
| MEMORY_ENABLE_HEAP_MEMORY          | Enable heap memory                                           |
| MEMORY_HEAP_IS_NEWLIB              | Heap memory management in newlib                             |
| MEMORY_HEAP_SIZE                   | Heap memory size (without newlib)                            |
| PIPEFS_MAX_BUF_COUNT               | Pipe buffer size                                             |
| PIPEFS_MAX_DIR_NUM                 | Maximum number of directories that can be opened             |
| PIPEFS_MAX_PIPE_NUM                | Maximum number of pipes                                      |
| RTC_DATETIME_SYNC_CYCLE            | RTC and kernel time synchronization calculation period (msec) |
| RTC_RESOLUTION_IS_NOT_SEC          | RTC resolution is more granular than seconds                 |
| SHELL_MAX_COM_ARGV                 | number of shell command maximum arguments                    |
| SHELL_MAX_COM_HIS                  | Number of shell command history                              |
| SHELL_MAX_LINE_COLUMS              | Maximum number of characters on the shell command line       |
| SHELL_USER_COMMAND_NUM             | shell user command registration number                       |
| SHELL_USE_FWTEST                   | Enable the file write test command (fwtest)                  |
| STM32CUBE_HAL_ARCH                 | STM32Cube Target Specification                               |
| SYSTEM                             | System name                                                  |
| TARGET_SYSTEM_32F469IDISCOVERY     | The target system is 32F469IDISCOVERY                        |
| TARGET_SYSTEM_EMU                  | The target system is an emulator.                            |
| TARGET_SYSTEM_NUCLEO_F411RE        | The target system is NUCLEO_F411RE                           |
| TARGET_SYSTEM_STM32F746GDISCOVERY  | Target system is 32F746GDISCOVERY                            |
| TARGET_SYSTEM_STM32F769IDISCOVERY  | The target system is 32F769IDISCOVERY                        |
| TCPIP_DEFAULT_DNSSERVER            | DNS server address                                           |
| TCPIP_DEFAULT_DNSSERVER2           | DNS Server Address 2                                         |
| TCPIP_DEFAULT_GATEWAY              | TCP/IP default gateway address                               |
| TCPIP_DEFAULT_IPADDR               | TCP/IP default IP address                                    |
| TCPIP_DEFAULT_NETMASK              | TCP/IP Default Netmask                                       |
| TCPIP_DEFAULT_NTP_SERVERNAME       | Default NTP server name                                      |
| TCPIP_ENABLE_DHCP                  | Enable DHCP                                                  |
| TCPIP_ENABLE_SNTP                  | Enable SNTP                                                  |
| TCPIP_ENABLE_START_SNTPINIT        | Enable SNTP when the system starts                           |
| TIMEZONE_STR                       | A string that indicates the time zone                        |
| UISTYLEDEF                         | UI style definition header file name                         |
| UI_BUTTON_REPEAT_TIME              | Time to repeat event when UI button long press occurs (ms)   |
| UI_NETWORK_CONFFILE                | Network Config Configuration File Name                       |
