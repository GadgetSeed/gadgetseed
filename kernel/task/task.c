/** @file
    @brief	タスク制御

    これらの関数は非タスク状態から実行されなければならない

    @date	2017.09.03
    @date	2011.02.27
    @author	Takashi SHUDO
*/

#include "timer.h"
#include "asm.h"
#include "str.h"
#include "tkprintf.h"
#include "tcb.h"
#include "task.h"
#include "event.h"
#include "mutex.h"
#include "syscall.h"
#include "calltrace.h"
#include "interrupt.h"

#include "task_opration.h"
#include "queue_opration.h"
#include "mutex_opration.h"
#include "event_opration.h"
#include "sleepqueue.h"
#include "waitqueue.h"
#include "syscall_param.h"

//#define DEBUGKBITS 2
#include "dkprintf.h"


#ifndef GSC_KERNEL_IDLE_TASK_STACK_SIZE
#define GSC_KERNEL_IDLE_TASK_STACK_SIZE	(1024)	// $gsc カーネルアイドルタスクのスタックサイズ
#endif

struct st_tcb *run_task;	// 実行中タスク
struct st_tcb dummy_task;	// タスクが起動される前のsyscall用タスクコンテキスト

static int new_task_id;
static int enable_schedule;
static struct st_tcb *last_task;	// ディスパッチ前の実行中タスク(DEBUG)
static struct tcb_queue task_list_head;	// 全タスクキュー
static struct st_queue ready_queue_head[GSC_KERNEL_MAX_TASK_PRIORITY];// 実行タスクキュー


static struct st_tcb idle_tcb;
static unsigned int idle_stack[GSC_KERNEL_IDLE_TASK_STACK_SIZE/sizeof(unsigned int)];

/* NOT API
   @brief	IDLE(何もしない)タスク
*/
static int idle_task(char *arg)
{
	DKFPRINTF(0x01, "idle_task start\n");

	while(1) {
#ifndef LINT
		sleep_cpu();	// 何もせず終了しない
#endif
	}

	return 0;
}

/* NOT API
   @brief	タスク制御を初期化する
*/
void init_task(void)
{
	int i;

	init_calltrace();

	new_task_id = 0;
	enable_schedule = 0;
	run_task = &dummy_task;

	init_queue(&task_list_head.queue);

	for(i=0; i<GSC_KERNEL_MAX_TASK_PRIORITY; i++) {
		init_queue(&ready_queue_head[i]);
	}

	init_queue(&timeout_wait_queue_head.queue);
	init_queue(&wait_queue_head);
	init_eventqueue();
	init_mutex();

	task_add_ISR(idle_task, "IDLE", GSC_KERNEL_MAX_TASK_PRIORITY-1, &idle_tcb,
		     idle_stack, GSC_KERNEL_IDLE_TASK_STACK_SIZE, 0);
}

static void task_startup(void)
{
	run_task->main_func(run_task->arg);

	task_exit();

	// 戻り値を親タスクに渡す(予定)
}

/* NOT API
    @brief	最高優先順位タスクのサーチ
*/
static struct st_tcb * search_next_task(void)
{
	int i;

	for(i=0; i<GSC_KERNEL_MAX_TASK_PRIORITY; i++) {
		if(check_queue(&ready_queue_head[i])) {
			DKPRINTF(2, "Run task id = %d \"%s\"\n",
				 ((struct st_tcb *)ready_queue_head[i].next)->id,
				 ((struct st_tcb *)ready_queue_head[i].next)->name);
			return (struct st_tcb *)(ready_queue_head[i].next);
		}
	}

	DKFPRINTF(0x01, "Cannot find exec task\n");

	return (struct st_tcb *)0;
}

static void print_queue(struct st_queue *queue)
{
	struct st_queue *tmp = queue->next;

	if(check_queue(queue) != 0) {
		while(tmp->next != queue->next) {
			tkprintf("->(%d:%s)", ((struct st_tcb *)tmp)->id,
				((struct st_tcb *)tmp)->name);
			tmp = tmp->next;
		}
	}
	tkprintf("\n");
}

static void print_tcb_queue(struct tcb_queue *queue)
{
	struct st_queue *tmp = ((struct st_queue *)queue)->next;

	if(check_queue((struct st_queue *)queue) != 0) {
		while(tmp->next != ((struct st_queue *)queue)->next) {
			tkprintf("->(%d:%s[%d])", ((struct tcb_queue *)tmp)->tcb->id,
				((struct tcb_queue *)tmp)->tcb->name,
				((struct tcb_queue *)tmp)->tcb->status);
			tmp = tmp->next;
		}
	}
	tkprintf("\n");
}

