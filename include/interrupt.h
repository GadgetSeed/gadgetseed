/** @file
    @brief	割り込みハンドラ

    @date	2007.12.31
    @author	Takashi SHUDO
*/

#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "sysconfig.h"
#include "asm.h"

extern int is_in_interrupt(void);
extern void init_interrupt_vector(void);
extern int register_interrupt(unsigned short vectnum, void (* func)(unsigned int intnum, void *sp));
extern int unregister_interrupt(unsigned short vectnum);
#ifdef GSC_KERNEL_ENABLE_INTERRUPT_COUNT
extern int get_interrupt_count(int intnum);
#endif

extern void syscall_inthdr(unsigned int intnum, void *sp);

#endif // INTERRUPT_H
