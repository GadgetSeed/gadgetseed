/** @file
    @brief	Cortex-M7

    @date	2017.01.08
    @author	Takashi SHUDO
*/

#ifndef CORTEX_M7_H
#define CORTEX_M7_H

#define ENDIAN_LITTLE

#define MAXVECT	126
#define IRQ_VECT_OFFSET	(16)
#define IRQ2VECT(x)	(x + IRQ_VECT_OFFSET)

/*
  割り込み処理時のスタック状態
  
  割り込みハンドラではスタック状態を以下の構造体と同等にしてから、割り
  込み処理を行うこと
*/
union st_regs {
	unsigned int data[18];
	struct {
		unsigned int sp;	// r13

		unsigned int r4;
		unsigned int r5;
		unsigned int r6;
		unsigned int r7;
		unsigned int r8;
		unsigned int r9;
		unsigned int r10;
		unsigned int r11;
		unsigned int ilr;	// r14

		// 以下は割り込み発生時に自動的にスタックに保存される
		unsigned int r0;
		unsigned int r1;
		unsigned int r2;
		unsigned int r3;
		unsigned int r12;
		unsigned int lr;	// r14
		unsigned int pc;	// r15
		unsigned int xpsr;
	} name;
}; ///< Cortex-M7 MCU レジスタ定義

struct st_context {
	union st_regs *regs;
}; ///< Cortex-M7 用タスクコンテキスト


#define disable_interrupt()	asm volatile("cpsid i");
#define enable_interrupt()	asm volatile("cpsie i");
#define sleep_cpu()		asm volatile("wfi");
//#define sleep_cpu()		asm volatile("wfe");
#define syscall_trap()		asm volatile("svc 0");
#define nop()			asm volatile("nop");

#define INTNUM_SYSCALL	11	// SVC 割り込み番号 #11

#endif // CORTEX_M7_H