const char status_str[][8] = {
	"READY",
	"RUN",
	"TIMER",
	"EVENT",
	"MUTEX",
	"REQUEST",
	"DRMNT"
};

/**
   @brief	タスク情報を取得する

   @param[out]	ti	タスク情報
   @param[in]	count	取得する最大タスク数

   @return	取得したタスク数

   @attention	tiのサイズはcount数分確保していなければならない
*/
int get_tasks_info(struct st_task_info *ti, int count)
{
	int rtn = 0;
	struct tcb_queue *queue = (struct tcb_queue *)&task_list_head;
	struct st_queue *tmp = ((struct st_queue *)queue)->next;

	if(check_queue((struct st_queue *)queue) != 0) {
		while(tmp->next != ((struct st_queue *)queue)->next) {
			if(rtn >= count) {
				break;
			}

			ti->id = ((struct tcb_queue *)tmp)->tcb->id;
			(void)strncopy((uchar *)ti->name, (uchar *)((struct tcb_queue *)tmp)->tcb->name, TASK_NAME_LEN);
			ti->priority = ((struct tcb_queue *)tmp)->tcb->priority;
			ti->status = ((struct tcb_queue *)tmp)->tcb->status;
			ti->run_time = ((struct tcb_queue *)tmp)->tcb->run_time;
			tmp = tmp->next;
			ti ++;
			rtn ++;
		}
	}

	return rtn;
}


void print_task(void)
{
	struct tcb_queue *queue = (struct tcb_queue *)&task_list_head;
	struct st_queue *tmp = ((struct st_queue *)queue)->next;
	unsigned int systime = get_kernel_time();

	tkprintf("PID Name       Pri Status Entry    PC       Stack(size)    SP       SleepTime\n");

	if(check_queue((struct st_queue *)queue) != 0) {
		while(tmp->next != ((struct st_queue *)queue)->next) {
			tkprintf("%3d %10s %3d %6s %P %08X %P(%04X) %P %9d\n",
				 ((struct tcb_queue *)tmp)->tcb->id,
				 ((struct tcb_queue *)tmp)->tcb->name,
				 ((struct tcb_queue *)tmp)->tcb->priority,
				 status_str[((struct tcb_queue *)tmp)->tcb->status],
				 (((struct tcb_queue *)tmp)->tcb->main_func),
#ifndef GSC_TARGET_SYSTEM_EMU
				 ((union st_regs *)(((struct tcb_queue *)tmp)->tcb->sp))->name.pc,
#else
				 0,
#endif
				 (((struct tcb_queue *)tmp)->tcb->stack_addr),
				 (unsigned int)(((struct tcb_queue *)tmp)->tcb->stack_size),
				 (((struct tcb_queue *)tmp)->tcb->sp),
				 (((struct tcb_queue *)tmp)->tcb->wup_time == 0) ?
				 0 : (unsigned int)(((struct tcb_queue *)tmp)->tcb->wup_time - systime)
				 );
			tmp = tmp->next;
		}
	}
}

void print_queues(void)
{
	int i;

	tkprintf("======== Task Queue ========\n");

	tkprintf("All         ");
	print_tcb_queue(&task_list_head);

	tkprintf("TimeoutWait ");
	print_tcb_queue(&timeout_wait_queue_head);

	for(i=0; i<GSC_KERNEL_MAX_TASK_PRIORITY; i++) {
		tkprintf("Ready [%d]   ", i);
		print_queue(&ready_queue_head[i]);
	}

	tkprintf("Wait        ");
	print_queue(&wait_queue_head);

	if(check_queue(&event_queue_list.list) != 0) {
		struct st_event *tmp = (struct st_event *)event_queue_list.list.next;

		while(tmp->list.next != event_queue_list.list.next) {
			tkprintf("Event [%10s](%d) ", tmp->name, fifo_size(&tmp->event));
			print_queue(&tmp->proc_head);
			tmp = (struct st_event *)tmp->list.next;
		}
	}

	i = 0;
	if(check_queue(&mutex_queue_list.list) != 0) {
		struct st_mutex *tmp = (struct st_mutex *)mutex_queue_list.list.next;

		while(tmp->list.next != mutex_queue_list.list.next) {
			tkprintf("MUTEX [%10s] ", tmp->name);
			if(tmp->lock_ps != 0) {
				tkprintf("Lock(%d:%s)",
					 tmp->lock_ps->id,
					 tmp->lock_ps->name);
			}
			print_queue(&tmp->wait_ps);
			tmp = (struct st_mutex *)tmp->list.next;
		}
	}

	tkprintf("Run : PID=%d \"%s\"\n", run_task->id, run_task->name);

	//disp_regs(run_task->sp);

	tkprintf("===============================\n");
}

