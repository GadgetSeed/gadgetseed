/** @file
    @brief	システムコール

    GadgetSeed のタスク関連システムコールはソフトウェア割り込みを使用します。

    @date	2011.02.26
    @author	Takashi SHUDO
*/

#include "asm.h"
#include "syscall.h"
#include "tkprintf.h"
#include "calltrace.h"
#include "syscall_param.h"
#include "task_opration.h"
#include "console.h"

//#define DEBUGKBITS 2
#include "dkprintf.h"


const char syscall_name[][16] = {
	"dispatch->",		// 0
	"TASK_ADD",		// 1
	"TASK_EXEC",		// 2
	"TASK_EXIT",		// 3
	"TASK_PAUSE",		// 4
	"TASK_SLEEP",		// 5
	"TASK_KILL",		// 6
	"TASK_WAKEUP",		// 7
	"EVT_REG",		// 8
	"EVT_WAIT",		// 9
	"EVT_PUSH",		// 10
	"EVT_CHECK",		// 11
	"EVT_CLEAR",		// 12
	"EVT_WAKEUP",		// 13
	"EVT_UNREG",		// 14
	"MTX_REG",		// 15
	"MTX_LOCK",		// 16
	"MTX_UNLOCK",		// 17
	"MTX_UNREG",		// 18
	"SET_CON_IN",		// 19
	"SET_CON_OUT",		// 20
	"SET_ERR_OUT",		// 21

	"TIM_WAKEUP",		// 22

	"PRINT_TASK",
	"PRINT_QUEUE",
	"PRINT_CALLTRACE",
	"GET_TASKS_INFO"
};

int syscall_cnt = 0;

