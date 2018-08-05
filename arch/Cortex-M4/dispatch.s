/*
 *	Cortex-M4 タスクディスパッチ
 *
 *	2013.03.04	Takashi SHUDO	 
 */

	.syntax unified
	.cpu cortex-m4
	.eabi_attribute 27, 3
	.fpu fpv4-sp-d16
	.eabi_attribute 20, 1
	.eabi_attribute 21, 1
	.eabi_attribute 23, 3
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 1
	.eabi_attribute 30, 2
	.eabi_attribute 34, 1
	.eabi_attribute 18, 4
	.thumb
	.text
	.align	4

	.global	_dispatch

_dispatch:

	add	r0, #4
	mov	sp, r0
	pop	{r4-r11,pc}

	.end
