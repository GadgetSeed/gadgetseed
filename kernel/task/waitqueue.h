/** @file
    @brief	待ちタスク制御

    これらの関数は非タスク状態からコールされる

    @date	2011.04.03
    @author	Takashi SHUDO
*/

#ifndef WAITQUEUE_H
#define WAITQUEUE_H

#include "task/task.h"

extern struct st_queue wait_queue_head;

void waitqueue_add(struct st_tcb *tcb);
struct st_tcb *waitqueue_wakeup(struct st_tcb *tcb);

#endif // WAITQUEUE_H
