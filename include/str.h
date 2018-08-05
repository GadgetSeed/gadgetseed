/** @file
    @brief	文字列処理

    @date	2002.03.02
    @author	Takashi SHUDO
*/

#ifndef	STR_H
#define	STR_H

#include "sysconfig.h"

typedef unsigned char	uchar;	///< GadgetSeedの文字(列)は unsigned char 型となる
typedef unsigned short	ushort;	///< 2バイト(UTF-16)文字

extern uchar * itohs(uchar *str, unsigned int strlen, int val);
extern uchar * lltohs(uchar *str, unsigned int strlen, long long val);

extern int hstoi(uchar *str);
extern unsigned int hstou(uchar *str);

extern uchar * itods(uchar *str, unsigned int strlen, int val);
extern uchar * lltods(uchar *str, unsigned int strlen, long long val);
extern uchar * uitods(uchar *str, unsigned int strlen, unsigned int val);
extern uchar * ulltods(uchar *str, unsigned int strlen, unsigned long long val);
extern uchar * itodsz(uchar *str, unsigned int strlen, int val);
extern uchar * lltodsz(uchar *str, unsigned int strlen, long long val);
extern uchar * uitodsz(uchar *str, unsigned int strlen, unsigned int val);
extern uchar * ulltodsz(uchar *str, unsigned int strlen, unsigned long long val);
extern int dstoi(uchar *str);
extern unsigned int dstou(uchar *str);

extern int strcomp(const uchar *s1, const uchar *s2);
extern int strncomp(const uchar *s1, const uchar *s2, unsigned int n);
extern uchar * strncopy(uchar *dest, const uchar *src, unsigned int n);
extern unsigned int strleng(const uchar *str);
extern uchar * str2cap(uchar *str);
extern int hdstoi(uchar *str);
extern unsigned int hdstou(uchar *str);

extern void * memoryset(void *dest, uchar data, unsigned int count);
extern void * memorycopy(void *dest, const void *src, unsigned int count);
extern uchar *strchar(const uchar *str, uchar ch);

#endif	// STR_H
