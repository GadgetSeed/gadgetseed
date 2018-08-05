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

	.global	DATAROM_START
	.global	DATARAM_START
	.global	DATARAM_END

	.global	BSS_START
	.global	BSS_END

	.global	INIT_STACK

	.global	SYSCALL_TABLE
	.global	SYSCALL_TABLE_END
	.global	SYSCALL_START

	.align 4

DATAROM_START:
	.long	_sidata

DATARAM_START:
	.long	_sdata

DATARAM_END:
	.long	_edata

BSS_START:
	.long	_sbss

BSS_END:
	.long	_ebss

INIT_STACK:
	.long	_endof_stack

/*
SYSCALL_TABLE:
	.long	_syscall_table

SYSCALL_TABLE_END:
	.long	_syscall_table_end

SYSCALL_START:
	.long	syscall_start
*/

	.end
