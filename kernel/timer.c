/** @file
    @brief	カーネルタイマ

    GadgetSeed のカーネルタイマは KERNEL_TIMER_INTERVAL_MSEC(ms) 周期
    処理タイマデバイスを使用します。

    @date	2007.03.18
    @author	Takashi SHUDO

    @page kernel_timer カーネルタイマ

    GadgetSeedのカーネルタイマはシステム起動からの経過時間をミリ秒単位で示す64ビット長カウンタです。

    カーネルタイマに同期した周期処理関数を登録または登録解除することが出来ます。\n
    周期処理関数は非タスクコンテキストで実行されます。


    ---
    @section kernel_timer_api カーネルタイマAPI

    カーネルタイマに関する以下のAPIがあります。\n
    これらは timer.h で定義されています。

    include ファイル : timer.h

    | API名			| 機能					|
    |:--------------------------|:--------------------------------------|
    | get_kernel_time()		| @copybrief get_kernel_time		|
    | register_timer_func()	| @copybrief register_timer_func	|
    | unregister_timer_func()	| @copybrief unregister_timer_func	|


    ---
    @section system_time システムタイム

    GadgetSeedは @ref kernel_timer とは別にシステムタイム値を取得する機能があります。

    システムタイムはus(マイクロ秒)の精度を持つ64ビット長のカウンタです。\n
    システムタイムはカーネルタイマデバイスより取得します。\n
    そのためシステムタイム値はカーネルタイマデバイスが持つ必要があります。

    システムタイム取得のためのデバイスドライバAPIは[TBD]参照。

    @subsection system_time_api システムタイムAPI

    include ファイル : timer.h

    | API名			| 機能				|
    |:--------------------------|:------------------------------|
    | get_system_utime()	| @copybrief get_system_utime	|

    システムタイムはカーネルタイマより分解能が高いカウンタです。\n
    但し取得するためのAPI(@ref get_system_utime)はカーネルタイマデバイスにアクセスするために取得の処理時間は長くなります。\n
    分解能が必要でない場合、@ref get_kernel_time を使うことを推奨します。
*/

#include "sysconfig.h"
#include "timer.h"
#include "device/timer_ioctl.h"
#include "datetime.h"
#include "tkprintf.h"

//#define DEBUGKBITS 0x01
#include "dkprintf.h"

#ifndef GSC_KERNEL_MAX_KERNEL_TIMER_FUNC
#define GSC_KERNEL_MAX_KERNEL_TIMER_FUNC	8	///< $gsc カーネルタイマに登録できる最大定期処理関数数
#endif

unsigned long long kernel_time_count;	///< カーネル時間(ms)

static struct st_device *timer_dev;	///< カーネルタイマデバイス
static unsigned char flg_exec_func;	///< タイマ関数実行フラグ
static timer_func timer_func_list[GSC_KERNEL_MAX_KERNEL_TIMER_FUNC];	///< タイマ関数リスト
static unsigned long func_interval[GSC_KERNEL_MAX_KERNEL_TIMER_FUNC];	///< タイマ関数実行間隔
static unsigned long func_timecnt[GSC_KERNEL_MAX_KERNEL_TIMER_FUNC];	///< タイマ関数タイマ
static timer_func kernel_timer_func = 0;

/**
   @brief	タイマ関数の周期実行を開始する

   @return	!=0:エラー
*/
void start_timer_func(void)
{
	flg_exec_func = 1;
}

/**
   @brief	タイマ関数の周期実行を停止する

   @return	!=0:エラー
*/
void stop_timer_func(void)
{
	flg_exec_func = 0;
}

/*
   @brief	カーネルタイマ定期処理

   @param[in]	sp	割り込み時スタックポインタ
*/
static volatile int flg_timer_count = 1;

static void kernel_timer(void *sp)
{
	int i;

	if(flg_timer_count != 0) {
		flg_timer_count = 0;
	}

	kernel_time_count += GSC_KERNEL_TIMER_INTERVAL_MSEC;

	if(flg_exec_func == 0) {
		return;
	}

	for(i=0; i<GSC_KERNEL_MAX_KERNEL_TIMER_FUNC; i++) {
		if(timer_func_list[i]) {
			func_timecnt[i] += GSC_KERNEL_TIMER_INTERVAL_MSEC;
			if(func_timecnt[i] >= func_interval[i]) {
				//func_timecnt[i] = 0;
				func_timecnt[i] -= func_interval[i];
				timer_func_list[i](sp, kernel_time_count);
			}
		}
	}

	if(kernel_timer_func != 0) {
		kernel_timer_func(sp, kernel_time_count);
	}
}

/**
  @brief	カーネルタイマを初期化する

  @param[in]	devname	カーネルタイマデバイス名
*/

