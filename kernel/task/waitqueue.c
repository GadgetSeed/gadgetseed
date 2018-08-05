/** @file
    @brief	待ちタスク制御

    これらの関数は非タスク状態からコールされる

    @date	2011.04.03
    @author	Takashi SHUDO
*/

#include "waitqueue.h"
#include "queue_opration.h"

//#define DEBUGKBITS 0x01
#include "dkprintf.h"


struct st_queue wait_queue_head;	///< 待ちタスクキュー

void waitqueue_add(struct st_tcb *tcb)
{
	DKFPRINTF(0x01, "Wait PID=%d \"%s\"\n", tcb->id, tcb->name);

	add_queue(&wait_queue_head, (struct st_queue *)tcb);
}

struct st_tcb *waitqueue_wakeup(struct st_tcb *tcb)
{
	DKFPRINTF(0x01, "Wait wakeup PID=%d \"%s\"\n", tcb->id, tcb->name);

	if(check_queue(&wait_queue_head) == 0) {
		return (struct st_tcb *)0;
	}

	del_queue((struct st_queue *)tcb);

	return tcb;
}
