/** @file
    @brief	タスク制御API

    これらの関数はタスク状態から実行されなければならない

    @date	2017.11.25
    @auther	Takashi SHUDO

    @page task_syscall	タスク制御

    GadgetSeedのカーネルはマルチタスクカーネルです。\n
    GedgetSeedはタスク間同期の機能として以下の機能があります。

    - @ref event
    - @ref mutex

    @section task_status タスク状態

    GadgetSeedの各タスクは以下のように状態遷移します。

    ![タスク状態](task_status.svg)

    | 状態名		| 状態				|
    |:------------------|:------------------------------|
    | READY		| 実行可能状態			|
    | RUN		| 実行状態			|
    | TIMER_WAIT	| タイマ待ち状態		|
    | EVENT_WAIT	| イベント待ち状態		|
    | MUTEX_WAIT	| MUTEXロック解除待ち状態	|
    | REQUEST_WAIT	| 起床待ち状態			|
    | DORMANT_WAIT	| 休止状態			|


    ---
    @section task_context タスクコンテキスト

    タスクコンテキストは以下の構造体で定義されます。

    @ref st_tcb @copybrief st_tcb

    ユーザが作成する各タスクは、スタティックに st_tcb 構造のデータを定義する必要があります。

    タスクには名前をつけることができます。

    タスクには優先順位があります。タスク優先順位デフォルトでは0(最高)から3(最低)の4段階を設定することができます。
    タスク優先順位の数はコンフィグレーションマクロ GSC_KERNEL_MAX_TASK_PRIORITY を定義することにより変更することができます。

    各タスクのスタックメモリは、各タスク毎にスタティックに定義する必要があります。

    タスクの実行例として、 init_gs() で実行している task_exec() の実行例を参照して下さい。

    タスク用APIは @ref task_api を参照して下さい。


    ---
    @section event イベント

    GadgetSeed は一般的な RTOS のイベントフラグのような同期機構としてイベントがあります。

    @ref event_api は、タスク間および、タスクと非タスクコンテキスト(割り込みハンドラ等)間の同期を行うために使用できます。

    @ref not_task_event_api はイベントの発生通知のみ行うことができます。

    例えば、タスクで event_wait() を実行し、IO割り込みハンドラで event_wakeup_ISR() を実行し、タスクでIOのデータの受信を待ちます。
    この場合、event_wait() を実行したタスクは、IO割り込みハンドラで event_wakeup_ISR() が実行されるまで「イベント待ち状態」になります。

    イベントを使用するには、以下に示すイベント構造体を定義し、
    eventqueue_register() 、 eventqueue_register_ISR() 関数でイベント
    構造体データをシステムに登録する必要があります。

    @ref st_event @copybrief st_event

    イベント用APIは @ref event_api を参照して下さい。


    ---
    @section mutex MUTEX

    GadgetSeed はリソースの排他機構 MUTEX があります。

    MUTEX API はタスクからのみ実行できます。

    MUTEX をロックしたタクスのみが、 MUTEX をアンロックすることができます。

    MUTEX を使用するには、以下に示す MUTEX 構造体を定義し、
    mutex_register() 、 mutex_register_ISR() 関数で MUTEX 構造体データ
    をシステムに登録する必要があります。

    @ref st_mutex @copybrief st_mutex

    MUTEX 用APIは @ref mutex_api を参照して下さい。


    ---
    @section task_api タスク制御API

    include ファイル : syscall.h

    | API名			| 機能				|
    |:--------------------------|:------------------------------|
    | task_add()		| @copybrief task_add		|
    | task_exec()		| @copybrief task_exec		|
    | task_exit()		| @copybrief task_exit		|
    | task_pause()		| @copybrief task_pause		|
    | task_sleep()		| @copybrief task_sleep		|
    | task_kill()		| @copybrief task_kill		|
    | task_wakeup()		| @copybrief task_wakeup	|
    | task_wakeup_id_ISR()	| @copybrief task_wakeup_id_ISR	|


    ---
    @section event_api イベントAPI

    @subsection task_event_api タスクコンテキスト用

    include ファイル : syscall.h

    | API名			| 機能					|
    |:--------------------------|:--------------------------------------|
    | eventqueue_register()	| @copybrief eventqueue_register	|
    | event_wait()		| @copybrief event_wait			|
    | event_check()		| @copybrief event_check		|
    | event_clear()		| @copybrief event_clear		|
    | event_wakeup()		| @copybrief event_wakeup		|
    | eventqueue_unregister()	| @copybrief eventqueue_unregister	|

    @subsection not_task_event_api 非タスクコンテキスト用(システム初期化時/割り込みハンドラ用)

    include ファイル : task/event.h

    | API名			| 機能					|
    |:--------------------------|:--------------------------------------|
    | eventqueue_register_ISR()	| @copybrief eventqueue_register_ISR	|
    | event_push_ISR()		| @copybrief event_push_ISR		|
    | event_set_ISR()		| @copybrief event_set_ISR		|
    | event_wakeup_ISR()	| @copybrief event_wakeup_ISR		|
    | eventqueue_unregister_ISR() | @copybrief eventqueue_unregister_ISR|


    ---
    @section mutex_api MUTEX API

    @subsection task_mutex_api タスクコンテキスト用

    include ファイル : syscall.h

    | API名			| 機能				|
    |:--------------------------|:------------------------------|
    | mutex_register()		| @copybrief mutex_register	|
    | mutex_lock()		| @copybrief mutex_lock		|
    | mutex_unlock()		| @copybrief mutex_unlock	|
    | mutex_unregister()	| @copybrief mutex_unregister	|

    @subsection not_task_mutex_api 非タスクコンテキスト用(システム初期化時/割り込みハンドラ用)

    include ファイル : task/mutex.h

    | API名			| 機能					|
    |:--------------------------|:--------------------------------------|
    | mutex_register_ISR()	| @copybrief mutex_register_ISR		|
    | mutex_unregister_ISR()	| @copybrief mutex_unregister_ISR	|
*/

