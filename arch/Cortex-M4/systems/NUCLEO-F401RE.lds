/*
 * STM32F401RE
 */
MEMORY {
	vectors(r)	: ORIGIN = 0x08000000, LENGTH = 0x200
	rom(xwr)	: ORIGIN = 0x08000200, LENGTH = 512k-0x200
	ram(xwr)	: ORIGIN = 0x20000000, LENGTH = 88k
	stack(xwr)	: ORIGIN = 0x20016000, LENGTH = 8k
}

_startof_flash = ORIGIN(rom);
_endof_flash = ORIGIN(rom) + LENGTH(rom);

_startof_sram = ORIGIN(ram);
_endof_sram = ORIGIN(ram) + LENGTH(ram);

_startof_stack = ORIGIN(stack);
_endof_stack = ORIGIN(stack)+LENGTH(stack);

SECTIONS {
	.VECTORS : {
		KEEP(*(vect))
	} >vectors

	.text : { FILL(0xff)
		_text = .;
		PROVIDE(text =	.);
		*(.text .text.*)
		*(.gnu.linkonce.t*)
		*(.gcc_except_table)
		*(.rodata .rodata.*)
		*(.gnu.linkonce.r*)
	} >rom

	.ARM.excep : { FILL(0xff)
		__exidx_start = .;
		*(.ARM.exidx*)
		__exidx_end = .;
		__extab_start = .;
		*(.ARM.extab*)
		__exidx_end = .;
	} >rom

	. = ALIGN(4);
	_etext = .;
	PROVIDE(etext = .);

	_sidata = .;

	.data : AT (_sidata) {	FILL(0xff)
		. = ALIGN(4);
		_sdata = .;
		*(.data .data.*)
		*(.gnu.linkonce.d*)
		. = ALIGN(4);
		_edata = .;
		PROVIDE(edata = .);
	} >ram

_heap_end = ORIGIN(ram)+LENGTH(ram)-4;	/* due to 4byte alignments */

	.bss (NOLOAD) : ALIGN(4) {
		_sbss = .;
		__bss_start = .;
		__bss_start__ = .;
		*(.bss .bss.*)
		*(.gnu.linkonce.b*)
		*(COMMON)
		. = ALIGN(4);
		_ebss = .;
		_heap  = .;
		PROVIDE(heap =	.);
		_bss_end__ = . ;
		__bss_end__ = . ;
		__end__ = . ;
		PROVIDE (end = .);
		_end  = .;
	} >ram

	.stack (NOLOAD) : ALIGN(8) {
		*(.stack)
		*(.STACK)
		. = ALIGN(4);
	} >stack

	mem_start = 0x20016000;
	mem_end   = 0x20018000;
}
