/*
 * STM32F746
 */
MEMORY {
	vectors(r)	: ORIGIN = 0x08000000, LENGTH = 0x200
	rom(xwr)	: ORIGIN = 0x08000200, LENGTH = 1024k-0x200
	stack(xwr)	: ORIGIN = 0x20000000, LENGTH =   32k
	ram(xwr)	: ORIGIN = 0x20008000, LENGTH =  304k-32k
	/*mem(xwr)	: ORIGIN = 0x2004C000, LENGTH =   16k*/
	nvram(wr)	: ORIGIN = 0x40024000, LENGTH =    4k
	sdram(xrw)	: ORIGIN = 0xC0000000, LENGTH =    8M
}

_startof_flash = ORIGIN(rom);
_endof_flash = ORIGIN(rom) + LENGTH(rom);

_startof_sram = ORIGIN(ram);
_endof_sram = ORIGIN(ram) + LENGTH(ram);

_startof_nvram = ORIGIN(nvram);
_endof_nvram = ORIGIN(nvram)+LENGTH(nvram);

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
/*		PROVIDE (end = .);
		_end  = .;*/
	} >ram

/*_heap_end = ORIGIN(ram)+LENGTH(ram)-4;*/	/* due to 4byte alignments */

	.nonvolatile (NOLOAD) : ALIGN(4) {
		_snonvolatile = .;
		*(.nonvolatile .nonvolatile.*)
		_enonvolatile = .;
	} >nvram

/*	.RxDescripSection     0x20000000 (NOLOAD) : { *(.RxDescripSection)     } >stack
	.TxDescripSection     0x200000A0 (NOLOAD) : { *(.TxDescripSection)     } >stack
	.RxBUF                0x20000140 (NOLOAD) : { *(.RxBUF)                } >stack
	.TxBUF                0x20001F04 (NOLOAD) : { *(.TxBUF)                } >stack
*/
	.RxDescripSection     0x20000000 (NOLOAD) : { *(.RxDescripSection)     } >stack
	.TxDescripSection     0x20000140 (NOLOAD) : { *(.TxDescripSection)     } >stack
	.RxBUF                0x20000280 (NOLOAD) : { *(.RxBUF)                } >stack
	.TxBUF                0x20004000 (NOLOAD) : { *(.TxBUF)                } >stack

	.stack (NOLOAD) : ALIGN(8) {
		*(.stack)
		*(.STACK)
		. = ALIGN(4);
	} >stack

    /* This is the External SRAM/SDRAM section */
	.extram (NOLOAD) :
	{
		_extram = . ;
		extram = . ;
		PROVIDE (extram = .);
		PROVIDE (_extram = .);
		*(.extram .extram.*)  
		*(.EXTRAM)
		. = ALIGN(4);
		PROVIDE (end = .);
		_end  = .;
	} >sdram

_heap_end = ORIGIN(sdram)+LENGTH(sdram)-4;	/* due to 4byte alignments */
}
