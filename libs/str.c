/** @file
    @brief	文字列処理

    @date	2017.11.30
    @date	2002.03.02
    @author	Takashi SHUDO

    @page string_handring 文字列処理

    GadgetSeedは独自の文字列処理APIがあります。

    GadgetSeedは半角文字を unsigned char として処理します。\n
    これは @ref uchar と定義されています。

    文字列処理は str.h で定義されています。

    内部の文字コードは UTF-8 としています。\n
    文字コード処理は @ref character_code を参照して下さい。

    @section hex_string_api 16進数文字列変換API

    include ファイル : str.h

    | API名		| 機能			|
    |:------------------|:----------------------|
    | itohs()		| @copybrief itohs	|
    | lltohs()		| @copybrief lltohs	|
    | hstoi()		| @copybrief hstoi	|
    | hstou()		| @copybrief hstou	|

    @section dec_string_api 10進数文字列変換API

    include ファイル : str.h

    | API名		| 機能			|
    |:------------------|:----------------------|
    | itods()		| @copybrief itods	|
    | lltods()		| @copybrief lltods	|
    | uitods()		| @copybrief uitods	|
    | ulltods()		| @copybrief ulltods	|
    | itodsz()		| @copybrief itodsz	|
    | lltodsz()		| @copybrief lltodsz	|
    | uitodsz()		| @copybrief uitodsz	|
    | ulltodsz()	| @copybrief ulltodsz	|
    | dstoi()		| @copybrief dstoi	|
    | dstou()		| @copybrief dstou	|

    @section string_api 文字列処理API

    include ファイル : str.h

    | API名		| 機能			|
    |:------------------|:----------------------|
    | strcomp()		| @copybrief strcomp	|
    | strncomp()	| @copybrief strncomp	|
    | strncopy()	| @copybrief strncopy	|
    | strleng()		| @copybrief strleng	|
    | str2cap()		| @copybrief str2cap	|
    | hdstoi()		| @copybrief hdstoi	|
    | hdstou()		| @copybrief hdstou	|
    | memoryset()	| @copybrief memoryset	|
    | memorycopy()	| @copybrief memorycopy	|
    | strchar()		| @copybrief strchar	|
*/

#include "str.h"

/*
 *	16進数文字列変換
 */

/**
   @brief	intを16進数字列変換

   valをstrlen桁の16進数字列へ変換してstrへ出力

   @param[out]	str	出力文字列ポインタ
   @param[in]	val	文字列にする値
   @param[in]	strlen	出力文字数(0-8)

   @return	出力文字列ポインタ
*/
uchar * itohs(uchar *str, unsigned int strlen, int val)
{
	unsigned int i;
	unsigned int mask = (unsigned int)(0xf0000000>>((8-strlen)*4));
	unsigned int tmp;

	for(i=0; i<strlen; i++) {
		tmp = (val & mask) >> ((strlen-1-i)<<2);
		if(tmp < 10) {
			*str = tmp + '0';
		} else {
			*str = tmp - 10 + 'a';
		}
		str ++;
		mask >>= 4;
	}
	*str = 0;

	return str;
}

/**
   @brief	long long 16進数字列変換

   valをstrlen桁の16進数字列へ変換してstrへ出力

   @param[out]	str	出力文字列ポインタ
   @param[in]	val	文字列にする値
   @param[in]	strlen	出力文字数(0-8)

   @return	出力文字列ポインタ
*/
uchar * lltohs(uchar *str, unsigned int strlen, long long val)
{
	unsigned int i;
	unsigned long long mask = (unsigned long long)(0xf000000000000000>>((16-strlen)*4));
	unsigned long long tmp;

	for(i=0; i<strlen; i++) {
		tmp = (val & mask) >> ((strlen-1-i)<<2);
		if(tmp < 10) {
			*str = (unsigned char)(tmp + '0');
		} else {
			*str = (unsigned char)(tmp - 10 + 'a');
		}
		str ++;
		mask >>= 4;
	}
	*str = 0;

	return str;
}

/**
   @brief	16進数文字列 int 変換

   strの16進数文字列8文字からintへ変換

   @param[in]	str	16進数文字列

   @return	変換した値
*/
int hstoi(uchar *str)
{
	int rt = 0;

	while(*str != 0) {
		rt <<= 4;
		if((('0' <= *str) && (*str <= '9'))
		   || (('A' <= *str) && (*str <= 'F'))
		   || (('a' <= *str) && (*str <= 'f'))) {
			if(('0' <= *str) && (*str <= '9')) {
				rt += *str - '0';
			} else if(('A' <= *str) && (*str <= 'F')) {
				rt += *str - 'A' + 10;
			} else if(('a' <= *str) && (*str <= 'f')) {
				rt += *str - 'a' + 10;
			}
		}
		str ++;
		if(*str == ';') {	// !!!
			break;
		}
	}

	return rt;
}

