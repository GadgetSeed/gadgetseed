/** @file
    @brief	MUTEX操作

    @date	2017.11.25
    @author	Takashi SHUDO
*/

#ifndef MUTEX_OPRATION_H
#define MUTEX_OPRATION_H

#include "mutex.h"

extern struct st_mutex mutex_queue_list;

void init_mutex(void);

int _mutex_lock(struct st_mutex *mutex, struct st_tcb *tcb);
void _mutex_wait(struct st_mutex *mutex, struct st_tcb *tcb);
struct st_tcb * _mutex_unlock(struct st_mutex *mutex, struct st_tcb *tcb);

#endif // MUTEX_OPRATION_H
