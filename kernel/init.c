/** @file
    @brief	GadgetSeed カーネル初期化

    @date	2007.12.31
    @author	Takashi SHUDO
*/

#include "sysconfig.h"
#include "gadgetseed.h"
#include "system.h"
#include "device.h"
#include "interrupt.h"
#include "console.h"
#include "timer.h"
#include "tkprintf.h"
#include "memory.h"
#include "font.h"
#include "sysevent.h"
#include "log.h"
#include "device/timer_ioctl.h"
#include "task/task.h"
#include "task/syscall.h"
#include "task/task_opration.h"

#if 0 // 以下はコメントです
// 以下のマクロは sysconfig.h で定義させるために、 configs/systems/*.conf に GSC_ を付けずに記載して下さい
#define GSC_ARCH	"CPU_ARCITECTURE_NAME"	///< $gsc CPU アーキテクチャ名
#define GSC_CPUNAME	"CPU_NAME"		///< $gsc CPU 名
#define GSC_SYSTEM	"SYSTEM_NAME"		///< $gsc システム名
// CPU、システムによっては以下のマクロが有効です
#define GSC_CPU_CLOCK_HZ	200000000UL	///< $gsc CPUクロック周波数(Hz)
#endif

#ifndef GSC_KERNEL_INITIALTASK_STACK_SIZE	///< $gsc カーネル初期化タスクのスタックサイズ
#ifdef GSC_TARGET_SYSTEM_EMU	// $gsc ターゲットシステムはエミュレータ
#define GSC_KERNEL_INITIALTASK_STACK_SIZE	(1024*4)
#else
#define GSC_KERNEL_INITIALTASK_STACK_SIZE	(1024*4)
#endif
#endif

#define xstr(s) str(s)
#define str(s) #s

const char arch_name[] = xstr(GSC_ARCH);
const char cpu_name[] = xstr(GSC_CPUNAME);
const char system_name[] = xstr(GSC_SYSTEM);

extern struct st_device GSC_KERNEL_ERROUT_DEVICE;	///< $gsc エラーメッセージ出力デバイス
extern struct st_device GSC_KERNEL_TIMER_DEVICE;	///< $gsc カーネルタイマデバイス
extern struct st_device GSC_KERNEL_MESSAGEOUT_DEVICE;	///< $gsc カーネルメッセージ出力デバイス
#ifdef GSC_KERNEL_MESSAGEOUT_LOG
extern struct st_device logout_device;
extern struct st_device logbuf_device;
#endif

extern const char hal_ver[];

// カーネルメッセージ出力デバイス初期化
static void register_kernel_message_out_device(void)
{
	// sysconfig.h
#ifdef GSC_KERNEL_MESSAGEOUT_LOG
	(void)register_device(&GSC_KERNEL_ERROUT_DEVICE, 0);
	(void)register_device(&logout_device, "debug");
	(void)register_device(&logbuf_device, 0);
	(void)register_kmess_out_dev(&logout_device);
	(void)register_kmess_log_dev(&logbuf_device);
#else
	(void)register_device(&GSC_KERNEL_ERROUT_DEVICE, 0);
	(void)register_kmess_out_dev(&GSC_KERNEL_MESSAGEOUT_DEVICE);
#endif
}

void display_bunner(void)
{
	extern const char os_version[];
	extern const char build_date[];
	extern const char build_time[];

	tkprintf(NORMAL_COLOR "\n");
	tkprintf("GadgetSeed Ver. %s\n", os_version);
	tkprintf("(c)2010-2020 Takashi SHUDO\n");
	tkprintf("CPU ARCH     : %s\n", arch_name);
	tkprintf("CPU NAME     : %s\n", cpu_name);
	tkprintf("SYSTEM       : %s\n", system_name);
	tkprintf("Build date   : %s %s\n", build_time, build_date);
	tkprintf("Compiler     : %s\n", __VERSION__);
#ifndef GSC_TARGET_SYSTEM_EMU
	tkprintf("STM32Cube HAL: %s\n", hal_ver);
#endif
}

extern void init_sect(void);
extern int initial_task(void *arg);

static struct st_tcb init_task_tcb;

static unsigned int init_task_stack[GSC_KERNEL_INITIALTASK_STACK_SIZE/sizeof(unsigned int)]  ATTR_STACK;

int flg_init_task_run = 0;

extern void startup(void);

/**
   @brief	イニシャルタスク

   @param[in]	arg	引数
*/
int initial_task(void *arg)
{
	flg_init_task_run = 1;

	startup();

	//while(1) {
	//	;
	//}

	return 0;
}


/**
   @brief	カーネルを初期化する

   @param[in]	argc	引数の数
   @param[in]	argv	引数の文字列
*/
void init_gs(int *argc, char ***argv)
{
	/*
	  ドライバ初期化前に必要なシステム固有初期化処理
	*/
	init_system(argc, argv);

	/*
	  カーネルAPI(SYSCALL)ジャンプテーブル初期化
	*/
#ifndef LINT
	disable_interrupt();
	init_interrupt_vector();	// 割り込みベクタテーブル初期化
	enable_interrupt();
#endif

	init_device_list();

#ifdef GSC_COMP_ENABLE_FONTS	/// $gsc 文字フォント表示を有効にする
	init_font();
#endif

	// カーネルタイマドライバ初期化
	register_device(&GSC_KERNEL_TIMER_DEVICE, 0);	// sysconfig.h

	// システム初期化
	init_system2();

	// コンソール初期化
	init_console_device();
	register_kernel_message_out_device();

	// バナー表示
	display_bunner();

	// カーネルタイマスタート
	init_timer(DEF_DEV_NAME_TIMER);

	/*
	  メモリリソース初期化、以後 memory_alloc() 使用可能
	*/
#ifdef GSC_MEMORY_ENABLE_HEAP_MEMORY	/// $gsc ヒープメモリを有効にする
	init_memory();
	display_memory_info();
#endif

	/*
	  マルチタスク制御初期化
	*/
	init_task();

	/*
	  システムコール割り込みハンドラ登録
	*/
	register_interrupt(INTNUM_SYSCALL, syscall_inthdr);

	/*
	  カーネルタイマスケジューラ登録
	*/
	register_kernel_timer_func(task_schedule);

	init_event();

	/*
	  ここまでが最小限の初期化
	  以後、各システム依存の初期化を行う
	*/

	/*
	  システム固有のドライバ登録
	*/
	init_system_drivers();

	/*
	  初期タスク起動
	*/
	task_exec(initial_task, "init", TASK_PRIORITY_APP_LOW, &init_task_tcb,
		  init_task_stack, GSC_KERNEL_INITIALTASK_STACK_SIZE, 0);
}