/* NOT API
    @brief	システムコール割り込み処理
*/
void syscall_inthdr(unsigned int intnum, void *sp)
{
	if(syscall_cnt == 0) {
		SYSERR_PRINT("!!! Invalid syscall ?\n");
	}
	syscall_cnt --;

	run_task->sp = sp;

	DKPRINTF(0x01, "-- SYSCALL --\n");
	DKPRINTF(0x01, "task \"%s\"\n", run_task->name);
	DKPRINTF(0x01, "SP = %p\n", sp);
	DKPRINTF(0x01, "type = %s(%d)\n", syscall_name[run_task->syscall.type],
		 run_task->syscall.type);
	DKPRINTF(0x01, "param = %p\n", run_task->syscall.param);
#ifdef DEBUG
#include "timer.h"
	{
		unsigned int systime = get_kernel_time();
		DKPRINTF(0x01, "systime = %ld\n", systime);
	}
#endif

	switch(run_task->syscall.type) {
	case SYSCALL_TASK_ADD:
	{
		struct exec_task_param *param =
			(struct exec_task_param *)run_task->syscall.param;

		DKPRINTF(0x01, "SC param = %p\n", param);
		DKPRINTF(0x01, "SC func = %p\n", param->func);
		DKPRINTF(0x01, "SC name = \"%s\"\n", param->name);
		DKPRINTF(0x01, "SC priority= %d\n", param->priority);
		DKPRINTF(0x01, "SC stack= %p\n", param->stack);
		DKPRINTF(0x01, "SC stack_size = %d\n", param->stack_size);
		DKPRINTF(0x01, "SC arg = %d\n", param->arg);

		if(param == 0) {
			SYSERR_PRINT("type=%d param=%p\n",
				     run_task->syscall.type,
				     run_task->syscall.param);
			print_queues();
			return;
		}
		task_add_ISR(param->func,
				param->name,
				param->priority,
				param->tcb,
				param->stack,
				param->stack_size,
				param->arg);
	}
	break;

	case SYSCALL_TASK_EXEC:
	{
		struct exec_task_param *param =
			(struct exec_task_param *)run_task->syscall.param;

		DKPRINTF(0x01, "SC param = %p\n", param);
		DKPRINTF(0x01, "SC func = %p\n", param->func);
		DKPRINTF(0x01, "SC name = \"%s\"\n", param->name);
		DKPRINTF(0x01, "SC priority= %d\n", param->priority);
		DKPRINTF(0x01, "SC stack= %p\n", param->stack);
		DKPRINTF(0x01, "SC stack_size = %d\n", param->stack_size);
		DKPRINTF(0x01, "SC arg = %d\n", param->arg);

		if(param == 0) {
			SYSERR_PRINT("type=%d param=%p\n",
				     run_task->syscall.type,
				     run_task->syscall.param);
			print_queues();
			return;
		}
		task_exec_ISR(param->func,
				 param->name,
				 param->priority,
				 param->tcb,
				 param->stack,
				 param->stack_size,
				 param->arg);
	}
	break;

	case SYSCALL_TASK_EXIT:
		task_exit_ISR(sp);
		break;

	case SYSCALL_TASK_PAUSE:
		task_pause_ISR(sp);
		break;

	case SYSCALL_TASK_SLEEP:
		task_sleep_ISR(sp, (unsigned int)(long)run_task->syscall.param);
		break;

	case SYSCALL_TASK_KILL:
		task_kill_id_ISR(sp, (int)(long)run_task->syscall.param);
		break;

	case SYSCALL_TASK_WAKEUP:
		task_wakeup_id_ISR(sp, (int)(long)run_task->syscall.param);
		break;

	case SYSCALL_EVTQUE_INIT:
	{
		struct evtque_param *param =
			(struct evtque_param *)run_task->syscall.param;
		eventqueue_register_ISR(param->evtque, param->name, param->arg, param->size, param->count);
	}
	break;

	case SYSCALL_EVTQUE_WAIT:
	{
		struct evtque_param *param =
			(struct evtque_param *)run_task->syscall.param;
		event_wait_ISR(sp, param->evtque, param->arg, param->timeout);
	}
	break;

	case SYSCALL_EVTQUE_CLEAR:
		event_clear_ISR(sp, (struct st_event *)run_task->syscall.param);
		break;

	case SYSCALL_EVTQUE_CHECK:
	{
		struct evtque_param *param =
			(struct evtque_param *)run_task->syscall.param;
		param->ret = event_check_ISR(sp, (struct st_event *)param->evtque);
	}
	break;

	case SYSCALL_EVTQUE_WAKEUP:
	{
		struct evtque_param *param =
			(struct evtque_param *)run_task->syscall.param;
		event_wakeup_ISR(sp, param->evtque, param->arg);
	}
	break;

	case SYSCALL_EVTQUE_DISPOSE:
		eventqueue_unregister_ISR(
				(struct st_event *)run_task->syscall.param);
		break;

	case SYSCALL_MUTEX_INIT:
	{
		struct mutex_param *param =
			(struct mutex_param *)run_task->syscall.param;
		mutex_register_ISR(param->mutex, param->name);
	}
	break;

	case SYSCALL_MUTEX_LOCK:
	{
		struct mutex_param *param =
			(struct mutex_param *)run_task->syscall.param;
		mutex_lock_ISR(sp, param->mutex, param->timeout);
	}
	break;

	case SYSCALL_MUTEX_UNLOCK:
	{
		struct mutex_param *param =
			(struct mutex_param *)run_task->syscall.param;
		mutex_unlock_ISR(sp, param->mutex);
	}
	break;

	case SYSCALL_MUTEX_DISPOSE:
		mutex_unregister_ISR((struct st_mutex *)run_task->syscall.param);
		break;

	case SYSCALL_SET_CONSOLE_IN:
		set_console_in_device_ISR((struct st_device *)run_task->syscall.param);
		break;

	case SYSCALL_SET_CONSOLE_OUT:
		set_console_out_device_ISR((struct st_device *)run_task->syscall.param);
		break;

	case SYSCALL_SET_ERROR_OUT:
		set_error_out_device_ISR((struct st_device *)run_task->syscall.param);
		break;

	case SYSCALL_PRINT_TASK_LIST:
		print_task();
		break;

	case SYSCALL_PRINT_TASK_QUEUE:
		task_print_task_queue();
		break;

	case SYSCALL_PRINT_CALLTRACE:
		print_calltrace();
		break;

	case SYSCALL_GET_TASKS_INFO:
	{
		struct st_task_info_param *param =
			(struct st_task_info_param *)run_task->syscall.param;
		param->ret = get_tasks_info(param->ti, param->count);
	}
	break;

	case SYSCALL_TASK_PRIORITY:
	{
		struct st_task_priority_param *param =
			(struct st_task_priority_param *)run_task->syscall.param;
		task_priority_ISR(param->id, param->priority);
	}
	break;

	default:
		SYSERR_PRINT("Undifined SYSCALL %d\n", run_task->syscall.type);
		break;
	}

	dispatch(run_task, run_task);
}

int last_syscall_type = 0;

void sys_call(int type, void *param)
{
	DKFPRINTF(0x01, "type = %d param = %p\n", type, param);

#if 0
	if(syscall_cnt != 0) {
		SYSERR_PRINT("!!! Have not executed syscall %d ?\n",
			     syscall_cnt);
	}
#endif
	syscall_cnt ++;

	if(run_task == &dummy_task) {
		DKPRINTF(0x01, "Cannot exec syscall type=%d\n", type);
	}

	run_task->syscall.type = type;
	run_task->syscall.param = param;

	last_syscall_type = type;

#ifndef LINT
	syscall_trap();
#endif
}
