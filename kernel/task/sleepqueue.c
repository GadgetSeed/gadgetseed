/** @file
    @brief	タイムアウト待ちタスクキューの操作

    GadgetSeed のタスク時間制御

    これらの関数は非タスク状態からコールされる

    @date	2011.03.17
    @author	Takashi SHUDO
*/

#include "sleepqueue.h"
#include "calltrace.h"
#include "timer.h"
#include "tkprintf.h"

#include "task_opration.h"
#include "queue_opration.h"

//#define DEBUGKBITS 0x01
#include "dkprintf.h"


struct tcb_queue timeout_wait_queue_head;	///< タイムアウト待ちタスクキュー

/* NOT API
    @brief	スリープ中タスクキューにTCBを追加する

    タイムアウト時間が早い順(tcb->wup_time が小さい)に追加
*/
void sleepqueue_add(struct st_tcb *tcb, unsigned int sleep_time,
		   unsigned int now_time)
{
	tcb->wup_time = now_time + sleep_time;	// 起床予定時間

	DKFPRINTF(0x01, "Sleep PID=%d \"%s\" %ld\n", tcb->id, tcb->name, sleep_time);
	DKPRINTF(0x01, "  now = %ld, wup = %ld\n", now_time, tcb->wup_time);
	//tkprintf("IN  :");
	//print_tcb_queue(&timeout_wait_queue_head);

	/*
	  タイムアウト時間の早いタスクの順に追加する
	*/
	if(check_queue(&timeout_wait_queue_head.queue) == 0) {
		DKPRINTF(0x02, "TQ 0 top\n");
		insert_queue(&timeout_wait_queue_head.queue,
			     &(tcb->timer_list.queue));
	} else {
		struct st_queue *tmp = timeout_wait_queue_head.queue.next;
		while(tmp->next != timeout_wait_queue_head.queue.next) {
			DKPRINTF(0x02, "T=%ld\n", ((struct tcb_queue *)tmp)->tcb->wup_time);
			if(tcb->wup_time <
			   ((struct tcb_queue *)tmp)->tcb->wup_time) {
				// キューに追加
				DKPRINTF(0x02, "TQ add\n");
				add_queue(tmp, &(tcb->timer_list.queue));
				goto end;
			}

			if(tmp == tmp->next) {
				SYSERR_PRINT("timeout_wait_queue broken? (%p)\n", tmp);
				disp_regs(run_task->sp);
				print_task();
				print_stack();
				//print_queues();
				print_calltrace();
				while(1);
			}
			tmp = tmp->next;
		}

		if(tcb->wup_time <
		   ((struct tcb_queue *)timeout_wait_queue_head.queue.next)
		   ->tcb->wup_time) {
			DKPRINTF(0x02, "TQ ? add\n");
			insert_queue(&timeout_wait_queue_head.queue,
				     &(tcb->timer_list.queue));
		} else {
			DKPRINTF(0x02, "TQ tail\n");
			add_queue(&timeout_wait_queue_head.queue,
				  &(tcb->timer_list.queue));
		}
	}

	//tkprintf("OUT :");
	//print_struct tcb_queue(&timeout_wait_queue_head);
end:
	return;
}

/* NOT API
    @brief	スリープ時間がタイムアウトしたTCBを返す
*/
struct st_tcb *sleepqueue_schedule(unsigned long long now_time)
{
	struct st_tcb *wup_tcb = 0;

	DKFPRINTF(0x01, "now_time = %ld\n", now_time);

	if(check_queue(&timeout_wait_queue_head.queue) == 0) {
		return (struct st_tcb *)0;
	} else {
		if(now_time >= ((struct tcb_queue *)(timeout_wait_queue_head.queue.next))->tcb->wup_time) {
			wup_tcb = ((struct tcb_queue *)del_next_queue(&timeout_wait_queue_head.queue))->tcb;
		} else {
			return (struct st_tcb *)0;
		}
	}

#ifdef DEBUGKBITS
	if(wup_tcb != 0) {
		DKPRINTF(0x01, "Sleep Timeout wakeup PID=%d \"%s\"\n", wup_tcb->id,
			 wup_tcb->name);
		DKPRINTF(0x02, "  now = %ld, wup = %ld\n", now_time, wup_tcb->wup_time);
	}
#endif

	return wup_tcb;
}