static void print_task_stack(struct st_tcb *tcb)
{
	kxdump(tcb->stack_addr, tcb->stack_size);
}

void print_stack(void)
{
	tkprintf("Stack dump(PID=%d \"%s\")\n", run_task->id, run_task->name);

	print_task_stack(run_task);
}

static void dispatch_task(struct st_tcb *task, int status)
{
	struct st_tcb *otcb = run_task;
	unsigned int now_time = get_system_utime();

	if(task == 0) return;

#ifdef GSC_TARGET_SYSTEM_EMU
	record_calltrace(SYSCALL_DISPATCH, run_task->status, task, status, 0, 0);
#else
	record_calltrace(SYSCALL_DISPATCH, run_task->status, task, status, 0, task->sp);
#endif

//	disable_interrupt();
	run_task->status = status;

	run_task->run_time += (now_time - run_task->meas_time);
	run_task = task;
	run_task->meas_time = now_time;

	run_task->wup_time = 0;
	enable_schedule = 1;
#ifdef DEBUG
	print_queues();
#endif
//	disp_regs(otcb);
//	disp_regs(task);

	last_task = run_task;

	run_task->status = PSTAT_RUN;

#ifndef GSC_TARGET_SYSTEM_EMU
	if(run_task->sp < run_task->stack_addr) {
		tkprintf("PID %d \"%s\" Stack OVER %p(%ld)\n",
			run_task->id,
			run_task->name,
			run_task->stack_addr,
			run_task->stack_size);
		disp_regs(run_task->sp);
	}
#endif

	dispatch(otcb, task);

	// ここにはこない
}

static void wakeup_task(struct st_tcb *tcb)
{
	DKFPRINTF(0x01, "Wakeup PID = %d \"%s\"\n", tcb->id, tcb->name);

	insert_queue(&ready_queue_head[tcb->priority], (struct st_queue *)tcb);

	DKPRINTF(0x02, "%s SLEEP PID = %d \"%s\"\n", __FUNCTION__,
		run_task->id, run_task->name);
#ifdef DEBUG
	disp_regs(run_task->sp);
#endif
	DKPRINTF(0x02, "%s WAKEUP PID = %d \"%s\"\n", __FUNCTION__,
		tcb->id, tcb->name);
#ifdef DEBUG
	disp_regs(tcb->sp);
#endif
	dispatch_task(tcb, PSTAT_READY);
}

static void task_add_queue(struct st_tcb *tcb)
{
	DKFPRINTF(0x01, "Add PID = %d \"%s\"\n", tcb->id, tcb->name);

	add_queue(&ready_queue_head[tcb->priority], (struct st_queue *)tcb);
}

#ifdef GSC_TARGET_SYSTEM_EMU
extern volatile int flg_interrput_proc;
#endif

/* NOT API
   @brief	タスクをスケジュールする

   タイマ割り込みで実行する
*/
void task_schedule(void *sp, unsigned long long systime)
{
	struct st_tcb *task;

	DKFPRINTF(3, "SYSTIME = %lu\n", systime);

	if(enable_schedule == 0) {
		return;
	}

	if(run_task == 0) {
		return;
	}

#if 0
	if(run_task == &dummy_task) {
		return;
	}
#endif

	run_task->sp = sp;

	task = sleepqueue_schedule(systime);

	if(task != 0) {
		// イベント、MUTEX待ちであればそれらの待ちキューからも外す
		switch(task->status) {
		case PSTAT_TIMER_WAIT:
			break;
		case PSTAT_EVENT_WAIT:
		case PSTAT_MUTEX_WAIT:
			del_queue(&(task->queue.queue));
			break;
		default:
			SYSERR_PRINT("Invalid timeout status(%d)", task->status);
			break;
		}

		record_calltrace(SYSCALL_TIMEOUT_WAKEUP, task->status, 0, 0, 0, sp);
#ifdef GSC_TARGET_SYSTEM_EMU
		flg_interrput_proc = 0;
#endif
		wakeup_task(task);
	}

#ifdef GSC_TARGET_SYSTEM_EMU
	flg_interrput_proc = 0;
#endif
}


