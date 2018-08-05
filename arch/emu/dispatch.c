/** @file
    @brief	エミュレータ用タスク制御

    @date	2012.12.08
    @author	Takashi SHUDO
*/

#include "sysconfig.h"
#include "task/task.h"
#include "tkprintf.h"

#include <stdio.h>

//#define DEBUG
#ifdef DEBUG
#define DPPRINTF	printf
#else
#define DPPRINTF(x, ...)
#endif

#ifdef DEBUG
static int dip_cnt = 0;
#endif

void dispatch(struct st_tcb *otcb, struct st_tcb *tcb)
{
	DPPRINTF("dispatch %d \"%s\" -> \"%s\"\n", dip_cnt,
		 otcb->name, tcb->name);

	if(otcb != tcb) {
#ifdef DEBUG
		dip_cnt ++;
#endif
		unlock_timer();
		swapcontext(&(otcb->ctx.uc), &(tcb->ctx.uc));
	}
}
