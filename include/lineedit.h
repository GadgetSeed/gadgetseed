/** @file
    @brief	ラインエディタ

    @date	2002.03.30
    @author	Takashi SHUDO
*/

#ifndef	LINEEDIT_H
#define	LINEEDIT_H

#include "sysconfig.h"
#include "str.h"
#include "console.h"

#ifndef GSC_SHELL_MAX_LINE_COLUMS
#define	GSC_SHELL_MAX_LINE_COLUMS		255	///< $gsc shellコマンドラインの最大文字数
#endif

// ASCIIコード
#define ASCII_CTRL_C	0x03	// CTRL + C
#define ASCII_CTRL_D	0x04	// CTRL + D
#define	ASCII_HT	0x09	// HT(水平タブ)
#define ASCII_LF	0x0a	// LF
#define	ASCII_CR	0x0d	// CR
#define	ASCII_XOFF	0x11
#define	ASCII_XON	0x13

// ヒストリを使用する場合の使用するメモリ=((SHELL_MAX_COM_HIS+2) * GSC_SHELL_MAX_LINE_COLUMS)

struct st_lineedit {
	uchar	buf[GSC_SHELL_MAX_LINE_COLUMS+1];	///< 編集中文字列
	unsigned int	cur_pos;	///< カーソル位置
	int	stat;			///< エスケープシーケンスデコード状態
}; ///< ラインエディタ

// do_lineedit()の戻り値
#define LER_NOP		0
#define LER_RETURN	1
#define LER_BACKLINE	2
#define LER_NEXTLINE	3

void init_lineedit(struct st_lineedit *le);
int do_lineedit(struct st_lineedit *le, uchar ch);
void new_lineedit(struct st_lineedit *le);
void set_str_lineedit(struct st_lineedit *le, uchar *str);
void draw_lineedit(struct st_lineedit *le);
void insert_str_lineedit(struct st_lineedit *le, uchar *str, unsigned int len);

#endif	// LINEEDIT_H