/**
   @brief	16進数文字列 unsigned int 変換

   strの16進数文字列8文字からunsigned intへ変換

   @param[in]	str	16進数文字列

   @return	変換した値
*/
unsigned int hstou(uchar *str)
{
	unsigned int rt = 0;

	while(*str != 0) {
		rt <<= 4;
		if((('0' <= *str) && (*str <= '9'))
		   || (('A' <= *str) && (*str <= 'F'))
		   || (('a' <= *str) && (*str <= 'f'))) {
			if(('0' <= *str) && (*str <= '9')) {
				rt += *str - '0';
			} else if(('A' <= *str) && (*str <= 'F')) {
				rt += *str - 'A' + 10;
			} else if(('a' <= *str) && (*str <= 'f')) {
				rt += *str - 'a' + 10;
			}
		}
		str ++;
		if(*str == ';') {	// !!!
			break;
		}
	}

	return rt;
}

/**
   @brief	int 10進数文字列変換

   valをstrlen桁の10進数文字列に変換してstrへ出力

   @param[out]	str	10進数文字列
   @param[in]	strlen	桁数
   @param[in]	val	変換する値

   @return	変換した文字列ポインタ
*/
uchar * itods(uchar *str, unsigned int strlen, int val)
{
	unsigned int i, j;
	int m = 0;

	if(val < 0) {
		val = 0-val;
		m = 1;
	}

	for(i=0; i<strlen; i++) {
		if(val == 0) {
			if(i == 0) {
				str[strlen-1-i] = (uchar)'0';
				i++;
			}
			for(j=0; j<(strlen-i); j++) {
				str[j] = (uchar)' ';
			}
			if(m) {
				if((int)((int)strlen-i-1) >= 0) {
					str[strlen-i-1] = (uchar)'-';
				}
			}
			goto end;
		}
		str[strlen-1-i] = (uchar)((val % 10) + '0');
		val /= 10;
	}

 end:
	str[strlen] = 0;

	return str;
}

/**
   @brief	long long 10進数文字列変換

   valをstrlen桁の10進数文字列に変換してstrへ出力

   @param[out]	str	10進数文字列
   @param[in]	strlen	桁数
   @param[in]	val	変換する値

   @return	変換した文字列ポインタ
*/
uchar * lltods(uchar *str, unsigned int strlen, long long val)
{
	unsigned int i, j;
	int m = 0;

	if(val < 0) {
		val = 0-val;
		m = 1;
	}

	for(i=0; i<strlen; i++) {
		if(val == 0) {
			if(i == 0) {
				str[strlen-1-i] = (uchar)'0';
				i++;
			}
			for(j=0; j<(strlen-i); j++) {
				str[j] = (uchar)' ';
			}
			if(m) {
				if((int)((int)strlen-i-1) >= 0) {
					str[strlen-i-1] = (uchar)'-';
				}
			}
			goto end;
		}
		str[strlen-1-i] = (uchar)((val % 10) + '0');
		val /= 10;
	}

 end:
	str[strlen] = 0;

	return str;
}

/**
   @brief	unsigned int 10進数文字列変換

   valをstrlen桁の10進数文字列に変換してstrへ出力(符号無し)

   @param[out]	str	10進数文字列
   @param[in]	strlen	桁数
   @param[in]	val	変換する値

   @return	変換した文字列ポインタ
*/
uchar * uitods(uchar *str, unsigned int strlen, unsigned int val)
{
	unsigned int i, j;

	for(i=0; i<strlen; i++) {
		if(val == 0) {
			if(i == 0) {
				str[strlen-1-i] = (uchar)'0';
				i++;
			}
			for(j=0; j<(strlen-i); j++) {
				str[j] = (uchar)' ';
			}
			goto end;
		}
		str[strlen-1-i] = (uchar)((val % 10) + '0');
		val /= 10;
	}

 end:
	str[strlen] = 0;

	return str;
}

/**
   @brief	unsigned long long 10進数文字列変換

   valをstrlen桁の10進数文字列に変換してstrへ出力(符号無し)

   @param[out]	str	10進数文字列
   @param[in]	strlen	桁数
   @param[in]	val	変換する値

   @return	変換した文字列ポインタ
*/
uchar * ulltods(uchar *str, unsigned int strlen, unsigned long long val)
{
	unsigned int i, j;

	for(i=0; i<strlen; i++) {
		if(val == 0) {
			if(i == 0) {
				str[strlen-1-i] = (uchar)'0';
				i++;
			}
			for(j=0; j<(strlen-i); j++) {
				str[j] = (uchar)' ';
			}
			goto end;
		}
		str[strlen-1-i] = (uchar)((val % 10) + '0');
		val /= 10;
	}

 end:
	str[strlen] = 0;

	return str;
}

