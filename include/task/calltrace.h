/** @file
    @brief	デバッグ用システムコールトレース

    @date	2015.11.08
    @author	Takashi SHUDO
*/

#ifndef CALLTRACE_H
#define CALLTRACE_H

#include "sysconfig.h"
#include "tcb.h"

#ifdef GSC_KERNEL_ENABLE_CALLTRACE	///< $gsc カーネルシステムコールトレースを有効にする
struct st_call_record {
	unsigned long time;
	struct st_tcb *tcb;
	int syscall;
	int status;
	void *resource;
	int arg;
	int count;
	void *sp;
};

void init_calltrace(void);
void record_calltrace(int syscall, int status, void *resource, int arg, int count, void *sp);
void print_calltrace(void);
#else
#define init_calltrace(...)
#define record_calltrace(...)
#define print_calltrace(...)
#endif

#endif // CALLTRACE_H
