/** @file
    @brief	ラインエディタ

    @date	2002.03.30
    @author	Takashi SHUDO

    @info

    機能

    ラインエディタ、ヒストリ対応<br>
    VT100互換のターミナル用<br>

    キー<br>
    ↑		1b 5b 41<br>
    ↓		1b 5b 42<br>
    CTRL-P	10<br>
    CTRL-N	0e		ヒストリー<br>
    ←		1b 5b 44<br>
    →		1b 5b 43	カーソル移動<br>
    CTRL-A	01		カーソルを先頭へ<br>
    CTRL-B	02		カーソルを１文字前へ<br>
    CTRL-D	04		１文字削除<br>
    CTRL-E	05		カーソルを末尾へ<br>
    CTRL-F	06		カーソルを１文字先へ<br>
    CTRL-H	08		１文字前を削除(BS)<br>
    CTRL-K	0b		カーソル以降を削除<br>
*/

#include "lineedit.h"
#include "str.h"
#include "console.h"

#define	BS	0x08
#define	HT	0x09
#define LF	0x0a
#define	CR	0x0d

///< lineedit.stat
enum {
	ESC0,	///< エスケープシーケンス未受信
	ESC1,	///< エスケープシーケンス1バイト目
	ESC2,	///< エスケープシーケンス2バイト目
};

/**
   @brief	lineeditを初期化する

   @param[in]	le	初期化するlineedit
*/
void init_lineedit(struct st_lineedit *le)
{
	int i;

	for(i=0; i<GSC_SHELL_MAX_LINE_COLUMS; i++) {
		le->buf[i] = 0;
	}
	le->cur_pos = 0;
	le->stat = ESC0;
}

/**
   @brief	リターンを受けるまでの１行編集を行う
		１文字づつデータを入れる

   @param[in]	le	編集するlineedit
   @param[in]	ch	入力文字

   @return	編集結果(LER_*)
*/
int do_lineedit(struct st_lineedit *le, uchar ch)
{
	int i, j;

	switch(le->stat) {
	case ESC1:	// エスケープシーケンス２バイト目
		switch(ch) {
		case 0x5b:
			le->stat = ESC2;
			break;

		default:
			le->stat = ESC0;
			break;
		}
		break;

	case ESC2:	// エスケープシーケンス３バイト目
		switch(ch) {
		case 0x41:	// ↑
			le->stat = ESC0;
			return LER_BACKLINE;

		case 0x42:	// ↓
			le->stat = ESC0;
			return LER_NEXTLINE;

		case 0x44:	// ←
			if(le->cur_pos != 0) {
				cputc(BS);
				le->cur_pos --;
			}
			le->stat = ESC0;
			break;

		case 0x43:	// →
			if(le->cur_pos < strleng(le->buf)) {
				cputc(le->buf[le->cur_pos]);
				le->cur_pos ++;
			}
			le->stat = ESC0;
			break;

		default:
			le->stat = ESC0;
			break;
		}
		break;

	default:
		switch(ch) {
		case 0x00:
			// NULL文字(0x00)は無視
			break;

		case 0x01:	// CTRL-A
			for(i=0; i<le->cur_pos; i++) {
				cputc(BS);
			}
			le->cur_pos = 0;
			break;

		case 0x02:	// CTRL-B
			if(le->cur_pos != 0) {
				cputc(BS);
				le->cur_pos --;
			}
			break;

		case 0x03:	// CTRL-C
			break;

		case 0x04:	// CTRL-D
			j = strleng(le->buf);
			if(le->cur_pos < j) {
				for(i=le->cur_pos; i<(j-1); i++) {
					le->buf[i] = le->buf[i+1];
					cputc(le->buf[i]);
				}
				le->buf[j-1] = 0;
				cputc(' ');
				for(i=le->cur_pos; i<j; i++) {
					cputc(BS);
				}
			}
			break;

		case 0x05:	// CTRL-E
			for(i=le->cur_pos; i<strleng(le->buf); i++) {
				cputc(le->buf[i]);
				le->cur_pos ++;
			}
			break;

		case 0x06:	// CTRL-F
			if(le->cur_pos < strleng(le->buf)) {
				cputc(le->buf[le->cur_pos]);
				le->cur_pos ++;
			}
			break;

		case 0x07:
			break;

		case 0x7f:
		case BS:	// バックスペース
			if(le->cur_pos > 0) {
				cputc(BS);
				j = strleng(le->buf);
				for(i=le->cur_pos; i<j; i++) {
					le->buf[i-1] = le->buf[i];
					cputc(le->buf[i]);
				}
				le->buf[j-1] = 0;
				cputc(' ');
				for(i=le->cur_pos; i<j; i++) {
					cputc(BS);
				}
				cputc(BS);
				le->cur_pos --;
			}
			break;

		case HT:	// HT(水平タブ)
			break;

		case LF:	// LF
			break;

		case 0x0b:	// CTRL-K
			cputc(0x1b);
			cputc(0x5b);
			cputc(0x4b);
			j = strleng(le->buf);
			for(i=le->cur_pos; i<j; i++) {
				le->buf[i] = 0;
			}
			break;

		case 0x0c:	// CTRL-L
			break;

		case CR:	// CTRL-M
			cputc(CR);
			cputc(LF);

			return LER_RETURN;

		case 0x0e:	// CTRL-N
			le->stat = ESC0;
			return LER_NEXTLINE;

		case 0x10:	// CTRL-P
			le->stat = ESC0;
			return LER_BACKLINE;

		case 0x11:	// CTRL-Q(X-ON)
			break;

		case 0x13:	// CTRL-S(X-OFF)
			break;

		case 0x0f:
		case 0x12:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1a:
			break;

		case 0x1b:	// エスケープシーケンス
			le->stat = ESC1;
			break;

		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:
			break;

		default:	// 文字入力
			{
				j = strleng(le->buf);

				if(j < GSC_SHELL_MAX_LINE_COLUMS) {
					if(j > 0) {
						for(i=j; i>=(int)le->cur_pos; i--) {
							le->buf[i] = le->buf[i-1];
						}
					}
					le->buf[le->cur_pos] = ch;
					le->buf[j+1] = 0;

					for(i=le->cur_pos; i<=j; i++) {
						cputc(le->buf[i]);
					}

					for(i=le->cur_pos; i<j; i++) {
						cputc(BS);
					}

					le->cur_pos ++;
				}
			}
			break;
		}
	}

	return LER_NOP;
}

