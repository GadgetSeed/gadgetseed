/** @file
    @brief	機能限定printf

    @date	2007.03.10
    @date	2002.03.02
    @author	Takashi SHUDO
*/

#ifndef VTPRINTF_H
#define VTPRINTF_H

#include <stdarg.h>
#include "console.h"

#define MAXFORMATSTR	((unsigned int)24)	// 1引数デコードの最大文字列(%n?:のnの最大数)

extern int vtprintf(io_write write, const char *fmt, unsigned int size, va_list args);

#define XDUMP_ADDR_ANY_WORD	0
#define XDUMP_ADDR_ANY_LONG	1
#define XDUMP_ADDR_DATA_ADDR	2

#define XDUMP_DATA_BYTE	0
#define XDUMP_DATA_WORD	1
#define XDUMP_DATA_LONG	2

extern void vxdump(unsigned int addr, unsigned char *data, unsigned int len,
		   int addr_type,	/* XDUMP_ADDR_*	*/
		   int data_size,	/* XDUMP_DATA_*	*/
		   int(* print)(const char *fmt, ...));

#endif // VTPRINTF_H