/* NOT API
   @brief	タスクコンテキストを初期化する
*/
static struct st_tcb * task_init(task_func func,
				 char *name,
				 int priority,
				 struct st_tcb *tcb,
				 void *stack,
				 int stack_size,
				 char *arg)
{
	struct st_tcb *task;
	extern struct st_device *con_in_dev;
	extern struct st_device *con_out_dev;
	extern struct st_device *con_err_dev;

	DKFPRINTF(0x01, "%s", name);

	task = tcb;
	if(task == 0) {
		SYSERR_PRINT("Cannot init tcb %p\n", tcb);
		return 0;
	}

	task->status = PSTAT_DORMANT;

	init_queue(&(task->queue.queue));
	task->queue.tcb = task;

	init_queue(&(task->timer_list.queue));
	task->timer_list.tcb = task;

	init_queue(&(task->task_list.queue));
	task->task_list.tcb = task;

	task->sp = (void *)(stack + stack_size); // (void *)ポインタへの加算は1バイトを期待
	/*
	  暫定的に設定(task->sp は setup_task() で更新)
	*/

	task->stack_addr = stack;
	task->stack_size = stack_size;
	task->main_func = func;
	task->id        = new_task_id;
	new_task_id ++;
	(void)strncopy((unsigned char *)task->name, (unsigned char *)name, TASK_NAME_LEN);
	task->priority = priority;
	task->wup_time = 0;
	task->arg = arg;
	task->stdin_dev = con_in_dev;
	task->stdout_dev = con_out_dev;
	task->error_dev = con_err_dev;
	task->meas_time = 0;
	task->run_time = 0;

	setup_task(stack, stack_size, task_startup, task);

	add_queue(&task_list_head.queue, &task->task_list.queue);

	DKPRINTF(0x01, "name(id) \"%s\" (%d)\n", task->name, task->id);
	DKPRINTF(0x01, "sp       = %p\n", task->sp);
	DKPRINTF(0x01, "start    = %p\n", task->main_func);
	DKPRINTF(0x01, "id       = %d\n", task->id);
	DKPRINTF(0x01, "priority = %d\n", task->priority);

	return task;
}


/**
   @brief	タスクを実行する

   @param[in]	func		タスク関数
   @param[in]	name		タスク名文字列ポインタ
   @param[in]	priority	タスク優先度
   @param[in]	tcb		タスクコンテキストポインタ
   @param[in]	stack		タスクスタックポインタ
   @param[in]	stack_size	タスクスタックサイズ
   @param[in]	arg		タスク実行時引数文字列ポインタ

   @remarks	割り込みハンドラからのみ実行可能
*/
void task_exec_ISR(task_func func, char *name, int priority, struct st_tcb *tcb,
		   void *stack, int stack_size, char *arg)
{
	struct st_tcb *task;

	task = task_init(func, name, priority, tcb, stack, stack_size, arg);

	if(task != 0) {
		wakeup_task(task);
	}
}


/**
   @brief	タスクを追加する

   @param[in]	func		タスク関数
   @param[in]	name		タスク名文字列ポインタ
   @param[in]	priority	タスク優先度
   @param[in]	tcb		タスクコンテキストポインタ
   @param[in]	stack		タスクスタックポインタ
   @param[in]	stack_size	タスクスタックサイズ
   @param[in]	arg		タスク実行時引数文字列ポインタ

   @remarks	割り込みハンドラからのみ実行可能
*/
void task_add_ISR(task_func func, char *name, int priority, struct st_tcb *tcb,
		  void *stack, int stack_size, char *arg)
{
	struct st_tcb *task;

	task = task_init(func, name, priority, tcb, stack, stack_size, arg);

	if(task != 0) {
		task_add_queue(task);
	}
}


/* NOT API
   @brief	実行中タスクを終了する

   @param[in]	sp	スタックポインタ

   @remarks	割り込みハンドラからのみ実行可能
*/
void task_exit_ISR(void *sp)
{
	DKFPRINTF(0x01, "Exit PID = %d \"%s\"\n", run_task->id,
		  run_task->name);

	del_queue(&run_task->task_list.queue);

	(void)del_next_queue(&ready_queue_head[run_task->priority]);

	dispatch_task(search_next_task(), PSTAT_DORMANT);
}


static struct st_tcb *search_id(struct st_queue *queue, int id)
{
	struct st_queue *tmp = queue->next;

	if(check_queue(queue) == 0) {
		return (struct st_tcb *)0;
	}

