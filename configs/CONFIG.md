# Gadgetseed Configuration

Configs/\ * .conf and configs/systems/\ * .conf files can have the following configuration items:

| Configuration name | Contents |
|------------------------|------|
| APPLICATION | Application Directory |
| APPLICATION2 | Application directory (2) |
| APPLICATION3 | Application directory (3) |
| APPLICATION4 | Application directory (4) |
| APPNAME | Application field name for executable file |
| APP_STARTUP | Application Startup Function name |
| APP_STARTUP2 | Application startup function name (second boot) |
| APP_STARTUP3 | Application Startup function name (3rd launch) |
| APP_STARTUP4 | Application startup function name (Launch to 4th) |
| ARCH | CPU Architecture Name |
| COMP_ENABLE_FATFS | Enable Fatfs |
| COMP_ENABLE_FONTS | Enable character Font display |
| COMP_ENABLE_GRAPHICS | Enable Graphic Drawing |
| COMP_ENABLE_PIPEFS | Enable PIPFS |
| COMP_ENABLE_SHELL | Enable Command Shell |
| COMP_ENABLE_TCPIP | TCP/IP. Enabling Networking |
| CPUNAME | CPU name |
| CPU_CLOCK_HZ | CPU clock frequency (HZ) |
| DEFAULT_LOGPRIORITY | Gslogn () Log Priority |
| DEV_ENABLE_ADC | Enable A/D converter device |
| DEV_ENABLE_AUDIO | Enable Audio Devices |
| DEV_ENABLE_BUTTON | Enable Button Devices |
| DEV_ENABLE_BUZZER | Enable Piezoelectric buzzer Device |
| DEV_ENABLE_ETHER | Enable Ether Device |
| DEV_ENABLE_GRCONSOLE | Enable console output for graphics devices |
| DEV_ENABLE_I2C | Enable I2C Host Interface Devices |
| DEV_ENABLE_I2CEEPROM | Enable I2C EEPROM Devices |
| DEV_ENABLE_KEYBOARD | Enable Keyboard Devices |
| DEV_ENABLE_LCD_HX8357D | Enable HX8357D LCD Device |
| DEV_ENABLE_LCD_ILI9325 | Enable ILI9325 LCD Device |
| DEV_ENABLE_LCD_ILI9341 | Enable ILI9341 LCD Device |
| DEV_ENABLE_LED | Enable LED devices |
| DEV_ENABLE_NULL | Enable NULL devices |
| DEV_ENABLE_RTC | Enable real-Time Clock |
| DEV_ENABLE_SPI | Enable SPI Master Controller devices |
| DEV_ENABLE_SPI2 | SPI (2) Enable Master Controller devices |
| DEV_ENABLE_STORAGE | Enable storage (SD card, etc.) devices |
| DEV_ENABLE_TEMPSENSOR_ADT7410 | Enable adt7410 temperature sensor Device |
| DEV_ENABLE_TEMPSENSOR_BME280 | bme280 temperature, humidity, Pressure sensor enable device |
| DEV_ENABLE_TOUCHSENSOR | Enable Touch sensor Devices |
| DIFF_FROM_LOCAL_TIME_SEC | Difference between UTC and Japan time (+ 9 hours) (seconds) |
| ENABLE_UILIB | Enable the User interface Library |
| ETHERDEV_DEFAULT_MACADDRESS | Ether device default MAC address |
| ETHERDEV_HARDWARE_CHECKSUM | Enable hardware checksums for Ether devices |
| FATFS_MAX_DIR_NUM | Maximum number of directories that can be opened in Fatfs |
| FATFS_MAX_FILE_NUM | Maximum number of files that can be opened in Fatfs |
| FONTS_ENABLE_FONT_12X16 | 12x16 Enable Fonts |
| FONTS_ENABLE_FONT_12X24 | 12x24 Enable Fonts |
| FONTS_ENABLE_FONT_16X24 | Enable 16x24 Fonts |
| FONTS_ENABLE_FONT_4X6 | Enable 4x6 Fonts |
| FONTS_ENABLE_FONT_4X8 | Enable 4x8 Fonts |
| FONTS_ENABLE_FONT_6X6 | Enable 6x6 fonts |
| FONTS_ENABLE_FONT_8X16 | Enable 8x16 Fonts |
| FONTS_ENABLE_FONT_GENSHINGOTHIC | Enable source True Gothic font |
| FONTS_ENABLE_FONT_JISKAN16 | Enable Jiskan16 Font |
| FONTS_ENABLE_FONT_JISKAN24 | Enable Jiskan24 Font |
| FONTS_ENABLE_FONT_MISAKI | Enable Misaki Font |
| FONTS_ENABLE_FONT_MPLUS | Enable M + fonts |
| FONTS_ENABLE_FONT_NAGA10 | Enable Naga 10 Fonts |
| FONTS_ENABLE_FONT_NUM24X32 | Enable num24x32 Font |
| FONTS_ENABLE_FONT_NUM24X40 | Enable num24x40 Font |
| FONTS_ENABLE_FONT_NUM24X48 | Enable num24x48 Font |
| FONTS_ENABLE_FONT_NUM48X64 | Enable num48x64 Font |
| FONTS_ENABLE_KANJI | Enable Kanji Font Drawing |
| FS_MAX_FILE_NUM | Maximum number of files to open |
| FS_VOLUME_NUM | Maximum number of storage device volumes |
| GRAPHICS_COLOR_16BIT | Graphics device 16-bit color |
| GRAPHICS_COLOR_24BIT | Graphics device 24-bit color |
| GRAPHICS_COLOR_32BIT | Graphics device 32-bit color |
| GRAPHICS_DISPLAY_HEIGHT | Graphic Device Display Height |
| GRAPHICS_DISPLAY_WIDTH | Graphic Device Display Width |
| GRAPHICS_DOTSIZE | One-dot size for emulator graphics devices |
| KERNEL_DRIVERS | Device drivers required for kernel operation (timers, serial, etc.) |
| KERNEL_ENABLE_CALLTRACE | Enable kernel system call Tracing |
| KERNEL_ENABLE_INTERRUPT_COUNT | Enable Kernel Interrupt Counter |
| KERNEL_ERROUT_DEVICE | Error message Output Device |
| KERNEL_IDLE_TASK_STACK_SIZE | Kernel idle task stack size |
| KERNEL_INITIALTASK_STACK_SIZE | Kernel initialization task Stack size |
| KERNEL_MAX_CALLTRACE_RECORD | Kernel system call Trace Record Count |
| KERNEL_MAX_DEVICE_NUM | Kernel maximum number of device drivers |
| KERNEL_MAX_KERNEL_TIMER_FUNC | Maximum number of periodic functions that can be registered with a kernel timer |
| KERNEL_MAX_SYSTEMEVENT_COUNT | Maximum number of buffers for system events |
| KERNEL_MAX_TASK_PRIORITY | Kernel task priority number of stages |
| KERNEL_MESSAGEOUT_DEVICE | Kernel Message Output Device |
| KERNEL_MESSAGEOUT_LOG | Enable kernel message Logging |
| KERNEL_MESSAGEOUT_MEMORY_SIZE | Kernel message output Memory size |
| KERNEL_SYSTEMEVENT_LIFE_TIME | System Event Life (msec) |
| KERNEL_TIMER_DEVICE | Kernel Timer Devices |
| KERNEL_TIMER_INTERVAL_MSEC | Kernel Timer interrupt interval (ms) |
| KEY_REPEAT_INTERVAL_TIME | Key repeat Interval time (msec) |
| KEY_REPEAT_START_TIME | Time to start key repeat (msec) |
| LIB_ENABLE_LIBFAAD2 | Enable the LIBFAAD2 (MPEG-4 and MPEG-2 AAC decoder) Library |
| LIB_ENABLE_LIBMAD | Enable the Libmad (MPEG Audio decoder) Library |
| LIB_ENABLE_LIBPNG | Enable Libpng Library |
| LIB_ENABLE_PICOJPEG | Enable Picojpeg (JPEG decoder) Library |
| LIB_ENABLE_RANDOM | Enable Random number Library |
| LIB_ENABLE_ZLIB | Enable Zlib Library |
| MAX_LOGBUF_SIZE | Log Buffer Size |
| MAX_TASK_INFO_NUM | Maximum number of tasks that can be displayed with the top command |
| MEMORY_ENABLE_HEAP_MEMORY | Enable Heap Memory |
| MEMORY_HEAP_IS_NEWLIB | Heap memory Management in Newlib |
| MEMORY_HEAP_SIZE | Heap memory size (without newlib) |
| PIPEFS_MAX_BUF_COUNT | Pipe Buffer Size |
| PIPEFS_MAX_DIR_NUM | Maximum number of directories to open |
| PIPEFS_MAX_PIPE_NUM | Maximum number of pipes |
| RTC_DATETIME_SYNC_CYCLE | RTC and Kernel Time synchronous computation period (msec) |
| RTC_RESOLUTION_IS_NOT_SEC | RTC resolution is more detailed than seconds |
| SHELL_MAX_COM_ARGV | Shell command number of maximum arguments |
| SHELL_MAX_COM_HIS | Shell Command History Count |
| SHELL_MAX_LINE_COLUMS | Maximum number of characters in the shell command line |
| SHELL_USER_COMMAND_NUM | The number of shell user commands can be registered |
| SHELL_USE_FWTEST | Enable file Write Test command (fwtest) |
| STM32CUBE_HAL_ARCH | Stm32cube Target Designation |
| SYSTEM | System name |
| TARGET_SYSTEM_32F469IDISCOVERY | Target System 32f469idiscovery |
| TARGET_SYSTEM_EMU | Target System Emulator |
| TARGET_SYSTEM_NUCLEO_F411RE | Target System Nucleo_f411re |
| TARGET_SYSTEM_STM32F746GDISCOVERY | Target System 32F746GDISCOVERY |
| TARGET_SYSTEM_STM32F769IDISCOVERY | Target System 32F769IDISCOVERY |
| TCPIP_DEFAULT_DNSSERVER | DNS server Address |
| TCPIP_DEFAULT_DNSSERVER2 | DNS Server Address 2 |
| TCPIP_DEFAULT_GATEWAY | TCP/IP Default gateway Address |
| TCPIP_DEFAULT_IPADDR | TCP/IP. Default IP Address |
| TCPIP_DEFAULT_NETMASK | TCP/IP Default Netmask |
| TIMEZONE_STR | String indicating time zone |
