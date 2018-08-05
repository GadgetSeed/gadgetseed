/** @file
    @brief	コンソールIO

    GadgetSeed はバイト入出力をもつデバイスドライバをコンソールとして
    使用することが出来ます。

    コンソール出力には tprintf() 関数でフォーマットされた文字列を出力ます。

    @date	2007.03.17
    @author	Takashi SHUDO

    @page console_io コンソールIO

    GadgetSeedは標準化されたコンソールIOがあります。

    コンソールIOはUNIX系OSの標準入出力に似ています。

    コンソールIOは「標準入力」、「標準出力」、「エラー出力」の３種類があります。\n
    これはらの入出力には何らかのデバイスドライバを割り当てることができます。\n
    また、入出力デバイスはタスク毎に固有のデバイスを割り当てることができます。

    コンソールデバイスにはUARTデバイスを使うことを推奨します。\n
    また、コンソールデバイスは @ref command_shell のインタフェースとして使用されます。


    ---
    @section console_api コンソールIO API

    @subsection standard_in_out_device_register_api 標準入出力デバイス登録API

    以下のAPIはシステムの標準入出力デバイスを登録します。\n
    これらのAPIで登録されたデバイスは生成されたタスクの標準入出力デバイスとなります。\n
    これらのAPIはシステムの初期化時に使用して下さい。\n
    コマンドシェルを使用する場合は、必ず何らかのデバイスを登録する必要があります。

    include ファイル : console.h

    | API名			| 機能					|
    |:--------------------------|:--------------------------------------|
    | register_console_in_dev()	| @copybrief register_console_in_dev	|
    | register_console_out_dev()| @copybrief register_console_out_dev	|
    | register_error_out_dev()	| @copybrief register_error_out_dev	|

    @subsection strings_out_api 文字列出力API

    GadgetSeedはprintf()に似た書式付きの文字列出力APIがあります。

    include ファイル : tprintf.h

    | API名			| 機能			|
    |:--------------------------|:----------------------|
    | tprintf()			| @copybrief tprintf	|
    | eprintf()			| @copybrief eprintf	|

    tprintf() は、 register_console_out_dev() で登録されたデバイスに出力されます。\n
    eprintf() は、 register_error_out_dev() で登録されたデバイスに出力されます。

    @subsection strings_in_api コンソール入力API

    標準入力から入力データを取得するには以下のAPIを使用します。

    include ファイル : console.h

    | API名			| 機能			|
    |:--------------------------|:----------------------|
    | cgets()			| @copybrief cgets	|
    | cgetc()			| @copybrief cgetc	|
    | cwait()			| @copybrief cwait	|
    | cgetcnw()			| @copybrief cgetcnw	|

    @subsection standard_in_out_device_change_api 標準入出力デバイス変更API

    実行中のタスクは、標準入出力デバイスは動的に変更することができます。\n
    標準入出力デバイスを変更するには、以下のAPIを使用します。

    include ファイル : syscall.h

    | API名			| 機能					|
    |:--------------------------|:--------------------------------------|
    | set_console_in_device()	| @copybrief set_console_in_device	|
    | set_console_out_device()	| @copybrief set_console_out_device	|
    | set_error_out_device()	| @copybrief set_error_out_device	|
*/

#include "console.h"
#include "tprintf.h"
#include "task/tcb.h"

struct st_device *con_in_dev;	///< デフォルト標準入力デバイス
struct st_device *con_out_dev;	///< デフォルト標準出力デバイス
struct st_device *con_err_dev;	///< デフォルトエラー出力デバイス

extern struct st_tcb *run_task;

/**
   @brief	システム標準のコンソール入力デバイスを登録する

   @param[in]	in_dev	コンソール入力デバイス
*/
void register_console_in_dev(const struct st_device *in_dev)
{
	con_in_dev = (struct st_device *)in_dev;
}

/**
   @brief	システム標準のコンソール出力デバイスを登録する

   @param[in]	out_dev	コンソール出力デバイス
*/
void register_console_out_dev(const struct st_device *out_dev)
{
	con_out_dev = (struct st_device *)out_dev;
}