	while(tmp->next != queue->next) {
		if(((struct st_tcb *)tmp)->id == id) {
			return (struct st_tcb *)tmp;
		}
		tmp = tmp->next;
	}

	return (struct st_tcb *)0;
}

static struct st_tcb *search_task_id(int id)
{
	struct st_tcb *tmp = 0;
	struct st_queue *tq;
	int i;

	for(i=0; i<GSC_KERNEL_MAX_TASK_PRIORITY; i++) {
		tmp = search_id(&ready_queue_head[i], id);
		if(tmp != 0) {
			return tmp;
		}
	}

	tmp = search_id(&timeout_wait_queue_head.queue, id);
	if(tmp != 0) {
		return tmp;
	}

	tmp = search_id(&wait_queue_head, id);
	if(tmp != 0) {
		return tmp;
	}

	//
	tq = event_queue_list.list.next;

	while(tq->next != event_queue_list.list.next) {
		tmp = search_id(&((struct st_event *)tq)->proc_head, id);
		if(tmp != 0) {
			return tmp;
		}
		tq = tq->next;
	}

	return (struct st_tcb *)0;
}

/* NOT API
   @brief	idタスクを終了する

   @param[in]	sp	スタックポインタ
   @param[in]	id		タスクID

   @remarks	割り込みハンドラからのみ実行可能
*/
void task_kill_id_ISR(void *sp, int id)
{
	struct st_tcb *task;

	task = search_task_id(id);

	if(task != 0) {
		del_queue((struct st_queue *)task);
		DKFPRINTF(0x01, "Kill PID = %d \"%s\"\n", task->id,
			  task->name);
		dispatch_task(search_next_task(), PSTAT_DORMANT);
	} else {
		DKFPRINTF(0x01, "No task id %d\n", id);
	}
}


/**
   @brief	idタスクを実行する

   @param[in]	sp	スタックポインタ
   @param[in]	id		タスクID

   @remarks	割り込みハンドラからのみ実行可能
*/
void task_wakeup_id_ISR(void *sp, int id)
{
	struct st_tcb *task;

	task = search_id(&wait_queue_head, id);

	if(task != 0) {
		run_task->sp = sp;
		del_queue((struct st_queue *)task);
		DKFPRINTF(0x01, "Wakeup PID = %d \"%s\"\n", task->id,
			  task->name);
		wakeup_task(task);
	} else {
		DKFPRINTF(0x01, "No task id %d\n", id);
	}
}


/* NOT API
   @brief	実行中タスクを待ち状態にする

   @param[in]	sp	スタックポインタ

   @remarks	割り込みハンドラからのみ実行可能
*/
void task_pause_ISR(void *sp)
{
	DKFPRINTF(0x01, "Pause PID = %d \"%s\"\n", run_task->id,
		  run_task->name);

	record_calltrace(SYSCALL_TASK_PAUSE, run_task->status,
			 0, 0, 0, sp);

	run_task->sp = sp;

	(void)del_next_queue(&ready_queue_head[run_task->priority]);

	waitqueue_add(run_task);

	dispatch_task(search_next_task(), PSTAT_REQUEST_WAIT);
}


/* NOT API
   @brief	実行中タスクをスリープする

   @param[in]	sp	スタックポインタ
   @param[in]	sleep_time	スリープ時間(msec)

   @remarks	割り込みハンドラからのみ実行可能
*/
void task_sleep_ISR(void *sp, unsigned int sleep_time)
{
	unsigned int systime = get_kernel_time();

	DKFPRINTF(0x01, "Sleep PID = %d \"%s\" time = %ld\n",
		  run_task->id, run_task->name, sleep_time);

	run_task->sp = sp;

	record_calltrace(SYSCALL_TASK_SLEEP, run_task->status,
			 0, (int)sleep_time, 0, sp);

	(void)del_next_queue(&ready_queue_head[run_task->priority]);

	sleepqueue_add(run_task, sleep_time, systime);

	dispatch_task(search_next_task(), PSTAT_TIMER_WAIT);
}