/**
   @brief	編集中のエディタを初期化(コマンド実行後にバッファ破棄)

   @param[in]	le	編集するlineedit
*/
void new_lineedit(struct st_lineedit *le)
{
	int i;

	for(i=0; i<GSC_SHELL_MAX_LINE_COLUMS; i++) {
		le->buf[i] = 0;
	}

	le->cur_pos = 0;
	le->stat = ESC0;
}

/**
   @brief	編集文字列を設定する

   @param[in]	le	文字列を設定するlineedit
   @param[in]	str	設定する文字列
*/
void set_str_lineedit(struct st_lineedit *le, uchar *str)
{
	unsigned int i;

	for(i=0; i<le->cur_pos; i++) {
		cputc(BS);
	}
	cputc(0x1b);
	cputc(0x5b);
	cputc(0x4b);

	le->cur_pos = 0;

	for(i=0; i<GSC_SHELL_MAX_LINE_COLUMS; i++) {
		le->buf[i] = str[i];
		if(le->buf[i] != 0) {
			cputc(le->buf[i]);
			le->cur_pos ++;
		} else {
			return;
		}
	}
}

/**
   @brief	表示する

   @param[in]	le	文字列を設定するlineedit
*/
void draw_lineedit(struct st_lineedit *le)
{
	unsigned int i, j;

	for(i=0; i<GSC_SHELL_MAX_LINE_COLUMS; i++) {
		if(le->buf[i] != 0) {
			cputc(le->buf[i]);
		} else {
			break;
		}
	}

	for(j=i; j>le->cur_pos; j--) {
		cputc(BS);
	}
}


void insert_str_lineedit(struct st_lineedit *le, uchar *str, unsigned int len)
{
	unsigned int i;

	for(i=0; i<len; i++) {
		do_lineedit(le, str[i]);
	}
}
