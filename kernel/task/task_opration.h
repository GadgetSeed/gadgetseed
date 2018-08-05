/** @file
    @brief	タスク操作

    @date	2017.11.25
    @author	Takashi SHUDO
*/

#ifndef TASK_OPRATION_H
#define TASK_OPRATION_H

extern struct st_tcb *run_task;
extern struct st_tcb dummy_task;

void init_task(void);
void task_schedule(void *sp, unsigned long long systime);


#endif // TASK_OPRATION_H
