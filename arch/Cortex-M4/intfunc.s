# STM32F40x Interrupt Function
# 
# 2013.02.02	Takashi SHUDO

	.syntax unified
	.cpu cortex-m4
#	.eabi_attribute 27, 3
#	.fpu fpv4-sp-d16
#	.eabi_attribute 20, 1
#	.eabi_attribute 21, 1
#	.eabi_attribute 23, 3
#	.eabi_attribute 24, 1
#	.eabi_attribute 25, 1
#	.eabi_attribute 26, 1
#	.eabi_attribute 30, 2
#	.eabi_attribute 34, 1
#	.eabi_attribute 18, 4
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
