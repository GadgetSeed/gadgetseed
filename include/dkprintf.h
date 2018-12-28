/** @file
    @brief	カーネル、ドライバ(非タスク)デバッグ用マクロ

    @date	2017.11.21
    @author	Takashi SHUDO
*/

#ifndef DKPRINTF_H
#define DKPRINTF_H

#include "sysconfig.h"
#include "tkprintf.h"

#ifdef DEBUGKBITS
#include <stdarg.h>
#define DKPRINTF(bits, format, ...)	if(bits & DEBUGKBITS) tkprintf(format, ## __VA_ARGS__)
#define DKFPRINTF(bits, format, ...)	if(bits & DEBUGKBITS) tkprintf("%s: " format, __FUNCTION__, ## __VA_ARGS__)
#define KXDUMP(bits, data, len)	if(bits & DEBUGKBITS) kxdump(data, len)
#define KXBDUMP(bits, data, len)	if(bits & DEBUGKBITS) kxbdump(data, len)
#else
#define DKPRINTF(bits, format, ...)
#define DKFPRINTF(bits, format, ...)
#define KXDUMP(bits, data, len)
#define KXBDUMP(bits, data, len)
#endif

#endif // DKPRINTF_H
