/** @file
    @brief	スリープタスク制御

    これらの関数は非タスク状態からコールされる

    @date	2011.03.17
    @author	Takashi SHUDO
*/

#ifndef SLEEPQUEUE_H
#define SLEEPQUEUE_H

#include "task/task.h"

extern struct tcb_queue timeout_wait_queue_head;

void sleepqueue_add(struct st_tcb *tcb, unsigned int sleep_time,
		    unsigned int now_time);
struct st_tcb *sleepqueue_schedule(unsigned long long now_time);

#endif // SLEEPQUEUE_H