/**
   @brief	int "0"付き10進数文字列変換

   valをstrlen桁の10進数文字列に変換してstrへ出力("0"付き)

   @param[out]	str	10進数文字列
   @param[in]	strlen	桁数
   @param[in]	val	変換する値

   @return	変換した文字列ポインタ
*/
uchar * itodsz(uchar *str, unsigned int strlen, int val)
{
	unsigned int i, j;

	if(val < 0) {
		val = 0-val;
		str[0] = (uchar)'-';
	} else {
		str[0] = (uchar)'0';
	}

	for(i=0; i<strlen; i++) {
		if(val == 0) {
			for(j=1; j<(strlen-i); j++) {
				str[j] = (uchar)'0';
			}
			goto end;
		}
		str[strlen-1-i] = (uchar)((val % 10) + '0');
		val /= 10;
	}

 end:
	str[strlen] = 0;

	return str;
}

/**
   @brief	long long 10進数文字列変換

   valをstrlen桁の10進数文字列に変換してstrへ出力("0"付き)

   @param[out]	str	10進数文字列
   @param[in]	strlen	桁数
   @param[in]	val	変換する値

   @return	変換した文字列ポインタ
*/
uchar * lltodsz(uchar *str, unsigned int strlen, long long val)
{
	unsigned int i, j;

	if(val < 0) {
		val = 0-val;
		str[0] = (uchar)'-';
	} else {
		str[0] = (uchar)'0';
	}

	for(i=0; i<strlen; i++) {
		if(val == 0) {
			for(j=1; j<(strlen-i); j++) {
				str[j] = (uchar)'0';
			}
			goto end;
		}
		str[strlen-1-i] = (uchar)((val % 10) + '0');
		val /= 10;
	}

 end:
	str[strlen] = 0;

	return str;
}

/**
   @brief	unsigned int 符号なし"0"付き10進数文字列変換

   valをstrlen桁の10進数文字列に変換してstrへ出力("0"付き,符号無し)

   @param[out]	str	10進数文字列
   @param[in]	strlen	桁数
   @param[in]	val	変換する値

   @return	変換した文字列ポインタ
*/
uchar * uitodsz(uchar *str, unsigned int strlen, unsigned int val)
{
	unsigned int i, j;

	for(i=0; i<strlen; i++) {
		if(val == 0) {
			for(j=0; j<(strlen-i); j++) {
				str[j] = (uchar)'0';
			}
			goto end;
		}
		str[strlen-1-i] = (val % 10) + (uchar)'0';
		val /= 10;
	}

 end:
	str[strlen] = 0;

	return str;
}

/**
   @brief	unsigned long long 符号なし"0"付き10進数文字列変換

   valをstrlen桁の10進数文字列に変換してstrへ出力("0"付き,符号無し)

   @param[out]	str	10進数文字列
   @param[in]	strlen	桁数
   @param[in]	val	変換する値

   @return	変換した文字列ポインタ
*/
uchar * ulltodsz(uchar *str, unsigned int strlen, unsigned long long val)
{
	unsigned int i, j;

	for(i=0; i<strlen; i++) {
		if(val == 0) {
			for(j=0; j<(strlen-i); j++) {
				str[j] = (uchar)'0';
			}
			goto end;
		}
		str[strlen-1-i] = (uchar)((val % 10) + '0');
		val /= 10;
	}

 end:
	str[strlen] = 0;

	return str;
}

/**
   @brief	10進数文字列 int 変換

   strの10進数文字列からintへ変換

   @param[in]	str		- 10進数文字列

   @return	変換した値
*/
int dstoi(uchar *str)
{
	int rt = 0;
	char m = 0;

	while(*str != 0) {
		if(*str == '-') {
			m = 1;
		}
		rt *= 10;
		if(('0' <= *str) && (*str <= '9')) {
			rt += *str - '0';
		}
		str ++;
		if(*str == ';') {
			break;
		}
	}

	if(m != 0) {
		rt = 0-rt;
	}

	return rt;
}

/**
   @brief	10進数文字列 unsigned int 変換

   strの10進数文字列からunsigned intへ変換

   @param[in]	str		- 10進数文字列

   @return	変換した値
*/
unsigned int dstou(uchar *str)
{
	unsigned int rt = 0;

	while(*str != 0) {
		rt *= 10;
		if(('0' <= *str) && (*str <= '9')) {
			rt += *str - '0';
		}
		str ++;
		if(*str == ';') {
			break;
		}
	}

	return rt;
}

