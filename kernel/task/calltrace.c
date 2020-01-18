/** @file
    @brief	デバッグ用システムコールトレース

    @date	2015.11.08
    @author	Takashi SHUDO
*/

#include "sysconfig.h"

#include "calltrace.h"
#include "syscall.h"
#include "timer.h"
#include "tkprintf.h"
#include "syscall_param.h"
#include "task_opration.h"

#ifndef GSC_KERNEL_MAX_CALLTRACE_RECORD
#define GSC_KERNEL_MAX_CALLTRACE_RECORD	32	// $gsc カーネルシステムコールトレース記録数
#endif

static struct st_call_record call_record[GSC_KERNEL_MAX_CALLTRACE_RECORD];
int callrec_num = 0;
char flg_rec_round = 0;

extern const char syscall_name[][16];

void init_calltrace(void)
{
	callrec_num = 0;
	flg_rec_round = 0;
}

void record_calltrace(int syscall, int status, void *resource, int arg, int count, void *sp)
{
	struct st_call_record *rp;

	rp = &call_record[callrec_num];

	rp->time = get_system_utime();
	rp->tcb = run_task;

	rp->syscall = syscall;
	rp->status = status;
	rp->resource = resource;
	rp->arg = arg;
	rp->count = count;
	rp->sp = sp;

	callrec_num ++;
	if(callrec_num >= GSC_KERNEL_MAX_CALLTRACE_RECORD) {
		callrec_num = 0;
		flg_rec_round = 1;
	}
}

extern const char status_str[][7];

static void print_record(struct st_call_record *rp)
{
	tkprintf("%8ld.%03ld : [%2d]%10s %6s %12s",
		 rp->time/1000,
		 rp->time%1000,
		 rp->tcb->id,
		 rp->tcb->name,
		 status_str[rp->status],
		 syscall_name[rp->syscall]);

	switch(rp->syscall) {
	case SYSCALL_DISPATCH:
		tkprintf("%10s", ((struct st_tcb *)(rp->resource))->name);
		break;

	case SYSCALL_TASK_ADD:
	case SYSCALL_TASK_EXEC:
	case SYSCALL_TASK_EXIT:
	case SYSCALL_TASK_PAUSE:
		break;

	case SYSCALL_TASK_SLEEP:
		tkprintf("%10s %8d", "", rp->arg);
		break;

	case SYSCALL_TASK_KILL:
	case SYSCALL_TASK_WAKEUP:
	case SYSCALL_EVTQUE_INIT:
		break;

	case SYSCALL_EVTQUE_WAIT:
		tkprintf("%10s %8d", ((struct st_event *)(rp->resource))->name, rp->arg);
		break;

	case SYSCALL_EVTQUE_PUSH:
		tkprintf("%10s", ((struct st_tcb *)(rp->resource))->name);
		break;

	case SYSCALL_EVTQUE_CLEAR:
		tkprintf("%10s %8s %8d", ((struct st_event *)(rp->resource))->name, "", rp->count);
		break;

	case SYSCALL_EVTQUE_CHECK:
		tkprintf("%10s", ((struct st_tcb *)(rp->resource))->name);
		break;

	case SYSCALL_EVTQUE_WAKEUP:
		tkprintf("%10s %8s %8d", ((struct st_event *)(rp->resource))->name, "", rp->count);
		break;

	case SYSCALL_EVTQUE_DISPOSE:
		tkprintf("%10s", ((struct st_tcb *)(rp->resource))->name);
		break;

	case SYSCALL_MUTEX_INIT:
		tkprintf("%10s", ((struct st_tcb *)(rp->resource))->name);
		break;

	case SYSCALL_MUTEX_LOCK:
		tkprintf("%10s %8d", ((struct st_mutex *)(rp->resource))->name, rp->arg);
		break;

	case SYSCALL_MUTEX_UNLOCK:
		tkprintf("%10s", ((struct st_mutex *)(rp->resource))->name);
		break;

	case SYSCALL_MUTEX_DISPOSE:
		tkprintf("%10s", ((struct st_tcb *)(rp->resource))->name);
		break;

	case SYSCALL_SET_CONSOLE_IN:
	case SYSCALL_SET_CONSOLE_OUT:
	case SYSCALL_SET_ERROR_OUT:
		break;

	case SYSCALL_TIMEOUT_WAKEUP:
		break;

	case SYSCALL_PRINT_TASK_LIST:
	case SYSCALL_PRINT_TASK_QUEUE:
	case SYSCALL_PRINT_CALLTRACE:
	case SYSCALL_TASK_PRIORITY:
		break;
	default:
		tkprintf("Invalid syscall");
		break;
	}

	tkprintf("\n");
//	tkprintf(" %p\n", rp->sp);
}

void print_calltrace(void)
{
	int i;

	if(flg_rec_round != 0) {
		for(i=callrec_num; i<GSC_KERNEL_MAX_CALLTRACE_RECORD; i++) {
			print_record(&call_record[i]);
		}
	}

	for(i=0; i<callrec_num; i++) {
		print_record(&call_record[i]);
	}
}