static int int_count = 0;

void init_timer(char *devname)
{
	int i;
	kernel_time_count = 0;

	stop_timer_func();

	for(i=0; i<GSC_KERNEL_MAX_KERNEL_TIMER_FUNC; i++) {
		timer_func_list[i] = 0;
	}
	kernel_timer_func = 0;

	timer_dev = open_device(devname);

	if(timer_dev == 0) {
		SYSERR_PRINT("Cannot open timer device.\n");
		goto err;
	}

	if(ioctl_device(timer_dev, IOCMD_TIMER_SETFUNC, 0, (void *)kernel_timer)) {
		SYSERR_PRINT("Cannot register timer function.\n");
		goto err;
	}

	if(ioctl_device(timer_dev, IOCMD_TIMER_START, 0, 0)) {
		SYSERR_PRINT("Cannot start system timer.\n");
		goto err;
	}

#ifndef GSC_TARGET_SYSTEM_EMU
	while(flg_timer_count) {
		int_count ++;
	}
#else
	int_count = 1000000; // [TODO] 適当な値
#endif

	DKPRINTF(0x01, "%d ms count = %d\n", GSC_KERNEL_TIMER_INTERVAL_MSEC, int_count);

err:
	return;
}

/**
   @brief	カーネル時間を取得する

   @return	カーネル時間(msec)
*/
unsigned long long get_kernel_time(void)
{
	return kernel_time_count + ioctl_device(timer_dev, IOCMD_TIMER_GETTIME, 0, 0);
}

/**
   @brief	システム時間を取得する

   @return	システム時間(usec)
*/
unsigned long long get_system_utime(void)
{
	unsigned long long utime;

	ioctl_device(timer_dev, IOCMD_TIMER_GETSYSTIME, 0, (void *)&utime);

	return utime;
}

/**
   @brief	指定時間待つ

   @param[in]	time	待ち時間(msec)
*/
void wait_time(unsigned int time)
{
	volatile unsigned long long t, nt;

	if(time == 0) {
		// 0なら即終了
		return;
	} else {
		// 最低 time 分はカウントするため +1
		nt = get_kernel_time() + time + 1;
	}

	while(1) {
		t = get_kernel_time();
		if(t < nt) {
			DKPRINTF(0x02, "kernel_time = %d\n", get_kernel_time());
		} else {
			break;
		}
	}
}

/**
   @brief	指定時間待つ

   @param[in]	time	待ち時間(usec)
*/
void wait_utime(unsigned int time)
{
	volatile unsigned int i, count;

	count = (time * int_count) / GSC_KERNEL_TIMER_INTERVAL_MSEC / 1000;

	DKFPRINTF(0x01, "timr = %d, count = %d\n", count);

	for(i=0; i<count; i++) {
		;
	}
}

/**
   @brief	カーネル周期処理を追加する

   @param[in]	func	周期処理関数
*/
void register_kernel_timer_func(timer_func func)
{
	kernel_timer_func = func;
}

/**
   @brief	周期処理を追加する

   @param[in]	func		周期処理関数
   @param[in]	interval	周期処理実行周期(msec)

   @return	!=0:エラー
*/
int register_timer_func(timer_func func, unsigned long interval)
{
	int i;

	for(i=0; i<GSC_KERNEL_MAX_KERNEL_TIMER_FUNC; i++) {
		if(timer_func_list[i] == 0) {
			timer_func_list[i] = func;
			func_interval[i] = interval;
			func_timecnt[i] = 0;
			start_timer_func();
			return 0;
		}
	}

	SYSERR_PRINT("Cannot register timer func (over GSC_KERNEL_MAX_KERNEL_TIMER_FUNC).\n");

	return -1;
}

/**
   @brief	周期処理を削除する

   @param[in]	func	周期処理関数

   @return	!=0:エラー
*/
int unregister_timer_func(timer_func func)
{
	int i;

	for(i=0; i<GSC_KERNEL_MAX_KERNEL_TIMER_FUNC; i++) {
		if(timer_func_list[i] == func) {
			timer_func_list[i] = 0;
			return 0;
		}
	}

	SYSERR_PRINT("Cannot unregister timer func.\n");

	return -1;
}

/**
   @brief	タイマを開始する

   @return	!=0:エラー
*/
int start_timer(void)
{
	int rt;

	rt = ioctl_device(timer_dev, IOCMD_TIMER_START, 0, 0);

	if(rt != 0) {
		return rt;
	}

#ifdef DEV_ENABLE_RTC
	rt = sync_time();
#endif
	return rt;
}

/**
   @brief	タイマを停止する

   @return	!=0:エラー
*/
int stop_timer(void)
{
	return ioctl_device(timer_dev, IOCMD_TIMER_STOP, 0, 0);
}
