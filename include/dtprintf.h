/** @file
    @brief	アプリケーション(タスク)デバッグ用マクロ

    @date	2017.11.21
    @author	Takashi SHUDO
*/

#ifndef DTPRINTF_H
#define DTPRINTF_H

#include "sysconfig.h"
#include "tprintf.h"

#ifdef DEBUGTBITS
#define DTPRINTF(bits, format, ...)	if(bits & DEBUGTBITS) eprintf(format, ## __VA_ARGS__)
#define DTFPRINTF(bits, format, ...)	if(bits & DEBUGTBITS) eprintf("%s: " format, __FUNCTION__, ## __VA_ARGS__)
#define XDUMP(bits, data, len)	if(bits & DEBUGTBITS) xdump(data, len)
#else
#define DTPRINTF(bits, format, ...)
#define DTFPRINTF(bits, format, ...)
#define XDUMP(bits, data, len)
#endif

#endif // DTPRINTF_H
