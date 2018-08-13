/*
	Cortex-M3 タスクディスパッチ

	2018.08.13	Takashi SHUDO
*/

	.syntax unified
	.cpu cortex-m3
	.thumb
	.text
	.align	4

	.global	_dispatch

_dispatch:

	add	r0, #4
	mov	sp, r0
	pop	{r4-r11,pc}

	.end
