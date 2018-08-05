/** @file
    @brief	エミュレータ疑似タスク制御

    @date	2012.12.08
    @author	Takashi SHUDO
*/

#include "asm-emu.h"
#include "task/task.h"
#include "tprintf.h"

#include <stdio.h>

//#define DEBUG
#ifdef DEBUG
#define PCPRINTF	printf
#else
#define PCPRINTF(x, ...)
#endif

static ucontext_t uctx;

void setup_task(void *sp, int stack_size, void (* task)(void),
		struct st_tcb *tcb)
{
	PCPRINTF("setup_task sp=%p, stack_size=%d, task=%p,"
		 " tcb=%p\r\n", sp, stack_size,
		 task, tcb);
	
	if(getcontext(&(tcb->ctx.uc)) == -1) {
		fprintf(stderr, "getcontext error");
	}
	tcb->ctx.uc.uc_stack.ss_sp = sp;
	tcb->ctx.uc.uc_stack.ss_size = stack_size;
	tcb->ctx.uc.uc_link = &uctx;
	makecontext(&(tcb->ctx.uc), (void *)task, 0);
}

void disp_regs(void *sp)
{
	// EMUはレジスタは表示できない
}