/**
   @brief	全てのコンソール入出力デバイスを初期化する
*/
void init_console_device(void)
{
	con_out_dev = 0;
	con_in_dev = 0;
	con_err_dev = 0;
}

/*
 * コンソールI/O関数
 */

/**
   @brief	標準出力より文字列を出力する

   @param[in]	str	文字列ポインタ
   @param[in]	len	文字列のバイト数

   @return	出力した文字数
*/
int cputs(unsigned char *str, unsigned int len)
{
	return write_device(run_task->stdout_dev, str, len);
}

/**
   @brief	標準出力より1文字を出力する

   @param[in]	td	文字

   @return	出力した文字数
*/
int cputc(unsigned char td)
{
	return putc_device(run_task->stdout_dev, td);
}

/**
   @brief	標準入力より文字列を取得する

   @param[in]	rd	入力文字ポインタ

   @return	!=0:取得文字有り
*/
int cgets(unsigned char *str, unsigned int count)
{
	int rt;

	if(run_task->stdin_dev != 0) {
		(void)select_device(run_task->stdin_dev, 0);
		rt = read_device(run_task->stdin_dev, str, count);
		if(rt != 0) {
			return rt;
		}
	}

	return 0;
}

/**
   @brief	標準入力より1文字を取得する

   @param[out]	rd	取得文字ポインタ

   @return	!=0:入力文字有り
*/
int cgetc(unsigned char *rd)
{
	int rt;

	if(run_task->stdin_dev != 0) {
		(void)select_device(run_task->stdin_dev, 0);
		rt = getc_device(run_task->stdin_dev, rd);
		if(rt != 0) {
			return rt;
		}
	}

	return 0;
}

/**
   @brief	標準入力より入力を待つ

   @param[in]	timeout	タイムアウト時間(ms)

   @return	!=0:入力文字有り
*/
int cwait(unsigned int timeout)
{
	if(run_task->stdin_dev != 0) {
		return select_device(run_task->stdin_dev, timeout);
	}

	return 0;
}

/**
   @brief	標準入力より1文字を取得する(待ち無し)

   @param[out]	rd	取得文字ポインタ

   @return	!=0:取得文字有り
*/
int cgetcnw(unsigned char *rd)
{
	int rt;

	if(run_task->stdin_dev != 0) {
		rt = getc_device(run_task->stdin_dev, rd);
		if(rt != 0) {
			return rt;
		}
	}

	return 0;
}


/**
   @brief	システム標準のエラー出力デバイスを登録する

   @param[in]	err_dev	エラー出力デバイス
*/
void register_error_out_dev(const struct st_device *err_dev)
{
	con_err_dev = (struct st_device *)err_dev;
}


/**
   @brief	エラー出力より文字列を出力する

   @param[in]	str	文字列ポインタ
   @param[in]	len	文字列のバイト数
*/
int eputs(unsigned char *str, unsigned int len)
{
	return write_device(run_task->error_dev, str, len);
}


/**
   @brief	標準入力デバイスを設定する

   @param[in]	dev	デバイス
*/
void set_console_in_device_ISR(struct st_device *dev)
{
	if(dev != 0) {
		run_task->stdin_dev = dev;
	} else {
		run_task->stdin_dev = con_in_dev;
	}
}


/**
   @brief	標準出力デバイスを設定する

   @param[in]	dev	デバイス
*/
void set_console_out_device_ISR(struct st_device *dev)
{
	if(dev != 0) {
		run_task->stdout_dev = dev;
	} else {
		run_task->stdout_dev = con_out_dev;
	}
}


/**
   @brief	エラー出力デバイスを設定する

   @param[in]	dev	デバイス
*/
void set_error_out_device_ISR(struct st_device *dev)
{
	if(dev != 0) {
		run_task->error_dev = dev;
	} else {
		run_task->error_dev = con_err_dev;
	}
}