/* NOT API
   @brief	実行中タスクをイベント待ち状態にする

   @param[in]	sp		スタックポインタ
   @param[in]	evtque		イベントキューポインタ
   @param[out]	args		イベント引数ポインタ
   @param[in]	timeout		イベントタイムアウト待ち時間(msec)

   @remarks	割り込みハンドラからのみ実行可能
*/
void event_wait_ISR(void *sp, struct st_event *evtque, void *arg, unsigned int timeout)
{
	struct st_tcb *next_task;
	int rtn;

	DKFPRINTF(0x01, "evtque = \"%s\" %p\n",
		  evtque->name, evtque);
	DKPRINTF(0x01, "PID = %d \"%s\" arg = %p, timeout = %ld\n",
		 run_task->id, run_task->name, arg, timeout);
	DKPRINTF(0x01, "evtque->size = %d\n", evtque->size);

	run_task->sp = sp;

	if(run_task == &dummy_task) {
#ifndef GSC_TARGET_SYSTEM_EMU
		SYSERR_PRINT("No running task\n");
		print_queues();
#endif
		((struct evtque_param *)run_task->syscall.param)->ret = -1;
		return;
	}

	record_calltrace(SYSCALL_EVTQUE_WAIT, run_task->status,
			 (void *)evtque, (int)timeout, fifo_size(&evtque->event), sp);

	if(arg != 0) {
		rtn = read_fifo(&evtque->event, arg, evtque->size);
	} else {
		rtn = drop_fifo(&evtque->event, evtque->size);
	}
	if(rtn != 0) {
		// 既にイベントが１回以上発生していた
		((struct evtque_param *)run_task->syscall.param)->ret = timeout;
		// なのでイベント待ちはせず終了
		return;
	}

	// 自タスクをレディーキューから外し
	(void)del_next_queue(&ready_queue_head[run_task->priority]);

	// イベント待ちキューに追加
	_eventqueue_wait(evtque, run_task);

	// タイムアウト時間指定の場合はタイムアウト待ちキューに追加
	if(timeout != 0) {
		unsigned int systime = get_kernel_time();

		sleepqueue_add(run_task, timeout, systime);
	}

	next_task = search_next_task();
	DKPRINTF(0x02, "%s WAIT_EVENT PID = %d \"%s\"\n", __FUNCTION__,
		 run_task->id, run_task->name);
#ifdef DEBUG
	disp_regs(run_task->sp);
#endif
	DKPRINTF(0x02, "%s WAKEUP PID = %d \"%s\"\n", __FUNCTION__,
		 next_task->id, next_task->name);
#ifdef DEBUG
	disp_regs(next_task->sp);
#endif
	dispatch_task(next_task, PSTAT_EVENT_WAIT);
}

/**
   @brief	イベントカウントを取得する

   @param[in]	sp		スタックポインタ
   @param[in]	evtque		イベントキューポインタ

   @remarks	割り込みハンドラからのみ実行可能
*/
int event_check_ISR(void *sp, struct st_event *evtque)
{
	int count = 0;

	DKFPRINTF(0x01, "evtque = \"%s\" %p\n", evtque->name, evtque);

	record_calltrace(SYSCALL_EVTQUE_CHECK, run_task->status, evtque, evtque->size, fifo_size(&evtque->event), sp);

	count = fifo_size(&evtque->event) / evtque->size;

	DKFPRINTF(0x01, "count = %d\n", count);

	return count;
}

/**
   @brief	イベントカウンタをクリアリセットする

   @param[in]	sp		スタックポインタ
   @param[in]	evtque		イベントキューポインタ

   @remarks	割り込みハンドラからのみ実行可能
   @remarks	イベント待ちタスクは実行状態にはならない
*/
void event_clear_ISR(void *sp, struct st_event *evtque)
{
	DKFPRINTF(0x01, "evtque = \"%s\" %p\n", evtque->name, evtque);

	record_calltrace(SYSCALL_EVTQUE_CLEAR, run_task->status, evtque, 0, fifo_size(&evtque->event), sp);

	clear_fifo(&evtque->event);
}

/**
   @brief	イベントFIFOにイベントを登録する

   @param[in]	sp		スタックポインタ
   @param[in]	evtque		イベントキューポインタ
   @param[in]	arg		イベント引数ポインタ

   @remarks	割り込みハンドラからのみ実行可能
   @remarks	イベント待ちタスクは実行状態にはならない
*/
void event_push_ISR(void *sp, struct st_event *evtque, void *arg)
{
	DKFPRINTF(0x01, "evtque = \"%s\" %p\n", evtque->name, evtque);

	record_calltrace(SYSCALL_EVTQUE_PUSH, run_task->status, evtque, 0, fifo_size(&evtque->event), sp);

	// イベントFIFOに追加
	if(write_fifo(&evtque->event, arg, evtque->size) != evtque->size) {
		DKFPRINTF(0x01, "event fifo full\n");
	}
}

