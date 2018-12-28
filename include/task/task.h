/** @file
    @brief	タスク制御

    GadgetSeed のタスク制御

    @date	2017.09.03
    @date	2011.02.27
    @author	Takashi SHUDO
*/

#ifndef TASK_H
#define TASK_H

#include "asm.h"
#include "tcb.h"

#ifndef GSC_KERNEL_MAX_TASK_PRIORITY
#define GSC_KERNEL_MAX_TASK_PRIORITY	6	///< $gsc カーネルタスクプライオリティ段階数
#else
#if GSC_KERNEL_MAX_TASK_PRIORITY < 6
#error "KERNEL_MAX_TASK_PRIORITY must begreater then 6"
#endif
#endif

/*
  タスクプライリティ

  高
  0: shell
  1: デバイスドライバ
  2: ネットワーク
  3: アプリケーション(高プライオリティ)
  4: アプリケーション(低プライオリティ)
  5: IDLEタスク
  低
 */

#define TASK_PRIORITY_SHELL		0
#define TASK_PRIORITY_DEVICE_DRIVER	1
#define TASK_PRIORITY_NETWORK		2
#define TASK_PRIORITY_APP_HIGH		3
#define TASK_PRIORITY_APP_LOW		4
#define TASK_PRIORITY_IDLE		(GSC_KERNEL_MAX_TASK_PRIORITY - 1)

// プロセッサ依存関数
void setup_task(void *sp, int stack_size, void (* task)(void), struct st_tcb *tcb);
void disp_debug_info(void);
void disp_regs(void *sp);
void dispatch(struct st_tcb *otcb, struct st_tcb *tcb);

// プロセッサ非依存関数
void task_add_ISR(task_func func, char *name, int priority, struct st_tcb *tcb, void *stack, int stack_size, char *arg);
void task_exec_ISR(task_func func, char *name, int priority, struct st_tcb *tcb, void *stack, int stack_size, char *arg);
void task_exit_ISR(void *sp);
void task_kill_id_ISR(void *sp, int id);
void task_wakeup_id_ISR(void *sp, int id);
void task_pause_ISR(void *sp);
void task_sleep_ISR(void *sp, unsigned int sleep_time);

struct st_task_info {
	int id;			///< タスクID
	char	name[TASK_NAME_LEN + 1];///< タスク名文字列
	int	priority;		///< タスクプライオリティ
	int	status;			///< タスク状態(PSTAT_*)
	unsigned int	run_time;	///< タスク実行時間
}; ///< タスク情報

int get_tasks_info(struct st_task_info *ti, int count);

void print_task(void);
void print_queues(void);
void print_stack(void);
void task_print_task_queue(void);
void disp_task_info(void);

#endif // TASK_H
