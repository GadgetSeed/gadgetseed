# STM32F7xx Interrupt Function
# 
# 2017.01.09	Takashi SHUDO

	.syntax unified
	.cpu cortex-m7
	.fpu fpv5-d16
	.thumb
	.text
	.align	2

	.extern inthdr_table
	.extern	interrupt_func	@ !!!
	.extern	fault_inthdr
	.global inthdr_func
	.global inthdr_fault

# 仮想ベクタ判定
inthdr_func:
	push	{r4-r11,lr}	@ R4-R11,LRをスタックに保存
	mov	r1, sp
	push	{r1}		@ SPをスタックに保存

	mov	r1, r0		@ R0=割り込み番号(第1引数)
#	lsl	r1, #2		@ ベクタ番号からテーブルを求める
#	ldr	r2, vtbl
#	add	r1, r2
#	ldr	r2, [r1, #0]	@ R2=とび先

	mov	r1, sp		@ R1=SP(第2引数)

	blx	interrupt_func	@ !!!
#!!!	blx	r2		@ ユーザ割りこみ処理へ

inthdr_func_end:
	pop	{r0}
	mov	sp, r0		@ SPをスタックから戻す

	pop	{r4-r11,pc}	@ R4-R11,PCをスタックから戻す

#vtbl:
#	.word	inthdr_table

inthdr_fault:
	push	{r4-r11,lr}	@ R4-R11,LRをスタックに保存
	mov	r1, sp
	push	{r1}		@ SPをスタックに保存

	mov	r1, #3		@ R0=割り込み番号(第1引数)
	mov	r1, sp		@ R1=SP(第2引数)
	
	blx	fault_inthdr
	
	.end
