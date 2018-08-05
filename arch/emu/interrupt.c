/** @file
    @brief	仮想割り込みハンドラ

    @date	2009.10.23
    @author	Takashi SHUDO
*/

#include "sysconfig.h"
#include "interrupt.h"
#include "tkprintf.h"
#include "task/tcb.h"
#include "asm-emu.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>

//#define DEBUG
#ifdef DEBUG
#define INPRINTF	printf
#else
#define INPRINTF(x, ...)
#endif

extern struct st_tcb *run_task;

#ifdef DEBUG
int trap_depth = 0;
#endif

volatile int flg_interrput_proc = 0;

void start_vtimer(void);
void stop_vtimer(void);

int is_in_interrupt(void)
{
	return flg_interrput_proc;
}

/*
  異なるスレッド間で setcontext() を実行させないためメインのスレッド以
  外はタイマ割り込み(SIGARM)を受け付けてはならない。そのため、スレッド
  を作成する前に block_timer_interrupt() を実行して SIGARM をマスクし
  ておく必要がある。(作成されたスレッドはマスクの値を引き継ぐ)
*/
int block_timer_interrupt(void)
{
	sigset_t mask, old_mask;
	int res;

	sigemptyset(&mask);
	sigaddset(&mask, SIGNAL_TIMER);

	res = pthread_sigmask(SIG_BLOCK, &mask, &old_mask);

	if(res != 0) {
		perror("trap sigprocmask(SIG_BLOCK)");
	}

	res = sigismember(&old_mask, SIGNAL_TIMER);
	if(res == 0) {
		//printf("New mask\n");	// マスクが消えていた？
	} else if(res == 1) {
		//printf("Mask OK\n");
	} else {
		printf("Mask check error(%d)\n", res);
	}

	return res;
}

void unblock_timer_interrupt(void)
{
	sigset_t mask;
	int res;

	sigemptyset(&mask);
	sigaddset(&mask, SIGNAL_TIMER);

	res = pthread_sigmask(SIG_UNBLOCK, &mask, NULL);

	if(res != 0) {
		perror("trap sigprocmask(SIG_UNBLOCK)");
	}
}

static void trap(int no)
{
	INPRINTF("trap %d SIGNAL IN \"%s\"(%d)\r\n", no, run_task->name);

	syscall_inthdr(0, 0);

	INPRINTF("trap SIGNAL syscall OUT \"%s\"\n", run_task->name);
}

void syscall_trap(void)
{
	block_timer_interrupt();

	trap(0);

	unblock_timer_interrupt();
}

void init_interrupt_vector(void)
{
	INPRINTF("%s\n", __FUNCTION__);
}

int register_interrupt(unsigned short vectnum,
		       void (* func)(unsigned int intnum, void *sp))
{
	return 0;
}

int unregister_interrupt(unsigned short vectnum)
{
	return 0;
}