#include "syscall.h"
#include "syscall_param.h"
#include "task_opration.h"

//#define DEBUGKBITS 0x01
#include "dkprintf.h"


/**
   @brief	タスクを追加する

   本関数は、タスクを実行キューに追加するのみであり、追加されたタスク
   は実行状態にならない。

   @param[in]	func		タスク関数
   @param[in]	name		タスク名文字列ポインタ
   @param[in]	priority	タスク優先度
   @param[in]	tcb		タスクコンテキストポインタ
   @param[in]	stack		タスクスタックポインタ
   @param[in]	stack_size	タスクスタックサイズ
   @param[in]	arg		タスク実行時引数文字列ポインタ

   @return	!=0:エラー
*/
int task_add(task_func func, char *name, int priority, struct st_tcb *tcb,
	     unsigned int *stack, int stack_size, char *arg)
{
	volatile struct exec_task_param param;

	DKFPRINTF(0x01, "\n");

	if(priority >= GSC_KERNEL_MAX_TASK_PRIORITY) {
		return -1;
	}

	param.func	= func;
	param.name	= name;
	param.priority	= priority;
	param.tcb	= tcb;
	param.stack	= stack;
	param.stack_size= stack_size;;
	param.arg	= arg;

	DKPRINTF(0x01, "exec param = %p\n", &param);
	DKPRINTF(0x01, "exec func = %p\n", param.func);
	DKPRINTF(0x01, "exec name = \"%s\"\n", param.name);

	sys_call(SYSCALL_TASK_ADD, (void *)&param);

	return param.ret;
}

