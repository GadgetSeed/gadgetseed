/** @file
    @brief	文字コード

    @date	2018.02.12
    @author	Takashi SHUDO

    @page character_code 文字コード

    GadgetSeed は文字コード UTF-8、SJIS、UTF-16 を変換する為のAPIがあります。

    これらのAPIを使用するには FatFs を有効にする必要があります。

    UTF-16 は BOM無しビッグエンディアンのみ使用できます。


    ---
    @section character_code_api 文字コード変換API

    include ファイル : charcode.h

    | API名			| 機能					|
    |:--------------------------|:--------------------------------------|
    | sjiscode_to_utf16code()	| @copybrief sjiscode_to_utf16code	|
    | sjisstr_to_utf8str()	| @copybrief sjisstr_to_utf8str		|
    | sj2utf8()			| @copybrief sj2utf8			|
    | utf16code_to_utf8code()	| @copybrief utf16code_to_utf8code	|
    | utf16str_to_utf8str()	| @copybrief utf16str_to_utf8str	|
    | utf162utf8()		| @copybrief utf162utf8			|
    | utf8code_to_utf16code()	| @copybrief utf8code_to_utf16code	|
    | utf8str_to_utf16str()	| @copybrief utf8str_to_utf16str	|
*/

#include "sysconfig.h"

#ifdef GSC_COMP_ENABLE_FATFS
#include "ff.h"
#define MAX_CONVERT_LEN	FF_MAX_LFN
#else
#define MAX_CONVERT_LEN	255
#endif

#include "charcode.h"
#include "tkprintf.h"

//#define DEBUGTBITS 0x01
#include "dtprintf.h"


#ifdef GSC_COMP_ENABLE_FATFS
/**
   @brief	SJIS から UTF-16 へ変換

   @param[in]	sjiscode	SJIS文字

   @return	UTF-16文字
*/
ushort sjiscode_to_utf16code(ushort sjiscode)
{
	ushort rt = ff_oem2uni(sjiscode, FF_CODE_PAGE);

	return rt;
}

/**
   @brief	SJIS 文字列から UTF-8 文字列へ変換

   @param[out]	utf8str		UTF-8文字列
   @param[in]	sjiscode	SJIS文字列
   @param[in]	count		出力UTF-8文字列最大長

   @return	UTF-8文字列
*/
uchar * sjisstr_to_utf8str(uchar *utf8str, uchar *sjisstr, unsigned int count)
{
	uchar *rt = utf8str;
	ushort wch, cwch;
	unsigned int l = 0;

	while((*sjisstr) != 0) {
		if(l < count) {
			if(*sjisstr < 0x80) {
				*utf8str = *sjisstr;
				utf8str ++;
				sjisstr ++;
				l ++;
			} else {
				uchar code[3];
				int i, clen;
				wch = (ushort)((((ushort)(*sjisstr)) << 8) + (*(sjisstr+1)));
				sjisstr += 2;

				cwch = sjiscode_to_utf16code(wch);

				clen = utf16code_to_utf8code(code, cwch);

				for(i=0; i<clen; i++) {
					if(l < count) {
						*utf8str = code[i];
						utf8str ++;
						l ++;
					} else {
						// 最後のUTF-8文字がcountに入らない場合、中途半端な文字データになる
						break;
					}
				}
			}
		} else {
			break;
		}
	}

	*utf8str = 0;

	return rt;
}

/**
   @brief	SJIS 文字列から最大文字バイト数固定 UTF-8 文字列へ変換

   @param[in]	sjiscode	SJIS文字列

   @return	UTF-8文字列
*/
uchar * sj2utf8(uchar *sjisstr)
{
	static uchar utf8str[MAX_CONVERT_LEN+1];

	(void)sjisstr_to_utf8str(utf8str, sjisstr, MAX_CONVERT_LEN);

	return &utf8str[0];
}
#endif

/**
   @brief	UTF-16 文字から UTF-8 文字へ変換

   @param[out]	utf8str		UTF-8文字
   @param[in]	utf16code	UTF-16文字

   @return	UTF-8文字バイト数
*/
int utf16code_to_utf8code(uchar *utf8code, ushort utf16code)
{
	int rt = 0;
	uchar code[2];

	code[0] = (uchar)(utf16code >> 8);
	code[1] = (uchar)(utf16code & 0xff);

	if(utf16code < 0x80) {
		// 00000000-0xxxxxxx -> 0xxxxxxx
		utf8code[0] = code[1];
		rt = 1;
	} else if(utf16code < 0x800) {
		// 00000xxx-xxyyyyyy -> 110xxxxx-10yyyyyy
		utf8code[0] = (uchar)0xC0 + ((code[0] & 0x7) << 2) + ((code[1] & 0xC0) >> 6);
		utf8code[1] = (uchar)0x80 + (code[1] & 0x3F);
		rt = 2;
	} else {
		// xxxxyyyy-yyzzzzzz -> 1110xxxx-10yyyyyy-10zzzzzz
		utf8code[2] = (uchar)0x80 + (code[1] & 0x3F);
		utf8code[1] = (uchar)0x80 + ((code[1] & 0xC0) >> 6) + ((code[0] & 0xF) << 2);
		utf8code[0] = (uchar)0xE0 + ((code[0] & 0xF0) >> 4);
		rt = 3;
	}

	return rt;
}

