/** @file
    @brief	文字コード処理

    @date	2018.02.12
    @author	Takashi SHUDO
*/

#ifndef	CHARCODE_H
#define	CHARCODE_H

#include "str.h"

extern ushort sjiscode_to_utf16code(ushort sjiscode);
extern uchar *sjisstr_to_utf8str(uchar *utf8str, uchar *sjisstr, unsigned int count);
extern uchar *sj2utf8(uchar *sjisstr);

extern int utf16code_to_utf8code(uchar *utf8code, ushort utf16code);
extern int utf16str_to_utf8str(uchar *utf8str, ushort *utf16str, unsigned int count);
extern uchar *utf162utf8(ushort *utf16str);

extern int utf8code_to_utf16code(ushort *utf16code, uchar *utf8code);
extern int utf8str_to_utf16str(ushort *utf16str, uchar *utf8str, unsigned int count);

#endif // CHARCODE_H