/**
   @brief	タスクを追加し起動する

   タスクを実行キューに追加し、追加したタスクを実行状態にする。

   @param[in]	func		タスク関数
   @param[in]	name		タスク名文字列ポインタ
   @param[in]	priority	タスク優先度
   @param[in]	tcb		タスクコンテキストポインタ
   @param[in]	stack		タスクスタックポインタ
   @param[in]	stack_size	タスクスタックサイズ
   @param[in]	arg		タスク実行時引数文字列ポインタ

   @return	!=0:エラー
*/
int task_exec(task_func func, char *name, int priority, struct st_tcb *tcb,
		 unsigned int *stack, int stack_size, char *arg)
{
	volatile struct exec_task_param param;

	DKFPRINTF(0x01, "\n");

	if(priority >= GSC_KERNEL_MAX_TASK_PRIORITY) {
		return -1;
	}

	param.func	= func;
	param.name	= name;
	param.priority	= priority;
	param.tcb	= tcb;
	param.stack	= stack;
	param.stack_size= stack_size;;
	param.arg	= arg;

	DKPRINTF(0x01, "exec param = %p\n", &param);
	DKPRINTF(0x01, "exec func = %p\n", param.func);
	DKPRINTF(0x01, "exec name = \"%s\"\n", param.name);

	sys_call(SYSCALL_TASK_EXEC, (void *)&param);

	return param.ret;
}

/**
   @brief	タスクを終了する

   本関数を実行したタスクは終了し、実行キューから削除される。
*/
void task_exit(void)
{
	DKFPRINTF(0x01, "\n");

	sys_call(SYSCALL_TASK_EXIT, (void *)0);
}

/**
   @brief	タスクを停止する

   本関数を実行したタスクは停止する。
*/
void task_pause(void)
{
	DKFPRINTF(0x01, "\n");

	sys_call(SYSCALL_TASK_PAUSE, (void *)0);
}

/**
   @brief	タスクを指定時間タイマ待ち状態にする

   本関数を実行したタスクを stime (msec)休止する。

   @param[in]	stime		停止時間(msec)
*/
void task_sleep(unsigned int stime)
{
	DKFPRINTF(0x01, "stime = %ld\n", stime);

	sys_call(SYSCALL_TASK_SLEEP, (void *)(long)stime);
}

/**
   @brief	指定したタスクを終了する

   id で指定したタスクを終了する。

   @param[in]	id		タスクID
*/
void task_kill(int id)
{
	DKFPRINTF(0x01, "id = %d\n", id);

	sys_call(SYSCALL_TASK_KILL, (void *)(long)id);
}

/**
   @brief	指定したタスクを実行状態にする

   id で指定したタスクを実行状態にする。

   @param[in]	id		タスクID
*/
void task_wakeup(int id)
{
	DKFPRINTF(0x01, "id = %d\n", id);

	sys_call(SYSCALL_TASK_WAKEUP, (void *)(long)id);
}


/*
 * イベント
 */

/**
   @brief	イベントキューを登録する

   @param[in]	evtque		イベントキューポインタ
   @param[in]	name		イベントキュー名文字列ポインタ
   @param[in]	args		イベントキュー引数バッファポインタ
   @param[in]	size		1イベント引数のサイズ
   @param[in]	count		キューするイベント数
*/
void eventqueue_register(struct st_event *evtque, const char *name, void *args, unsigned int size, int count)
{
	volatile struct evtque_param param;

	DKFPRINTF(0x01, "evtque = \"%s\" %p size = %d, count = %d\n",
		 name, evtque, size, count);

	param.evtque = evtque;
	param.name = name;
	param.arg = args;
	param.size = size;
	param.count = count;

	sys_call(SYSCALL_EVTQUE_INIT, (void *)&param);
}

