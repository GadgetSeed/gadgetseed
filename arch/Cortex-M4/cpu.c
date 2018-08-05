/** @file
    @brief	Cortex-M4 CPU制御

    @date	2013.03.04
    @author	Takashi SHUDO
*/

#include "asm-Cortex-M4.h"
#include "task/task.h"
#include "tkprintf.h"

void setup_task(void *sp, int stack_size, void (* process)(void),
		struct st_tcb *tcb)
{
	union st_regs *regs;

	// 割り込み処理時のレジスタをスタックに待避した状態
	tcb->sp = (void *)((unsigned int)sp + stack_size - sizeof(union st_regs));
	regs = (union st_regs *)tcb->sp;

	regs->name.r0 = (unsigned int)tcb; /* task() 第1引数 */
	regs->name.r1 = 1;
	regs->name.r2 = 2;
	regs->name.r3 = 3;
	regs->name.r4 = 4;
	regs->name.r5 = 5;
	regs->name.r6 = 6;
	regs->name.r7 = 7;
	regs->name.r8 = 8;
	regs->name.r9 = 9;
	regs->name.r10 = 10;
	regs->name.r11 = 11;
	regs->name.r12 = 12;
	regs->name.sp = (unsigned int)tcb->sp;
	regs->name.lr = (unsigned int)process;
	regs->name.ilr = 0xFFFFFFF9;
	regs->name.pc = (unsigned int)process;
	regs->name.xpsr = 0x01000000;	// Thumb状態
}

void disp_regs(void *sp)
{
	union st_regs *regs = (union st_regs *)sp;

	tkprintf("xPSR= %08lX\n", regs->name.xpsr);
	tkprintf("R0  = %08lX    ", regs->name.r0);
	tkprintf("R1  = %08lX    ", regs->name.r1);
	tkprintf("R2  = %08lX    ", regs->name.r2);
	tkprintf("R3  = %08lX    ", regs->name.r3);
	tkprintf("\n");
	tkprintf("R4  = %08lX    ", regs->name.r4);
	tkprintf("R5  = %08lX    ", regs->name.r5);
	tkprintf("R6  = %08lX    ", regs->name.r6);
	tkprintf("R7  = %08lX    ", regs->name.r7);
	tkprintf("\n");
	tkprintf("R8  = %08lX    ", regs->name.r8);
	tkprintf("R9  = %08lX    ", regs->name.r9);
	tkprintf("R10 = %08lX    ", regs->name.r10);
	tkprintf("R11 = %08lX    ", regs->name.r11);
	tkprintf("\n");
	tkprintf("R12 = %08lX    ", regs->name.r12);
	tkprintf("SP  = %08lX    ", regs->name.sp);
	tkprintf("LR  = %08lX    ", regs->name.lr);
	tkprintf("PC  = %08lX    ", regs->name.pc);
	tkprintf("\n");
//	tkprintf("iLR = %08lX    ", regs->name.ilr);
//	tkprintf("\n");
}

//#define NVIC_CCR	(*(volatile unsigned int *)0xE000ED14)
//#define SHCSR (*(volatile unsigned int *)0xE000ED24)
//#define SVCALLACT	(1 << 7)

void dispatch(struct st_tcb *otcb, struct st_tcb *tcb)
{
	extern void _dispatch(void *sp);
//	static int svc_flg = 0;

//	if(svc_flg == 1) {
//		SHCSR &= ~(SVCALLACT);
//	}
//	svc_flg ++;
//	NVIC_CCR |= 0x00000001;

	if(((union st_regs *)(tcb->sp))->name.pc >= 0x20000000) {
		SYSERR_PRINT("Stack Error ?\n");
		disp_task_info();
		disp_regs(tcb->sp);
		disp_debug_info();
		print_queues();
	}

	_dispatch(tcb->sp);
}
