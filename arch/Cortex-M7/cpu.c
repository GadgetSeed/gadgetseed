/** @file
    @brief	Cortex-M7 CPU制御

    @date	2017.01.09
    @author	Takashi SHUDO
*/

#include "asm-Cortex-M7.h"
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

	tkprintf("xPSR= %08X\n", regs->name.xpsr);
	tkprintf("R0  = %08X    R1  = %08X    R2  = %08X    R3  = %08X\n",
		 regs->name.r0, regs->name.r1, regs->name.r2, regs->name.r3);
	tkprintf("R4  = %08X    R5  = %08X    R6  = %08X    R7  = %08X\n",
		 regs->name.r4, regs->name.r5, regs->name.r6, regs->name.r7);
	tkprintf("R8  = %08X    R9  = %08X    R10 = %08X    R11 = %08X\n",
		 regs->name.r8, regs->name.r9, regs->name.r10, regs->name.r11);
	tkprintf("R12 = %08X    SP  = %08X    LR  = %08X    PC  = %08X\n",
		 regs->name.r12, regs->name.sp, regs->name.lr, regs->name.pc);
//	tkprintf("iLR = %08X    ", regs->name.ilr);
//	tkprintf("\n");
}

//#define NVIC_CCR	(*(volatile unsigned int *)0xE000ED14)
//#define SHCSR (*(volatile unsigned int *)0xE000ED24)
//#define SVCALLACT	(1 << 7)

void dispatch(struct st_tcb *otcb, struct st_tcb *tcb)
{
	extern void _dispatch(void *sp);
	union st_regs *regs = (union st_regs *)tcb->sp;
//	static int svc_flg = 0;

//	if(svc_flg == 1) {
//		SHCSR &= ~(SVCALLACT);
//	}
//	svc_flg ++;
//	NVIC_CCR |= 0x00000001;

	if((((void *)(regs->name.sp)) < tcb->stack_addr) ||
	   (((void *)(regs->name.sp)) >= (tcb->stack_addr + tcb->stack_size))) {
		SYSERR_PRINT("Stack Over ERROR\n");
		disp_task_info();
		disp_regs(tcb->sp);
		disp_debug_info();
		print_queues();
		while(1);
	}

	_dispatch(tcb->sp);
}