/**
   @brief	タスクをイベント待ち状態にする

   @param[in]	evtque		イベントキューポインタ
   @param[out]	args		イベント引数ポインタ
   @param[in]	timeout		イベントタイムアウト待ち時間(msec)

   @return	待ちタイムアウト残り時間(msec)(=0:タイムアウト)
*/
int event_wait(struct st_event *evtque, void *arg, unsigned int timeout)
{
	volatile struct evtque_param param;

	DKFPRINTF(0x01, "evtque = \"%s\" %p timeout = %ld\n",
		 evtque->name, evtque, timeout);

	param.evtque = evtque;
	param.timeout = timeout;
	param.arg = arg;
	param.ret = 0;	// タイムアウト時は0を返す為

	DKPRINTF(0x01, "exec param = %p\n", &param);
	DKPRINTF(0x01, "evtque = %p\n", param.evtque);
	DKPRINTF(0x01, "arg = %p\n", param.arg);
	DKPRINTF(0x01, "timeout = %ld\n", (unsigned int)param.timeout);
	KXDUMP(0x02, arg, evtque->size);

	if(run_task == &dummy_task) {
#ifndef GSC_TARGET_SYSTEM_EMU
		SYSERR_PRINT("No running task\n");
		print_queues();
#endif
		return 0;
	}

	sys_call(SYSCALL_EVTQUE_WAIT, (void *)&param);

	return param.ret;
}

/**
   @brief	イベントキューにイベントが登録されているか調べる

   @param[in]	evtque		イベントキューポインタ

   @return	登録されているイベント数(=0:イベント未登録)
*/
int event_check(struct st_event *evtque)
{
	volatile struct evtque_param param;

	DKFPRINTF(0x01, "evtque = \"%s\" %p\n",
		 evtque->name, evtque);

	param.evtque = evtque;
	param.ret = 0;

	if(run_task == &dummy_task) {
#ifndef GSC_TARGET_SYSTEM_EMU
		SYSERR_PRINT("No running task\n");
		print_queues();
#endif
		return 0;
	}

	sys_call(SYSCALL_EVTQUE_CHECK, (void *)&param);

	return param.ret;
}

/**
   @brief	イベントキューに登録されているイベントを削除する

   @param[in]	evtque		イベントキューポインタ
*/
void event_clear(struct st_event *evtque)
{
	DKFPRINTF(0x01, "evtque = \"%s\" %p\n", evtque->name, evtque);

	if(run_task == &dummy_task) {
#ifndef GSC_TARGET_SYSTEM_EMU
		SYSERR_PRINT("No running task\n");
		print_queues();
#endif
		return;
	}

	sys_call(SYSCALL_EVTQUE_CLEAR, (void *)evtque);
}

/**
   @brief	イベントキューにイベントを登録する

   このイベントキューでイベント待ちのタスクは起床する

   @param[in]	evtque		イベントキューポインタ
   @param[in]	arg		イベント引数ポインタ
*/
void event_wakeup(struct st_event *evtque, void *arg)
{
	volatile struct evtque_param param;

	DKFPRINTF(0x01, "evtque = \"%s\" %p\n", evtque->name, evtque);
	KXDUMP(0x02, arg, evtque->size);

	param.evtque = evtque;
	param.arg = arg;

	if(run_task == &dummy_task) {
#ifndef GSC_TARGET_SYSTEM_EMU
		SYSERR_PRINT("No running task\n");
		print_queues();
#endif
		return;
	}

	sys_call(SYSCALL_EVTQUE_WAKEUP, (void *)&param);
}

/**
   @brief	イベントキューを登録解除する

   @param[in]	evtque		イベントキューポインタ
*/
void eventqueue_unregister(struct st_event *evtque)
{
	DKFPRINTF(0x01, "evtque = \"%s\" %p\n", evtque->name, evtque);

	sys_call(SYSCALL_EVTQUE_DISPOSE, (void *)evtque);
}


/*
 * MUTEX
 */

/**
   @brief	MUTEXを登録する

   @param[in]	mutex		MUTEXポインタ
   @param[in]	name		MUTEX名文字列ポインタ
*/
void mutex_register(struct st_mutex *mutex, const char *name)
{
	volatile struct mutex_param param;

	DKFPRINTF(0x01, "%s mutex = \"%s\" %p\n", name, mutex);

	param.mutex = mutex;
	param.name = name;

	sys_call(SYSCALL_MUTEX_INIT, (void *)&param);
}

