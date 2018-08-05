/** @file
    @brief	GadgetSeed メイン

    @date	2009.01.10
    @author	Takashi SHUDO

    @mainpage GadgetSeedについて

    GadgetSeedは組み込み機器向けのマルチタスクOSです。

    GadgetSeedは現在、以下のハードウェアで動作します。

    | ハードウェア	| MCU		| アーキテクチャ |
    |-------------------|:--------------|:---------------|
    | 32F769IDISCOVERY	| STM32F769NIH6 | ARM Cortex-M7  |
    | 32F746GDISCOVERY	| STM32F746NGH6 | ARM Cortex-M7  |
    | NUCLEO-F411RE	| STM32F401RET6 | ARM Cortex-M4  |

    ---
    @section feature 特徴

    GadgetSeedは以下の機能があります。

    - マルチタスクカーネル - @ref task_syscall 参照
    - デバッグを支援するコマンドシェル - @ref command_shell 参照
    - 標準化されたデバイスドライバAPI - @ref device_driver 参照
    - メモリ管理 - @ref memory_manage 参照
    - 日付時刻API - @ref date_time 参照
    - 標準化されたシステムイベント - @ref system_event 参照
    - グラフィックス描画API - @ref graphics 参照
    - 文字フォント描画API - @ref font_draw 参照
    - ファイルシステムに FatFS 対応 - @ref file_system 参照
    - LwIP TCP/IPプロトコルスタックに対応 - @ref network 参照

    @page configration コンフィグレーション

    GadgetSeed をビルドするには、最初にコンフィグレーションを行う必要があります。

    コンフィグレーションとは GadgetSeed のソースツリーのルートディレクトリに以下のファイルを作成することです。

    ```
    config.mk
    ```

    config.mk は GadgetSeed を実行するハードウェア(システム)と組み込まれるアプリケーションが記載されています。

    config.mk は make 実行でメニューよりシステム及びアプリケーションを選択することで作成することができます。

    以下に、 config.mk の例を示します。

    ```
    $ cat config.mk
    SYSTEM_CONFIG = 32F769IDISCOVERY
    APPLICATION_CONFIG = musicplay_hr
    ```

    config.mk は以下の2つの項目を記載します。

    | 項目			| 内容				|
    |:--------------------------|:------------------------------|
    | SYSTEM_CONFIG		| 動作させるシステム		|
    | APPLICATION_CONFIG	| 実行するアプリケーション	|


    ---
    @section system_config システムコンフィグ

    SYSTEM_CONFIG には、ソースツリーの以下のディレクトリにあるファイルより、１つのファイル名を拡張子(.conf)を除いて記述します。

    ```
    configs/systems
    ```

    上記のディレクトリの各ファイルには、各システムに対応したCPUアーキテクチャや、デバイスドライバ等が記載されています。


    ---
    @section apprication_config アプリケーションコンフィグ

    APPLICATION_CONFIG には、ソースツリーの以下のディレクトリにあるファイルより、１つのファイル名を拡張子(.conf)を除いて記述します。

    ```
    configs
    ```

    上記のディレクトリの各ファイルには、各アプリケーションが必要とする GadgetSeed のコンポーネントやライブラリ等が記載されています。


    ---
    @section prepare_build ビルドの準備

    config.mk ファイル作成後 make コマンドを実行すると以下のファイルが作成されます。

    | ファイル名		| 内容									|
    |:--------------------------|:----------------------------------------------------------------------|
    | include/sysconfig.h	| GadgetSeed コンフィグレーションの為のC言語マクロ定義ヘッダ		|
    | include/asm.h		| GadgetSeed ターゲットアーキテクチャのヘッダファイルシンボリックリンク	|
    | targetconfig.mk		| GadgetSeed ビルドのための Makefile インクルードファイル		|


    ---
    @section config_item コンフィグレーション項目

    GadgetSeed 各ソースファイルのコンフィグレーション項目は include/sysconfig.h に記載されたC言語マクロ定義により設定されます。
    各コンフィグレーションマクロには接頭詞 GSC_ が付きます。
    configs/\*.conf および configs/systems/\*.conf ファイルの、各コンフィグレーション項目には GSC_ は記載しません。

    以下のコマンドで GadgetSeed で定義されているコンフィグレーション項目の一覧を表示することができます。

    ```
    sh tools/configlist.sh
    ```
*/

#include "sysconfig.h"
#include "gadgetseed.h"
#include "system.h"
#include "shell.h"
#include "task/syscall.h"
#include "datetime.h"
#include "tkprintf.h"

#ifdef GSC_COMP_ENABLE_TCPIP
#include "net.h"
#endif

extern void GSC_APP_STARTUP();
extern void GSC_APP_STARTUP2();
extern void GSC_APP_STARTUP3();
extern void GSC_APP_STARTUP4();

int main(int argc, char *argv[])
{
	init_gs(&argc, &argv);

	return 0;
}

/**
   @brief	タスクコンテキストで実行される初期化処理
*/
void startup(void)
{
	init_system_process();

#ifdef GSC_COMP_ENABLE_TCPIP	/// $gsc TCP/IPネットワークを有効にする
	startup_network();
#endif

#ifdef GSC_COMP_ENABLE_SHELL	/// $gsc コマンドシェルを有効にする
	startup_shell();
#endif

#ifdef GSC_APP_STARTUP		/// $gsc アプリケション起動関数名
	GSC_APP_STARTUP();
#endif

#ifdef GSC_APP_STARTUP2		/// $gsc アプリケション起動関数名(2番めに起動)
	GSC_APP_STARTUP2();
#endif

#ifdef GSC_APP_STARTUP3		/// $gsc アプリケション起動関数名(3番めに起動)
	GSC_APP_STARTUP3();
#endif

#ifdef GSC_APP_STARTUP4		/// $gsc アプリケション起動関数名(4番めに起動)
	GSC_APP_STARTUP4();
#endif

#ifdef GSC_DEV_ENABLE_RTC
#ifdef GSC_RTC_DATETIME_SYNC_CYCLE	/// $gsc RTCとカーネル時刻の同期計算周期(msec)
	while(1) {
		// 定期的にRTCからシステム時刻を同期させる
		task_sleep(GSC_RTC_DATETIME_SYNC_CYCLE);
		adjust_systime();
	}
#endif
#endif
}