/**
   @brief	イベント待ちタスクを起動する

   @param[in]	sp		スタックポインタ
   @param[in]	evtque		イベントキューポインタ

   @remarks	割り込みハンドラからのみ実行可能
   @attention	event_push_ISR() をコールした割り込み処理内で実行する必要がある
   @attention	event_set_ISR() 実行以前に1回以上 event_push_ISR() が実行されている必要がある
*/
void event_set_ISR(void *sp, struct st_event *evtque)
{
	struct st_tcb *task;

	DKFPRINTF(0x01, "%s evtque = \"%s\" %p\n", evtque->name, evtque);

	// イベント待ちキューを取得
	task = _eventqueue_wakeup(evtque);

	if(task != 0) {
		// イベント待ちタスク有り
		unsigned int systime = get_kernel_time();
		run_task->sp = sp;
		DKFPRINTF(0x01, "WAKEUP PID = %d \"%s\"\n", task->id, task->name);

		if(task->wup_time != 0) {
			// タイムアウト時間指定有り
			// タイムアウトを戻り値に設定
			((struct evtque_param *)task->syscall.param)->ret =
					task->wup_time - systime;
			// タイムアウト待ちキューから削除
			del_queue(&(task->timer_list.queue));
		} else {
			// タイムアウト時間指定無しは戻り値0
			((struct evtque_param *)task->syscall.param)->ret = 0;
		}
		// イベント待ちタスクを起床
		record_calltrace(SYSCALL_EVTQUE_WAKEUP, task->status, evtque, 0, fifo_size(&evtque->event), sp);
		wakeup_task(task);
	} else {
		record_calltrace(SYSCALL_EVTQUE_WAKEUP, 0, evtque, 0, fifo_size(&evtque->event), sp);
	}
}

/**
   @brief	イベントキューにイベントを登録し、イベント待ちタスクを起動する

   @param[in]	sp		スタックポインタ
   @param[in]	evtque		イベントキューポインタ
   @param[in]	arg		イベント引数ポインタ

   @remarks	割り込みハンドラからのみコール可能
   @remarks	イベント待ちタスクがなければイベントカウンタをインクリメントする
*/
void event_wakeup_ISR(void *sp, struct st_event *evtque, void *arg)
{
	struct st_tcb *task;

	DKFPRINTF(0x01, "%s evtque = \"%s\" %p, arg = %p\n", evtque->name, evtque, arg);
	DKPRINTF(0x01, "PID = %d \"%s\"\n", run_task->id, run_task->name);
	DKPRINTF(0x01, "evtque->size = %d\n", evtque->size);

#ifdef GSC_TARGET_SYSTEM_EMU
	if(is_in_interrupt()) {
		SYSERR_PRINT("EMU in SIGARM?\n");
	}
#endif

	// イベント待ちキューを取得
	task = _eventqueue_wakeup(evtque);

	if(task != 0) {
		// イベント待ちタスク有り
		unsigned int systime = get_kernel_time();
		run_task->sp = sp;
		DKFPRINTF(0x01, "WAKEUP PID = %d \"%s\"\n", task->id, task->name);

		// argを設定
		if(arg != 0) {
			int i;
			unsigned char *asp, *adp;
			asp = arg;
			adp = ((struct evtque_param *)task->syscall.param)->arg;
			for(i=0; i<evtque->size; i++) {
				*adp = *asp;
				adp ++;
				asp ++;
			}
		}

		if(task->wup_time != 0) {
			// タイムアウト時間指定有り
			// タイムアウトを戻り値に設定
			((struct evtque_param *)task->syscall.param)->ret =
					task->wup_time - systime;
			// タイムアウト待ちキューから削除
			del_queue(&(task->timer_list.queue));
		} else {
			// タイムアウト時間指定無しは戻り値0
			((struct evtque_param *)task->syscall.param)->ret = 0;
		}
		// イベント待ちタスクを起床
		record_calltrace(SYSCALL_EVTQUE_WAKEUP, task->status, evtque, 0, fifo_size(&evtque->event), sp);
		wakeup_task(task);
	} else {
		// イベントFIFOに追加
		if(write_fifo(&evtque->event, arg, evtque->size) != evtque->size) {
			DKFPRINTF(0x01, "event fifo full\n");
		}
		record_calltrace(SYSCALL_EVTQUE_WAKEUP, 0, evtque, 0, fifo_size(&evtque->event), sp);
	}
}


