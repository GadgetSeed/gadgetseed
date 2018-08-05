/** @file
    @brief	カーネルタイマ

    @date	2007.03.18
    @author	Takashi SHUDO
*/

#ifndef TIMER_H
#define TIMER_H

#include "device.h"

#ifndef GSC_KERNEL_TIMER_INTERVAL_MSEC
#define GSC_KERNEL_TIMER_INTERVAL_MSEC	10	///< $gsc カーネルタイマ割り込み間隔(ms)
#endif

extern unsigned long long kernel_time_count;	///< カーネル時間(ms)

typedef void (* timer_func)(void *sp, unsigned long long systime);	///< カーネルタイマ周期処理関数の型

extern void start_timer_func(void);
extern void stop_timer_func(void);
extern void init_timer(char *devname);
extern unsigned long long get_kernel_time(void);
extern void wait_time(unsigned int time);
extern void wait_utime(unsigned int time);
extern void register_kernel_timer_func(timer_func func);
extern int register_timer_func(timer_func func, unsigned long interval);
extern int unregister_timer_func(timer_func func);
extern int start_timer(void);
extern int stop_timer(void);
extern unsigned long long get_system_utime(void);

#endif // TIMER_H
