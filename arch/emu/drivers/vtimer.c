/** @file
    @brief	仮想タイマドライバ

    @date	2009.10.25
    @author	Takashi SHUDO
*/

#include "sysconfig.h"
#include "asm-emu.h"
#include "device.h"
#include "tkprintf.h"
#include "device/timer_ioctl.h"
#include "task/tcb.h"

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>

//#define DEBUG
#ifdef DEBUG
#define VTPRINTF	printf
#else
#define VTPRINTF(x, ...)
#endif

static pthread_mutex_t vtimer_mutex;

extern struct st_tcb *run_task;

static void (* inth_func)(unsigned long sp);

extern int syscall_cnt;

static struct timespec vtime_start_time;

extern volatile int flg_interrput_proc;

void start_vtimer(void);

void lock_timer(void)
{
	pthread_mutex_lock(&vtimer_mutex);
}

void unlock_timer(void)
{
	pthread_mutex_unlock(&vtimer_mutex);
}

static void inthdr_vtimer(int no)
{
	lock_timer();
	flg_interrput_proc = 1;

	VTPRINTF("inthdr_vtimer\n");

	if(syscall_cnt > 1) {
		SYSERR_PRINT("!!! Invalid timer intarrupt %d\n", syscall_cnt);
	}

	if(run_task == 0) {
		goto timer_exit;
	}

	if(inth_func) {
		inth_func(0);
	}

timer_exit:

	flg_interrput_proc = 0;
	unlock_timer();
}

static timer_t tid;
static struct itimerspec itval, stval;

static void start_emutimer(void)
{
	VTPRINTF("start_emutimer interval\n");

	if(timer_settime(tid, 0, &itval, NULL) < 0) {
		perror("timer_settime");
		while(1);
	}
}

void start_vtimer()
{
	start_emutimer();
}

void stop_vtimer(void)
{
	if(timer_settime(tid, 0, &stval, NULL) < 0) {
		perror("timer_settime");
	}
}

/*
  @brief	デバイスドライバ登録
*/
static int timer_register(struct st_device *dev, char *param)
{
	struct sigaction inttimer;

	memset(&inttimer, 0, sizeof(struct sigaction));
	sigemptyset(&inttimer.sa_mask);
	inttimer.sa_handler = inthdr_vtimer;
	inttimer.sa_flags |= SA_RESTART;

	if(sigaction(SIGNAL_TIMER, &inttimer, NULL) != 0) {
		fprintf(stderr, "sigaction(2) error!\n");
	}

	inth_func = 0;

	if(sigaction(SIGNAL_TIMER, &inttimer, NULL) < 0) {
		perror("sigaction()");
		return -1;
	}

	stval.it_value.tv_sec = 0;
	stval.it_value.tv_nsec = 0;
	stval.it_interval.tv_sec = 0;
	stval.it_interval.tv_nsec = 0;

	itval.it_value.tv_sec = 1;	// すぐにタイマを開始すると何故かgdbで正しく動作しない
	itval.it_value.tv_nsec = 0;
	itval.it_interval.tv_sec = 0;
	itval.it_interval.tv_nsec = GSC_KERNEL_TIMER_INTERVAL_MSEC * 1000000;

	if(timer_create(CLOCK_MONOTONIC, NULL, &tid) < 0) {
		perror("timer_create");
		while(1);
	}

	clock_gettime(CLOCK_MONOTONIC_RAW, &vtime_start_time);

	start_vtimer();

	return 0;
}

/*
  @brief	デバイスドライバ削除
*/
static int timer_unregister(struct st_device *dev)
{
	return 0;
}

/*
  @brief	タイマを制御する
  @param[in]	com	コマンド
  @param[in]	arg	コマンド引数
  @return	<0:エラー
*/
static int timer_ioctl(struct st_device *dev, unsigned int com, unsigned int arg, void *param)
{
	switch(com) {
	case IOCMD_TIMER_GETTIME:
		// GSC_KERNEL_TIMER_INTERVAL_MSEC の分解能以下のms単位の時間を返す
		{
			struct timespec nowtime;
			clock_gettime(CLOCK_MONOTONIC_RAW, &nowtime);
			//tkprintf("%d\n", nowtime.tv_nsec);
			return (nowtime.tv_nsec / 1000000) % GSC_KERNEL_TIMER_INTERVAL_MSEC;
		}
		break;

	case IOCMD_TIMER_GETSYSTIME:
		{
			struct timespec nowtime;
			unsigned long long utime;

			clock_gettime(CLOCK_MONOTONIC_RAW, &nowtime);
			nowtime.tv_sec -= vtime_start_time.tv_sec;
			if(nowtime.tv_nsec > vtime_start_time.tv_nsec) {
				nowtime.tv_nsec -= vtime_start_time.tv_nsec;
			} else {
				nowtime.tv_nsec -= vtime_start_time.tv_nsec;
				nowtime.tv_nsec += 1000000000;
				nowtime.tv_sec --;
			}
			//tkprintf("%9d\n", nowtime.tv_sec);
			utime = (nowtime.tv_sec * 1000000) + (nowtime.tv_nsec / 1000);
			*((unsigned long long *)param) = utime;
			return 0;
		}
		break;

	case IOCMD_TIMER_START:
		start_vtimer();
		return 0;
		break;

	case IOCMD_TIMER_STOP:
		stop_vtimer();
		return 0;
		break;

	case IOCMD_TIMER_SETFUNC:
		inth_func = (void (*))param;
		return 0;
		break;

	default:
		SYSERR_PRINT("Unknown ioctl(%08X)\n", com);
		return -1;
	}

	return -1;
}

/*
  @brief	タイマを休止状態にする
  @return	<0:エラー
*/
static int timer_suspend(struct st_device *dev)
{
	return 0;
}

/*
  @brief	タイマを活性化する
  @return	<0:エラー
*/
static int timer_resume(struct st_device *dev)
{
	return 0;
}

const struct st_device timer_device = {
	.name		= DEF_DEV_NAME_TIMER,
	.explan		= "EMU Timer",
	.register_dev	= timer_register,
	.unregister_dev	= timer_unregister,
	.ioctl		= timer_ioctl,
	.suspend	= timer_suspend,
	.resume		= timer_resume,
}; //!< タイマデバイスドライバ
