/** @file
    @brief	システムコール

    @date	2011.02.26
    @author	Takashi SHUDO
*/

#ifndef SYSCALL_H
#define SYSCALL_H

#include "task.h"
#include "tcb.h"
#include "event.h"
#include "mutex.h"
#include "device.h"

extern int task_add(task_func func, char *name, int priority, struct st_tcb *tcb, unsigned int *stack, int stack_size, void *arg);
extern int task_exec(task_func func, char *name, int priority, struct st_tcb *tcb, unsigned int *stack, int stack_size, void *arg);
extern void task_exit(void);
extern void task_pause(void);
extern void task_sleep(unsigned int stime);
extern void task_kill(int id);
extern void task_wakeup(int id);
extern void task_priority(int id, int priority);

extern void eventqueue_register(struct st_event *evtque, const char *name, void *args, unsigned int size, int count);
extern int event_wait(struct st_event *evtque, void *argp, unsigned int timeout);
extern int event_check(struct st_event *evtque);
extern void event_clear(struct st_event *evtque);
extern void event_wakeup(struct st_event *evtque, void *arg);
extern void eventqueue_unregister(struct st_event *evtque);

extern void mutex_register(struct st_mutex *mutex, const char *name);
extern int mutex_lock(struct st_mutex *mutex, unsigned int timeout);
extern int mutex_unlock(struct st_mutex *mutex);
extern void mutex_unregister(struct st_mutex *mutex);

extern void set_console_in_device(struct st_device *dev);
extern void set_console_out_device(struct st_device *dev);
extern void set_error_out_device(struct st_device *dev);

extern void print_task_list(void);
extern void print_task_queue(void);
extern void print_call_trace(void);

extern int task_get_tasks_info(struct st_task_info *ti, int count);

#endif // SYSCALL_H
