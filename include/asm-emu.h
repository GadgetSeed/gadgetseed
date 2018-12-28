/** @file
    @brief	エミュレータ用制御

    @date	2013.03.03
    @author	Takashi SHUDO
*/

#ifndef ASM_EMU_H
#define ASM_EMU_H

#include <ucontext.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>

#define ENDIAN_LITTLE

#define ATTR_STACK

extern int pause(void);	// <unistd.h>

#define SIGNAL_TIMER	SIGALRM

struct st_context {
	ucontext_t uc;
}; ///< エミュレータ用タスクコンテキスト

#define disable_interrupt()	block_timer_interrupt()
#define enable_interrupt()	unblock_timer_interrupt()
#define sleep_cpu()	do { ualarm(GSC_KERNEL_TIMER_INTERVAL_MSEC*1000, 0); pause(); } while(0)
#define user_mode()

#define	INTNUM_SYSCALL	1	// 割り込み番号(ダミー)

extern void syscall_trap(void);
extern int block_timer_interrupt(void);
extern void unblock_timer_interrupt(void);
extern void lock_timer(void);
extern void unlock_timer(void);

extern void *MEM_START;
extern void *MEM_END;

#endif // ASM_EMU_H
