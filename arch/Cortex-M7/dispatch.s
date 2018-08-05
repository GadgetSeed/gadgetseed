/*
	Cortex-M7 タスクディスパッチ

	2017.01.09	Takashi SHUDO
*/

	.syntax unified
	.cpu cortex-m7
	.fpu fpv5-d16
	.thumb
	.text
	.align	4

	.global	_dispatch

_dispatch:

	add	r0, #4
	mov	sp, r0
	pop	{r4-r11,pc}

	.end