/* NOT API
   @brief	MUTEXのロックを試みる

   ロックされていない場合はロックを行う
   既にロックされていた場合はロック待ち状態となる

   @param[in]	sp		スタックポインタ
   @param[in]	evtque		MUTEXポインタ
   @param[in]	timeout		ロックタイムアウト時間(msec)
*/
void mutex_lock_ISR(void *sp, struct st_mutex *mutex, unsigned int timeout)
{
	int rtn = 0;

	DKFPRINTF(0x01, "%s mutex = \"%s\" %p\n", mutex->name, mutex);

	if(run_task == &dummy_task) {
#ifndef GSC_TARGET_SYSTEM_EMU
		SYSERR_PRINT("No running task\n");
		print_queues();
#endif
		((struct mutex_param *)run_task->syscall.param)->ret = -1;
		return;
	}

	record_calltrace(SYSCALL_MUTEX_LOCK, run_task->status,
			 (void *)mutex, (int)timeout, 0, sp);

	rtn = _mutex_lock(mutex, run_task);
	if(rtn > 0) {
		// 既にロック済みのMUTEX
		run_task->sp = sp;

		// 自タスクをREADYキューより外して
		(void)del_next_queue(&ready_queue_head[run_task->priority]);
		// MUTEXロック解除待ちとする
		DKPRINTF(0x01, "mutex locked wait %s\n", run_task->name);
		_mutex_wait(mutex, run_task);

		// タイムアウトした場合のために戻り値は 0 にしておく
		((struct mutex_param *)run_task->syscall.param)->ret = 0;

		// タイムアウト時間指定の場合はタイムアウト待ちキューに追加
		if(timeout != 0) {
			unsigned int systime = get_kernel_time();

			sleepqueue_add(run_task, timeout, systime);
		}

		// 次のタスクにスイッチ
		dispatch_task(search_next_task(), PSTAT_MUTEX_WAIT);
	} else if(rtn < 0) {
		// 自タスクで既にロック済みだったMUTEX(エラー)
		//disp_regs(run_task->sp);// DEBUG
		((struct mutex_param *)run_task->syscall.param)->ret = -1;
	} else {
		// ロックさせていないMUTEXだったので
		// MUTEXロック成功
		DKPRINTF(0x01, "mutex lock ok %s\n", run_task->name);
		((struct mutex_param *)run_task->syscall.param)->ret = timeout;
	}
}

/* NOT API
   @brief	MUTEXのロックを解除する

   ロック待ちタスクがいた場合、ロック解除したタスクがロックし、ロッ
   ク待ちタスクが実行状態になる

   @param[in]	sp		スタックポインタ
   @param[in]	evtque		MUTEXポインタ
*/
void mutex_unlock_ISR(void *sp, struct st_mutex *mutex)
{
	struct st_tcb *task;
	unsigned int systime;

	DKFPRINTF(0x01, "%s mutex = \"%s\" %p\n", mutex->name, mutex);

	run_task->sp = sp;

	if(run_task == &dummy_task) {
#ifndef GSC_TARGET_SYSTEM_EMU
		SYSERR_PRINT("No running task\n");
		print_queues();
#endif
		return;
	}

	record_calltrace(SYSCALL_MUTEX_UNLOCK, run_task->status,
			 (void *)mutex, 0, 0, sp);

	// 次のMUTEXロック待ちタスクを取り出す
	task = _mutex_unlock(mutex, run_task);

	systime = get_kernel_time();

	if(task != 0) {
		// 次のMUTEXロック待ちタスクが有る
		if(task->wup_time != 0) {
			// タイムアウト指定の場合タウムアウトキューから外す
			((struct mutex_param *)task->syscall.param)->ret =
					task->wup_time - systime;
			del_queue(&(task->timer_list.queue));
		} else {
			// タイムアウトの指定は無かった
			((struct mutex_param *)task->syscall.param)->ret = 0;
		}
		DKPRINTF(0x01, "mutex unlock wakeup %s\n", task->name);
		// 次のタスクを実行
		wakeup_task(task);
	}
	// MUTEXロック待ちタスクが無かったのでそのまま終了
	//tkprintf("No wait & lock mutex ?\n");
}

/***
 *
 */

void task_print_task_queue(void)
{
	tkprintf("systime: %10ld\n", get_kernel_time());
	print_queues();
}

void disp_task_info(void)
{
	tkprintf("Kernel Time = %ld\n", kernel_time_count);

	tkprintf("NOW  PID = %d \"%s\"\n", run_task->id, run_task->name);
	tkprintf("LAST PID = %d \"%s\"\n", last_task->id, last_task->name);
	tkprintf("LAST SYSCALL = %s(%d)\n",
		 syscall_name[last_syscall_type],
		 last_syscall_type);
}
