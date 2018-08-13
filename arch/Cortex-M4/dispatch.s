/*
	Cortex-M4 タスクディスパッチ

	2013.03.04	Takashi SHUDO
*/

	.syntax unified
	.cpu cortex-m4
	.fpu fpv4-sp-d16
	.thumb
	.text
	.align	4

	.global	_dispatch

_dispatch:

	add	r0, #4
	mov	sp, r0
	pop	{r4-r11,pc}

	.end
