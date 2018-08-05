/** @file
    @brief	機能限定printf

    @date	2007.03.10
    @data	2002.03.02
    @author	Takashi SHUDO
*/

#include "console.h"
#include "vtprintf.h"
#include "tprintf.h"

static char *sp;

static int str_puts(unsigned char *data, unsigned int len)
{
	unsigned int i;

	for(i=0; i<len; i++) {
		*sp = (char)*data;
		sp ++;
		data ++;
	}

	return (int)len;
}

/**
   @brief	簡易sprintf

   @param[out]	str	出力文字列ポインタ
   @param[in]	fmt	出力フォーマット文字列

   @return	出力文字列バイト数
*/
int tsprintf(char *str, const char *fmt, ...)
{
	va_list	args;
	int len = 0;
	sp = str;

	va_start(args, fmt);
	len += vtprintf(str_puts, fmt, 0, args);
	va_end(args);

	*sp = 0;

	return len;
}

/**
   @brief	簡易snprintf

   @param[out]	str	出力文字列ポインタ
   @param[in]	str	出力文字列バイト数
   @param[in]	fmt	出力フォーマット文字列

   @return	出力文字列バイト数
*/
int tsnprintf(char *str, unsigned int size, const char *fmt, ...)
{
	va_list	args;
	int len = 0;
	sp = str;
	size --;

	va_start(args, fmt);
	len += vtprintf(str_puts, fmt, size, args);
	size -= len;
	va_end(args);

	*sp = 0;
	len ++;

	return len;
}

/**
   @brief	簡易printf

   @param[in]	fmt	出力フォーマット文字列

   @return	出力文字列バイト数
*/
int tprintf(const char *fmt, ...)
{
	va_list	args;
	int len = 0;

	va_start(args, fmt);
	len += vtprintf(cputs, fmt, 0, args);
	va_end(args);

	return len;
}

/**
   @brief	エラー出力用簡易printf

   @param[in]	fmt	出力フォーマット文字列

   @return	出力文字列バイト数
*/
int eprintf(const char *fmt, ...)
{
	va_list	args;
	int len = 0;

	va_start(args, fmt);
	len += vtprintf(eputs, fmt, 0, args);
	va_end(args);

	return len;
}

void xdump(unsigned char *data, unsigned int len)
{
	vxdump(0, data, len, XDUMP_ADDR_ANY_WORD, XDUMP_DATA_BYTE, tprintf);
}

void xadump(unsigned int addr, unsigned char *data, unsigned int len)
{
	vxdump(addr, data, len, XDUMP_ADDR_ANY_WORD, XDUMP_DATA_BYTE, tprintf);
}
