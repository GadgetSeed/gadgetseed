# GadgetSeed コンフィグレーション

configs/\*.conf および configs/systems/\*.conf ファイルには以下のコンフィグレーション項目を設定することができます。

| コンフィグレーション名 | 内容 |
|------------------------|------|
| APPLICATION                        | アプリケーションディレクトリ                                               |
| APPLICATION2                       | アプリケーションディレクトリ(2)                                            |
| APPLICATION3                       | アプリケーションディレクトリ(3)                                            |
| APPLICATION4                       | アプリケーションディレクトリ(4)                                            |
| APPNAME                            | 実行ファイルのアプリケーションフィールド名                                        |
| APP_STARTUP                        | アプリケション起動関数名                                                 |
| APP_STARTUP2                       | アプリケション起動関数名(2番めに起動)                                         |
| APP_STARTUP3                       | アプリケション起動関数名(3番めに起動)                                         |
| APP_STARTUP4                       | アプリケション起動関数名(4番めに起動)                                         |
| ARCH                               | CPU アーキテクチャ名                                                 |
| COMP_ENABLE_FATFS                  | FATFSを有効にする                                                  |
| COMP_ENABLE_FONTS                  | 文字フォント表示を有効にする                                               |
| COMP_ENABLE_GRAPHICS               | グラフィック描画を有効にする                                               |
| COMP_ENABLE_GSFFS                  | GSFFSを有効にする                                                  |
| COMP_ENABLE_PIPEFS                 | PIPFSを有効にする                                                  |
| COMP_ENABLE_SHELL                  | コマンドシェルを有効にする                                                |
| COMP_ENABLE_TCPIP                  | TCP/IPネットワークを有効にする                                           |
| CPUNAME                            | CPU 名                                                        |
| CPU_CLOCK_HZ                       | CPUクロック周波数(Hz)                                               |
| DEFAULT_LOGPRIORITY                | gslogn()のログプライオリティ                                           |
| DEV_ENABLE_ADC                     | A/Dコンバータデバイスを有効にする                                           |
| DEV_ENABLE_AUDIO                   | オーディオデバイスを有効にする                                              |
| DEV_ENABLE_BUTTON                  | ボタンデバイスを有効にする                                                |
| DEV_ENABLE_BUZZER                  | 圧電ブザーデバイスを有効にする                                              |
| DEV_ENABLE_ETHER                   | Etherデバイスを有効にする                                              |
| DEV_ENABLE_GRCONSOLE               | グラフィックデバイス用コンソール出力を有効にする                                     |
| DEV_ENABLE_I2C                     | I2Cホストインタフェースデバイスを有効にする                                      |
| DEV_ENABLE_I2CEEPROM               | I2C EEPROMデバイスを有効にする                                         |
| DEV_ENABLE_KEYBOARD                | キーボードデバイスを有効にする                                              |
| DEV_ENABLE_LCD_HX8357D             | HX8357D LCDデバイスを有効にする                                        |
| DEV_ENABLE_LCD_ILI9325             | ILI9325 LCDデバイスを有効にする                                        |
| DEV_ENABLE_LCD_ILI9341             | ILI9341 LCDデバイスを有効にする                                        |
| DEV_ENABLE_LED                     | LEDデバイスを有効にする                                                |
| DEV_ENABLE_NULL                    | NULLデバイスを有効にする                                               |
| DEV_ENABLE_QSPI                    | QSPI ROMデバイスを有効にする                                           |
| DEV_ENABLE_RTC                     | リアルタイムクロックを有効にする                                             |
| DEV_ENABLE_SPI                     | SPIマスタコントローラデバイスを有効にする                                       |
| DEV_ENABLE_SPI2                    | SPI(2)マスタコントローラデバイスを有効にする                                    |
| DEV_ENABLE_STORAGE                 | ストレージ(SDカードなど)デバイスを有効にする                                     |
| DEV_ENABLE_TEMPSENSOR_ADT7410      | ADT7410温度センサデバイスを有効にする                                       |
| DEV_ENABLE_TEMPSENSOR_BME280       | BME280温度、湿度、気圧センサデバイスを有効にする                                  |
| DEV_ENABLE_TOUCHSENSOR             | タッチセンサデバイスを有効にする                                             |
| DEV_QSPI_MEMORYMAP                 | QSPI-ROMをメモリマップする                                            |
| DIFF_FROM_LOCAL_TIME_SEC           | UTCと日本時間(+9時間)との時差(秒)                                        |
| ENABLE_MUSICPLAY_INTERNETRADIO     | インターネットラジオアプリを有効にする                                          |
| ENABLE_UILIB                       | ユーザインタフェースライブラリを有効にする                                        |
| ENABLE_UI_DIALOG_NETSET            | ネットワーク設定ダイアログを使用する                                           |
| ENABLE_UI_DIALOG_TIMESET           | 時刻設定ダイアログを使用する                                               |
| ETHERDEV_DEFAULT_MACADDRESS        | EtherデバイスデフォルトMACアドレス                                        |
| ETHERDEV_HARDWARE_CHECKSUM         | Etherデバイスのハードウェアチェックサムを有効にする                                 |
| FATFS_ENABLE_CHMOD                 | FATファイルシステムのchmod APIを有効にする                                  |
| FATFS_ENABLE_EXFAT                 | exFATファイルシステムを有効にする                                          |
| FATFS_ENABLE_MKFS                  | FATファイルシステムのmkfs APIを有効にする                                   |
| FATFS_MAX_DIR_NUM                  | FatFsでオープンできる最大ディレクトリ数                                       |
| FATFS_MAX_FILE_NUM                 | FatFsでオープンできる最大ファイル数                                         |
| FATFS_VOLUME_NUM                   | FATファイルシステムのボリューム数                                           |
| FONTS_DEFAULT_FONT                 | デフォルトのフォント名                                                  |
| FONTS_ENABLE_FONT_12X16            | 12X16フォントを有効にする                                              |
| FONTS_ENABLE_FONT_12X24            | 12X24フォントを有効にする                                              |
| FONTS_ENABLE_FONT_16X24            | 16X24フォントを有効にする                                              |
| FONTS_ENABLE_FONT_4X6              | 4X6フォントを有効にする                                                |
| FONTS_ENABLE_FONT_4X8              | 4X8フォントを有効にする                                                |
| FONTS_ENABLE_FONT_6X6              | 6X6フォントを有効にする                                                |
| FONTS_ENABLE_FONT_8X16             | 8X16フォントを有効にする                                               |
| FONTS_ENABLE_FONT_GENSHINGOTHIC    | 源真ゴシックフォントを有効にする                                             |
| FONTS_ENABLE_FONT_JISKAN16         | jiskan16フォントを有効にする                                           |
| FONTS_ENABLE_FONT_JISKAN16GS       | jiskan16gsフォントを有効にする                                         |
| FONTS_ENABLE_FONT_JISKAN24         | jiskan24フォントを有効にする                                           |
| FONTS_ENABLE_FONT_MISAKI           | 美咲フォントを有効にする                                                 |
| FONTS_ENABLE_FONT_MPLUS            | M+フォントを有効にする                                                 |
| FONTS_ENABLE_FONT_NAGA10           | ナガ10フォントを有効にする                                               |
| FONTS_ENABLE_FONT_NUM24X32         | NUM24X32フォントを有効にする                                           |
| FONTS_ENABLE_FONT_NUM24X40         | NUM24X40フォントを有効にする                                           |
| FONTS_ENABLE_FONT_NUM24X48         | NUM24X48フォントを有効にする                                           |
| FONTS_ENABLE_FONT_NUM32X48         | NUM32X48フォントを有効にする                                           |
| FONTS_ENABLE_FONT_NUM48X64         | NUM48X64フォントを有効にする                                           |
| FONTS_ENABLE_KANJI                 | 漢字フォントの描画を有効にする                                              |
| FONTS_MAP_BITMAPDATA_EXTROM        | フォントビットマップデータを外部ROMにマップする                                    |
| FS_MAX_FILE_NUM                    | オープンできる最大ファイル数                                               |
| FS_VOLUME_NUM                      | 最大ストレージデバイスボリューム数                                            |
| GRAPHICS_COLOR_16BIT               | グラフィックデバイスは16ビットカラー                                          |
| GRAPHICS_COLOR_24BIT               | グラフィックデバイスは24ビットカラー                                          |
| GRAPHICS_COLOR_32BIT               | グラフィックデバイスは32ビットカラー                                          |
| GRAPHICS_DISPLAY_HEIGHT            | グラフィックデバイスの表示高さ                                              |
| GRAPHICS_DISPLAY_WIDTH             | グラフィックデバイスの表示幅                                               |
| GRAPHICS_DOTSIZE                   | エミュレータ用グラフィックデバイスの1ドットサイズ                                    |
| GRAPHIC_ENABLE_DEV_MUTEX           | グラフィックデバイスのMUTEXを有効にする                                       |
| GSFFS_MAX_DIR_NUM                  | GSFFSでオープンできる最大ディレクトリ数                                       |
| GSFFS_MAX_FILE_NUM                 | GSFFSでオープンできる最大ファイル数                                         |
| GSFFS_USE_ERASESECTCOUNT           | GSFFSで使用する消去セクタ数(最低2以上)                                      |
| KERNEL_DRIVERS                     | カーネル動作に必要なデバイスドライバ(タイマ、シリアル等)                                |
| KERNEL_ENABLE_CALLTRACE            | カーネルシステムコールトレースを有効にする                                        |
| KERNEL_ENABLE_INTERRUPT_COUNT      | カーネル割込カウンタを有効にする                                             |
| KERNEL_ERROUT_DEVICE               | エラーメッセージ出力デバイス                                               |
| KERNEL_IDLE_TASK_STACK_SIZE        | カーネルアイドルタスクのスタックサイズ                                          |
| KERNEL_INITIALTASK_STACK_SIZE      | カーネル初期化タスクのスタックサイズ                                           |
| KERNEL_MAX_CALLTRACE_RECORD        | カーネルシステムコールトレース記録数                                           |
| KERNEL_MAX_DEVICE_NUM              | カーネル最大デバイスドライバ数                                              |
| KERNEL_MAX_KERNEL_TIMER_FUNC       | カーネルタイマに登録できる最大定期処理関数数                                       |
| KERNEL_MAX_SYSTEMEVENT_COUNT       | システムイベントの最大バッファ数                                             |
| KERNEL_MAX_TASK_PRIORITY           | カーネルタスクプライオリティ段階数                                            |
| KERNEL_MESSAGEOUT_DEVICE           | カーネルメッセージ出力デバイス                                              |
| KERNEL_MESSAGEOUT_LOG              | カーネルメッセージのログ出力を有効にする                                         |
| KERNEL_MESSAGEOUT_MEMORY_SIZE      | カーネルメッセージ出力メモリサイズ                                            |
| KERNEL_SYSTEMEVENT_LIFE_TIME       | システムイベントの寿命(msec)                                            |
| KERNEL_TIMER_DEVICE                | カーネルタイマデバイス                                                  |
| KERNEL_TIMER_INTERVAL_MSEC         | カーネルタイマ割り込み間隔(ms)                                            |
| KEY_REPEAT_INTERVAL_TIME           | キーリピート間隔時間(msec)                                             |
| KEY_REPEAT_START_TIME              | キーリピート開始までの時間(msec)                                          |
| LIB_ENABLE_LIBFAAD2                | libfaad2(MPEG-4 and MPEG-2 AAC decoder)ライブラリを有効にする           |
| LIB_ENABLE_LIBMAD                  | libmad(MPEG audio decoder)ライブラリを有効にする                        |
| LIB_ENABLE_LIBPNG                  | libpngライブラリを有効にする                                            |
| LIB_ENABLE_MT19937AR               | 乱数ライブラリを有効にする                                                |
| LIB_ENABLE_PICOJPEG                | picojpeg(JPEG decoder)ライブラリを有効にする                            |
| LIB_ENABLE_RANDOM                  | 乱数APIを有効にする                                                  |
| LIB_ENABLE_ZLIB                    | zlibライブラリを有効にする                                              |
| LOG_DISPLOGPRI                     | 表示するログ優先順位                                                   |
| LOG_RECORDLOGPRI                   | 記録するログ優先順位                                                   |
| MAX_LOGBUF_SIZE                    | ログバッファサイズ                                                    |
| MAX_TASK_INFO_NUM                  | topコマンドで表示可能な最大タスク数                                          |
| MEMORY_ENABLE_HEAP_MEMORY          | ヒープメモリを有効にする                                                 |
| MEMORY_HEAP_IS_NEWLIB              | ヒープメモリ管理をnewlibで行う                                           |
| MEMORY_HEAP_SIZE                   | ヒープメモリサイズ(newlibを使わない場合)                                     |
| PIPEFS_MAX_BUF_COUNT               | パイプバッファサイズ                                                   |
| PIPEFS_MAX_DIR_NUM                 | オープンできる最大ディレクトリ数                                             |
| PIPEFS_MAX_PIPE_NUM                | 最大パイプ数                                                       |
| RTC_DATETIME_SYNC_CYCLE            | RTCとカーネル時刻の同期計算周期(msec)                                      |
| RTC_RESOLUTION_IS_NOT_SEC          | RTCの解像度は秒より精細                                                |
| SHELL_MAX_COM_ARGV                 | shellコマンド最大引数の数                                              |
| SHELL_MAX_COM_HIS                  | shellコマンドヒストリの数                                              |
| SHELL_MAX_LINE_COLUMS              | shellコマンドラインの最大文字数                                           |
| SHELL_USER_COMMAND_NUM             | shellユーザコマンド登録可能数                                            |
| SHELL_USE_FWTEST                   | ファイル書き込みテストコマンド(fwtest)を有効にする                                |
| STM32CUBE_HAL_ARCH                 | STM32Cubeターゲット指定                                             |
| SYSTEM                             | システム名                                                        |
| TARGET_SYSTEM_32F469IDISCOVERY     | ターゲットシステムは32F469IDISCOVERY                                   |
| TARGET_SYSTEM_EMU                  | ターゲットシステムはエミュレータ                                             |
| TARGET_SYSTEM_NUCLEO_F411RE        | ターゲットシステムはNUCLEO_F411RE                                      |
| TARGET_SYSTEM_STM32F746GDISCOVERY  | ターゲットシステムは32F746GDISCOVERY                                   |
| TARGET_SYSTEM_STM32F769IDISCOVERY  | ターゲットシステムは32F769IDISCOVERY                                   |
| TCPIP_DEFAULT_DNSSERVER            | DNSサーバアドレス                                                   |
| TCPIP_DEFAULT_DNSSERVER2           | DNSサーバアドレス2                                                  |
| TCPIP_DEFAULT_GATEWAY              | TCP/IPデフォルトゲートウェイアドレス                                        |
| TCPIP_DEFAULT_IPADDR               | TCP/IPデフォルトIPアドレス                                            |
| TCPIP_DEFAULT_NETMASK              | TCP/IPデフォルトネットマスク                                            |
| TCPIP_DEFAULT_NTP_SERVERNAME       | デフォルトのNTPサーバ名                                                |
| TCPIP_ENABLE_DHCP                  | DHCPを有効にする                                                   |
| TCPIP_ENABLE_SNTP                  | SNTPを有効にする                                                   |
| TCPIP_ENABLE_START_SNTPINIT        | システム起動時にSNTPを有効にする                                           |
| TIMEZONE_STR                       | タイムゾーンを示す文字列                                                 |
| UISTYLEDEF                         | UIスタイル定義ヘッダファイル名                                             |
| UI_BUTTON_REPEAT_TIME              | UIボタン長押し時のリピートイベント発生までの時間(ms)                                |
| UI_NETWORK_CONFFILE                | ネットワークコンフィグ設定ファイル名                                           |
