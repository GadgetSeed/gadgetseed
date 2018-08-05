/** @file
    @brief	イベントキュー操作

    @date	2017.11.25
    @au/	Takashi SHUDO
*/

#ifndef EVENT_OPRATION_H
#define EVENT_OPRATION_H

#include "event.h"

extern struct st_event event_queue_list;

void init_eventqueue(void);
void _eventqueue_wait(struct st_event *evtque, struct st_tcb *tcb);
struct st_tcb *_eventqueue_wakeup(struct st_event *evtque);

#endif // EVENT_OPRATION_H