/**
   @brief	UTF-16 文字列から UTF-8 文字列へ変換

   @param[out]	utf8str		UTF-8文字列
   @param[in]	utf16str	UTF-16文字列
   @param[in]	count		最大出力UTF-8文字列バイト数

   @return	UTF-8文字列バイト長
*/
int utf16str_to_utf8str(uchar *utf8str, ushort *utf16str, unsigned int count)
{
	unsigned int len = 0;
	uchar *u8p = utf8str;
	ushort *u16p = utf16str;

	while((*u16p) != 0) {
		if(len < count) {
			if(*u16p < 0x0080) {
				*u8p = (uchar)*u16p;
				u8p ++;
				u16p ++;
				len ++;
			} else {
				uchar code[3];
				int i, clen;

				clen = utf16code_to_utf8code(code, *u16p);
				u16p ++;

				for(i=0; i<clen; i++) {
					if(len < count) {
						*u8p = code[i];
						u8p ++;
						len ++;
					} else {
						// 最後のUTF-8文字がcountに入らない場合、中途半端な文字データになる
						break;
					}
				}
			}
		} else {
			break;
		}
	}

	*u8p = 0;

	XDUMP(0x01, (unsigned char *)utf16str, len);
	XDUMP(0x01, utf8str, len);

	return (int)len;
}

/**
   @brief	UTF-16 文字列から最大文字バイト数固定 UTF-8 文字列へ変換

   @param[in]	utf16str	UTF-16文字列

   @return	UTF-8文字列
*/
uchar * utf162utf8(ushort *utf16str)
{
	static uchar utf8str[MAX_CONVERT_LEN+1];

	(void)utf16str_to_utf8str(utf8str, utf16str, MAX_CONVERT_LEN);

	return &utf8str[0];
}

/**
   @brief	UTF-8 文字から UTF-16 文字へ変換

   @param[out]	utf16code	UTF-16文字
   @param[in]	utf8code	UTF-8文字

   @return	UTF-8文字バイト数
*/
int utf8code_to_utf16code(ushort *utf16code, uchar *utf8code)
{
	if(*utf8code < 0x80) {
		*utf16code = *utf8code;
		return 1;
	} else if((*utf8code & 0xe0) == 0xc0) {
		// 110xxxxx-10yyyyyy -> 00000xxx-xxyyyyyy
		*utf16code = (unsigned short)(((((unsigned short)utf8code[0] & 0x1f) >> 2) << 8) +
					      ((utf8code[0] & 0x03) << 6) +
					      (utf8code[1] & 0x3f));
		return 2;
	} else if((*utf8code & 0xf0) == 0xe0) {
		// 1110xxxx-10yyyyyy-10zzzzzz -> xxxxyyyy-yyzzzzzz
		*utf16code = (unsigned short)(((((unsigned short)utf8code[0] & 0x0f) << 4) << 8) +
					      ((((unsigned short)utf8code[1] & 0x3c) >> 2) << 8) +
					      ((utf8code[1] & 0x03) << 6) +
					      (utf8code[2] & 0x3f));
		return 3;
	} else if((*utf8code & 0xf8) == 0xf0) {
		*utf16code = 0; // 未サポート
		return 4;
	} else if((*utf8code & 0xfc) == 0xf8) {
		*utf16code = 0; // 未サポート
		return 5;
	} else if((*utf8code & 0xfe) == 0xfc) {
		*utf16code = 0; // 未サポート
		return 6;
	} else {
		//SYSERR_PRINT("Unsuport UTF-8 CODE %02X %02X %02X\n", utf8code[0], utf8code[1], utf8code[2]);
		return 0;	// どれにも該当しない
	}
}

/**
   @brief	UTF-8 文字列から UTF-16 文字列へ変換

   @param[out]	utf16str	UTF-16文字列
   @param[in]	utf8str		UTF-8文字列
   @param[in]	count		最大出力UTF-16文字列バイト数

   @return	変換後UTF-16文字列バイト長
*/
int utf8str_to_utf16str(ushort *utf16str, uchar *utf8str, unsigned int count)
{
	ushort cwch;
	unsigned int len = 0;

	while((*utf8str) != 0) {
		if(len < count) {
			if(*utf8str < 0x80) {
				*utf16str = *utf8str;
				utf16str ++;
				utf8str ++;
				len ++;
			} else {
				int clen;

				clen = utf8code_to_utf16code(&cwch, utf8str);
				utf8str += clen;

				if((len+1) < count) {
					if(clen > 1) {
						*utf16str = cwch;
						utf16str ++;
						len ++;
					} else {
						// 最後の文字がcountに入らない場合、中途半端な文字データになる
						break;
					}
				}
			}
		} else {
			break;
		}
	}

	*utf16str = 0;

	return (int)len;
}
