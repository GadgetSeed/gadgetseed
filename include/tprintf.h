/** @file
    @brief	機能限定printf

    @date	2002.03.01
    @author	Takashi Shudo
*/

#ifndef TPRINTF_H
#define TPRINTF_H

#include "console.h"

#define UNUSED_VARIABLE(x) (void)(x)

extern int tsprintf(char *str, const char *fmt, ...)__attribute__ ((format(printf, 2, 3)));
extern int tsnprintf(char *str, unsigned int size, const char *fmt, ...)__attribute__ ((format(printf, 3, 4)));
extern int tprintf(const char *fmt, ...)__attribute__ ((format(printf, 1, 2)));
extern int eprintf(const char *fmt, ...)__attribute__ ((format(printf, 1, 2)));
extern void xdump(unsigned char *data, unsigned int len);
extern void xadump(unsigned int addr, unsigned char *data, unsigned int len);

#endif // TPRINTF_H