/**
   @brief	文字列比較

   文字列s1とs2を比較して結果を返す

   @return	0:同じ -1:*s1<*s2 1:*s1>*s2
*/
int strcomp(const uchar *s1, const uchar *s2)
{
	do {
		if(*s1 > *s2) {
			return 1;
		} else if(*s1 < *s2) {
			return -1;
		}
		s1 ++;
		s2 ++;
	} while((*s1 != 0) || (*s2 != 0));

	return 0;
}

/**
   @brief	文字列比較(長さ指定あり)

   文字列s1とs2をn文字分比較して結果を返す

   @return	0:同じ -1:*s1<*s2 1:*s1>*s2
*/
int strncomp(const uchar *s1, const uchar *s2, unsigned int n)
{
	while(n) {
		if(*s1 > *s2) {
			return 1;
		} else if(*s1 < *s2) {
			return -1;
		}
		s1 ++;
		s2 ++;
		n --;
	}

	return 0;
}

/**
   @brief	文字列コピー

   文字列srcをdestへn文字分コピー

   @param[out]	dest	コピー先文字列ポインタ
   @param[in]	src	コピー元文字列ポインタ
   @param[in]	n	コピーする文字のバイト数 

   @return	コピー先文字列ポインタ
*/
uchar * strncopy(uchar *dest, const uchar *src, unsigned int n)
{
	while(n) {
		if(*src == 0) {
			break;
		}
		*dest = *src;
		dest ++;
		src ++;
		n --;
	}
	*dest = 0;

	return dest;
}

/**
   @brief	文字列長

   文字列strの長さを返す

   @param[in]	str	文字列のポインタ

   @return	文字列のバイト数
*/
unsigned int strleng(const uchar *str)
{
	unsigned int len = 0;

	while((*str) != 0) {
		len ++;
		str ++;
	}

	return len;
}

/**
   @brief	小文字から大文字へ変換

   文字列strの小文字を大文字にする

   @param[in]	str	文字列のポインタ

   @return	変換した文字列のポインタ
*/
uchar * str2cap(uchar *str)
{
	uchar *p = str;
	while((*str) != 0) {
		if(('a' <= *str) && (*str <= 'z')) {
			*str -= ('a' - 'A');
		}
		str ++;
	}

	return p;
}

/**
   @brief	10進数または16進数文字列 int 変換

   文字列の最初が"#"の場合strの10進数文字列からintへ変換
   それ以外strの16進数文字列8文字からintへ変換

   @param[in]	str	文字列のポインタ

   @return	変換した値
*/
int hdstoi(uchar *str)
{
	if(*str == '#') {
		return dstoi(&str[1]);
	} else {
		return hstoi(str);
	}
}

/**
   @brief	10進数または16進数文字列 unsigned int 変換

   文字列の最初が"#"の場合strの10進数文字列からunsigned intへ変換
   それ以外strの16進数文字列8文字からunsigned intへ変換

   @param[in]	str	文字列のポインタ

   @return	変換した値
*/
unsigned int hdstou(uchar *str)
{
	if(*str == '#') {
		return dstou(&str[1]);
	} else {
		return hstou(str);
	}
}

/**
   @brief	メモリを任意の値に設定

   @param[out]	dest	値を設定されるメモリのポインタ
   @param[in]	data	設定する値
   @param[in]	count	設定するバイト数

   @return	値が設定されたメモリのポインタ
*/
void * memoryset(void *dest, uchar data, unsigned int count)
{
	uchar *p = dest;

	while(count) {
		*p = data;
		p ++;
		count --;
	}

	return dest;
}

/**
   @brief	メモリコピー

   @param[out]	dest	コピー先メモリのポインタ
   @param[in]	src	コピー元メモリのポインタ
   @param[in]	count	設定するバイト数

   @return	コピー先メモリのポインタ
*/
void * memorycopy(void *dest, const void *src, unsigned int count)
{
	uchar *d = dest;
	uchar *s = (uchar *)src;

	while(count) {
		*d = *s;
		d ++;
		s ++;
		count --;
	}

	return dest;
}

/**
    @brief  文字を検索

    @param[in]	str	検索対象文字列
    @param[in]	ch	検出する文字

    @return	検索した文字のポインタ
*/
uchar *strchar(const uchar *str, uchar ch)
{
	const uchar *p = str;

	while(*p != 0) {
		if(*p == 0) {
			return 0;
		} else if(*p == ch) {
			return (uchar *)p;
		}

		p ++;
	}

	return 0;
}