/**
   @brief	MUTEXをロックする

   @param[in]	mutex		MUTEXポインタ
   @param[in]	timeout		タイムアウト時間(msec)(=0:タイムアウト無し)

   @return	待ちタイムアウト残り時間(msec)(=0:タイムアウト)
*/
int mutex_lock(struct st_mutex *mutex, unsigned int timeout)
{
	volatile struct mutex_param param;

	DKFPRINTF(0x01, "mutex = \"%s\" %p timeout = %ld\n",
		 mutex->name, mutex, timeout);

	param.mutex = mutex;
	param.timeout = timeout;

	DKPRINTF(0x01, "exec param = %p\n", &param);
	DKPRINTF(0x01, "mutex = %p\n", param.mutex);
	DKPRINTF(0x01, "timeout = %08lX\n", (unsigned int)param.timeout);

	sys_call(SYSCALL_MUTEX_LOCK, (void *)&param);

	return param.ret;
}

/**
   @brief	MUTEXをアンロックする

   @param[in]	mutex		MUTEXポインタ

   @return	待ちタイムアウト残り時間(msec)(=0:タイムアウト指定のないMUTEX)
*/
int mutex_unlock(struct st_mutex *mutex)
{
	volatile struct mutex_param param;

	DKFPRINTF(0x01, "mutex = \"%s\" %p\n", mutex->name, mutex);

	param.mutex = mutex;

	DKPRINTF(0x01, "exec param = %p\n", &param);
	DKPRINTF(0x01, "mutex = %p\n", param.mutex);

	sys_call(SYSCALL_MUTEX_UNLOCK, (void *)&param);

	return param.ret;
}

/**
   @brief	MUTEXを登録解除する

   @param[in]	mutex		MUTEXポインタ
*/
void mutex_unregister(struct st_mutex *mutex)
{
	DKFPRINTF(0x01, "mutex = \"%s\" %p\n", mutex->name, mutex);

	sys_call(SYSCALL_MUTEX_DISPOSE, (void *)mutex);
}

/*
  コンソールIO設定
 */

/**
   @brief	実行タスクの標準入力デバイスを設定する

   @param[in]	dev	デバイス

   @remarks	dev が 0 の場合はシステム標準入力デバイスが設定される
*/
void set_console_in_device(struct st_device *dev)
{
	DKFPRINTF(0x01, "dev = \"%s\" %p\n", dev->name, dev);

	sys_call(SYSCALL_SET_CONSOLE_IN, (void *)dev);
}

/**
   @brief	実行タスクの標準出力デバイスを設定する

   @param[in]	dev	デバイス

   @remarks	dev が 0 の場合はシステム標準出力デバイスが設定される
*/
void set_console_out_device(struct st_device *dev)
{
	DKFPRINTF(0x01, "dev = \"%s\" %p\n", dev->name, dev);

	sys_call(SYSCALL_SET_CONSOLE_OUT, (void *)dev);
}

/**
   @brief	実行タスクのエラー出力デバイスを設定する

   @param[in]	dev	デバイス

   @remarks	dev が 0 の場合はシステム標準エラー出力デバイスが設定される
*/
void set_error_out_device(struct st_device *dev)
{
	DKFPRINTF(0x01, "dev = \"%s\" %p\n", dev->name, dev);

	sys_call(SYSCALL_SET_ERROR_OUT, (void *)dev);
}

/*
  デバッグ用API
*/

int task_get_tasks_info(struct st_task_info *ti, int count)
{
	volatile struct st_task_info_param param;

	DKFPRINTF(0x01, "\n");

	param.ti = ti;
	param.count = count;

	sys_call(SYSCALL_GET_TASKS_INFO, (void *)&param);

	return param.ret;
}

void print_task_list(void)
{
	DKFPRINTF(0x01, "\n");

	sys_call(SYSCALL_PRINT_TASK_LIST, (void *)0);
}

void print_task_queue(void)
{
	DKFPRINTF(0x01, "\n");

	sys_call(SYSCALL_PRINT_TASK_QUEUE, (void *)0);
}

void print_call_trace(void)
{
	DKFPRINTF(0x01, "\n");

	sys_call(SYSCALL_PRINT_CALLTRACE, (void *)0);
}
