/** @file
    @brief	フォント

    @date	2007.03.25
    @author	Takashi SHUDO

    @page font_draw フォント描画

    GadgetSeedは表示デバイスへの文字フォント描画機能があります。\n
    フォントは複数の種類を持つことができます。\n
    またそれぞれのフォントは異なる文字の大きさやデザインを定義することができます。\n
    GadgeeSeedでは選択できるフォントの単位をフォントセット @ref st_fontset として定義しています。

    フォント描画を使用するには、以下のコンフィグ項目を有効にして下さい。

    * COMP_ENABLE_FONTS


    ---
    @section フォントセット

    GadgetSeedは使用するフォントセットを @ref configration で選択することができます。

    @subsection	フォントセット構造体

    フォントセットは以下の構造体で定義されます。

    @ref st_fontset @copybrief st_fontset

    フォントセット構造体は１種類のフォントセットを定義するデータ構造です。\n
    フォントセット構造体は以下の組み合わせのフォントデータ @ref st_font を定義することが出来ます。\n

    - 半角フォントデータのみ
    - 半角フォントデータと全角フォントデータ

    それぞれのフォントデータは以下のように定義しています。

    - 半角フォントデータ - 文字コードが1バイトで0x00 - 0xFFの文字
    - 全角フォントデータ - 文字コードが2バイト以上の文字

    全角フォントは主に漢字文字を示しています。

    @subsection	フォントデータ構造体

    フォントデータは以下の構造体で定義されています。

    @ref st_font @copybrief st_font


    ---
    @section	文字描画API

    include ファイル : font.h

    | API名			| 機能					|
    |:--------------------------|:--------------------------------------|
    | set_fontset()		| @copybrief set_fontset		|
    | get_fontset()		| @copybrief get_fontset		|
    | fontset_count()		| @copybrief fontset_count		|
    | fontset_name()		| @copybrief fontset_name		|
    | get_fontptr_by_name()	| @copybrief get_fontptr_by_name	|
    | set_font_by_name()	| @copybrief set_font_by_name		|
    | set_font_drawmode()	| @copybrief set_font_drawmode		|
    | draw_char()		| @copybrief draw_char			|
    | draw_str()		| @copybrief draw_str			|
    | draw_fixed_width_str()	| @copybrief draw_fixed_width_str	|
    | draw_str_in_box()		| @copybrief draw_str_in_box		|
    | font_width()		| @copybrief font_width			|
    | str_width()		| @copybrief str_width			|
    | font_height()		| @copybrief font_height		|


    ---
    @section フォントデータの作成

    @subsection テキストデータからのフォントデータ作成

    以下のような文字イメージを記載したテキストデータからフォントデータのCソースを作成するツールを使用することができます。

        # 0x41 0 8 A
        . . X X X . . .
        . X X X X X . .
        X X X . X X X .
        X X . . . X X .
        X X . . . X X .
        X X . . . X X .
        X X . . . X X .
        X X X X X X X .
        X X X X X X X .
        X X . . . X X .
        X X . . . X X .
        X X . . . X X .
        X X . . . X X .
        X X . . . X X .
        X X . . . X X .
        . . . . . . . .

    "8x16"フォントは以下のテキストデータを使用して作成されています。

        fontdata/8x16/font_8x16.txt

    テキストデータからのCソースデータの作成手順は以下のMakefileを参照してください。

        fontdata/8x16/Makefile
        fontdata/txt2font.mk

    @subsection BDFフォントからのフォントデータ作成

    BDFフォントからフォントデータを作成することができます。

    "jiskan16"フォントはBDFフォントのjiskan16より作成されています。

    BDFフォントからのCソースデータの作成手順は以下のMakefileを参照してください。

        fontdata/jiskan16/Makefile
        fontdata/bdf2dbfont.mk

    @subsection OTF、TTFフォントからのフォントデータ作成

    OTFフォントやTTFフォントからフォントデータを作成することができます。
    これは、実験的な試みです。
*/

#include "font.h"
#include "graphics.h"
#include "str.h"
#include "tkprintf.h"
#include "charcode.h"

//#define DEBUGTBITS 0x01
#include "dtprintf.h"


extern const struct st_fontset * const * const font_list[];

static struct st_fontset *now_fontset;
static int font_dmode;

void init_font(void)
{
	font_dmode = FONT_FIXEDWIDTH;
	now_fontset = (struct st_fontset *)*font_list[0];
}

/**
   @brief	描画に使用するフォントセット(カレントフォントセット)を設定する

   @param[in]	fontset	描画に使用するフォントセット
*/
void set_fontset(struct st_fontset *fontset)
{
	now_fontset = fontset;
}

/**
   @brief	描画に使用されているフォントセット(カレントフォントセット)取得する

   @return	描画に使用するフォントセット
*/
struct st_fontset * get_fontset(void)
{
	return now_fontset;
}

/**
   @brief	登録されているフォントセットの数を取得する

   @return	フォントセットの数
*/
int fontset_count(void)
{
	int i = 0;

	while(font_list[i]) {
		i++;
	}

	return i;
}

/**
   @brief	フォント名を取得する

   @param[in]	num	フォント番号

   @return	フォント名
*/
const char *fontset_name(int num)
{
	return (*font_list[num])->name;
}

/**
   @brief	フォントセットのポインタを取得する

   @param[in]	name	フォント名

   @return	フォントセットポインタ
*/
struct st_fontset *get_fontptr_by_name(char *name)
{
	int i = 0;

	DTFPRINTF(0x01, "name = %s\n", name);

	while(font_list[i]) {
		DTPRINTF(0x01, "Fined %s\n", (*font_list[i])->name);
		if(strcomp((unsigned char *)name, (unsigned char *)((*font_list[i])->name)) == 0) {
			return (struct st_fontset *)*font_list[i];
		}
		i++;
	}

	SYSERR_PRINT("Cannot find fontname \"%s\"\n", name);

	return 0;
}

/**
   @brief	カレントフォントセットをフォント名で設定する

   @param[in]	name	フォント名

   @return	カレントフォントセットポインタ
*/
struct st_fontset *set_font_by_name(char *name)
{
	struct st_fontset *font;

	font = get_fontptr_by_name(name);

	if(font != 0) {
		now_fontset = font;
		return now_fontset;
	}

	return 0;
}

/**
   @brief	フォント描画モードを設定する

   @param[in]	mode	フォント描画モード

   @info	mode は @ref FONT_FIXEDWIDTH または @ref FONT_PROPORTIONAL が設定可能
*/
void set_font_drawmode(int mode)
{
	font_dmode = mode;
}

static void draw_nofont(short x, short y, struct st_font *ft)
{
	struct st_rect rect;

	rect.left = x;
	rect.top = y;
	rect.right = x + ft->width;
	rect.bottom = y + ft->height;

	set_draw_mode(GRP_DRAWMODE_REVERSE);
	draw_fill_rect(&rect);
	set_draw_mode(GRP_DRAWMODE_NORMAL);
	draw_rect(&rect);
	draw_line(x, y, rect.right, rect.bottom);
}

static struct st_font *get_fontdata(unsigned short code)
{
	struct st_font *ft = 0;

	if(code <= 0xff) {
		ft = now_fontset->font;
	} else {
#ifdef GSC_FONTS_ENABLE_KANJI	// $gsc 漢字フォントの描画を有効にする
		if(now_fontset->w_font != 0) {
			ft = now_fontset->w_font;
		} else {
			ft = now_fontset->font;
		}
#endif
	}

	return ft;
}

static signed char *get_char_bitmap(unsigned short code, struct st_font *ft)
{
	signed char *fontp = 0;
	int cseg = (code >> 8);
	int start, end;
	int lcode = (code & 0xff);
	unsigned int **indexp, *index;
	unsigned int offset;

	DTPRINTF(0x01, "SEG : %d %04X\n", cseg, code);

	indexp = (unsigned int **)ft->index;
	index = indexp[cseg];
	DTPRINTF(0x01, "INDEX : %p\n", index);
	if(index == 0) {
		return 0;
	}

	start = index[0];
	end = index[1];

	DTPRINTF(0x01, "START : %08X\n", start);
	DTPRINTF(0x01, "END   : %08X\n", end);

	if((lcode < start) || (lcode > end)) {
		fontp = 0;
	} else {
		offset = index[lcode - start + 2];	// 最初の2つはstart,endデータ
		DTPRINTF(0x01, "OFFSET : %08X\n", offset);
		if(offset != 0xffffffff) {
			fontp = &(ft->bitmap[offset]);
		} else {
			fontp = 0;
		}
	}

	return fontp;
}

#define MAX_FONT_WIDTH	40
#define MAX_FONT_HEIGHT	40

static unsigned char font_bitmap[(MAX_FONT_WIDTH/8) * MAX_FONT_HEIGHT];

static unsigned char *create_font_bitmap(signed char *fdata, short fwidth, short fheight)
{
	short dwidth = 0;
	short width = 0;
	short height = 0;
	short ox = 0;
	short oy = 0;
	unsigned char *ddata, *sdata;
	int top = 0;
	int dlbytes = 0;
	int slbytes = 0;
	int i, j;

	dwidth	= (short)(*(fdata + 0));
	width	= (short)(*(fdata + 1));
	height	= (short)(*(fdata + 2));
	ox	= (short)(*(fdata + 3));
	oy	= (short)(*(fdata + 4));

	if((fwidth == width) && (ox == 0) && (oy == 0)) {
		DTPRINTF(0x01, "No need to create font bitmap data\n");
		return (unsigned char *)(fdata + 5);
	}

	ddata = &font_bitmap[0];
	sdata = (unsigned char *)(fdata + 5);
	slbytes = (width+7)/8;
	dlbytes = (dwidth+7)/8;

	DTPRINTF(0x01, "Font height %d\n", fheight);
	DTPRINTF(0x01, "dwidth : %d\n", dwidth);
	DTPRINTF(0x01, "width  : %d\n", width);
	DTPRINTF(0x01, "height : %d\n", height);
	DTPRINTF(0x01, "slbytes: %d\n", slbytes);
	DTPRINTF(0x01, "dlbytes: %d\n", dlbytes);
	DTPRINTF(0x01, "ox     : %d\n", ox);
	DTPRINTF(0x01, "oy     : %d\n", oy);

	top = fheight-height-oy;
	DTPRINTF(0x01, "TOP : %d\n", top);

	// 上の空白
	for(i=0; i<(top * dlbytes); i++) {
		*ddata = 0;
		ddata ++;
	}

	// ビットマップデータの作成
	for(i=0; i<height; i++) {
		unsigned char *dp, *sp;
		dp = ddata;
		sp = sdata;
		*dp = 0;
		*(dp+1) = 0;
		*(dp+2) = 0;
		*(dp+3) = 0; // 32(ドット)以上のoxになるとゴミが出る
		for(j=0; j<slbytes; j++) {
			if(ox != 0) {
				if(ox > 0) {
					*(dp + (ox/8)) |= ((*sp) >> (ox & 7));
					if((ox & 7) != 0) {
						*(dp + (ox/8) + 1) = ((*sp) << (8 - (ox & 7)));
					}
				} else {
					short aox = (0 - ox);
					*(dp + (aox/8)) |= ((*sp) << (aox & 7));
					if((aox & 7) != 0) {
						*(dp + (aox/8) + 1) = ((*sp) >> (8 - (aox & 7)));
					}
				}
			} else {
				*dp = *sp;
			}
			dp ++;
			sp ++;
		}
		ddata += dlbytes;
		sdata += slbytes;
	}

	// 下の空白
	if((top+height) < fheight) {
		int count = (fheight-(top+height)) * dlbytes; 
		for(i=0; i<count; i++) {
			*ddata = 0;
			ddata ++;
		}
	}

	XDUMP(0x02, (unsigned char *)(fdata + 5), slbytes * height);
	XDUMP(0x02, font_bitmap, dlbytes * fheight);

	return font_bitmap;
}

/**
   @brief	文字を描画する

   @param[in]	x	描画X座標
   @param[in]	y	描画Y座標
   @param[in]	ch	描画文字コード(UTF-16)
*/
unsigned short draw_char(short x, short y, unsigned short ch)
{
	signed char *ip;
	unsigned char *fp;
	struct st_font *ft;
	short dwidth = 0;

	DTFPRINTF(0x02, "CH : %04X\n", ch);
#ifdef DEBUGTBITS
	{
		unsigned char str[4] =  {0, 0, 0, 0};
		utf16code_to_utf8code(str, ch);
		DTPRINTF(0x01, "CH : \"%s\"\n", str);
	}
#endif

	ft = get_fontdata(ch);
	if(ft == 0) {
		return 0;
	}

	DTPRINTF(0x02, "start - end : %04X - %04X\n", ft->start, ft->end);

	ip = get_char_bitmap(ch, ft);
	if(ip == 0) {
		DTPRINTF(0x04, "No font ch 0x%04x\n", ch);
		draw_nofont(x, y, ft);
		return (unsigned short)ft->width;
	}

	dwidth	= (short)(*(ip + 0));

	fp = create_font_bitmap(ip, ft->width, ft->height);
	draw_bitdata(x, y, dwidth, ft->height, fp, (dwidth+7)/8);

#if DEBUGTBITS > 0
	{
		struct st_box fbox;
		unsigned int lcolor;

		fbox.pos.x = x;
		fbox.pos.y = y;
		//fbox.sur.width = ft->width;
		fbox.sur.width = (*(ip + 0));
		fbox.sur.height = ft->height;
		lcolor = get_forecolor();
		set_forecolor(RGB(255,0,0));
		draw_box(&fbox);
		set_forecolor(lcolor);
	}
#endif
	return (unsigned short)dwidth;
}

/**
   @brief	文字列を描画する

   @param[in]	x	描画X座標
   @param[in]	y	描画Y座標
   @param[in]	ch	描画文字列(UTF-16)
*/
void draw_str(short x, short y, uchar *str)
{
	ushort code;
	int len = 0;

	while((*str) != 0) {
		XDUMP(0x02, str, 3);
#ifdef GSC_FONTS_ENABLE_KANJI
		len = utf8code_to_utf16code(&code, str);
		DTFPRINTF(0x02, "0x%04X\n", (int)code);
#else
		code = *str;
		len = 1;
#endif
		x += draw_char(x, y, code);
		str += len;
	}
}

/**
   @brief	固定幅で文字列を描画する

   @param[in]	x	描画X座標
   @param[in]	y	描画Y座標
   @param[in]	ch	描画文字列(UTF-16)
*/
void draw_fixed_width_str(short x, short y, uchar *str, short width)
{
	ushort code;
	int len = 0;
	int fwidth = 0, dwidth = 0;

	while((*str) != 0) {
		XDUMP(0x02, str, 3);
#ifdef GSC_FONTS_ENABLE_KANJI
		len = utf8code_to_utf16code(&code, str);
		DTFPRINTF(0x02, "0x%04X\n", (int)code);
#else
		code = *str;
		len = 1;
#endif
		fwidth = font_width(code);
		if((dwidth + fwidth) <= width) {
			dwidth += fwidth;
			x += draw_char(x, y, code);
			str += len;
		} else {
			// 全てかけない文字は描かない [TODO]欠けても文字を描く
			break;
		}
	}

	DTFPRINTF(0x01, "dwidth=%d width=%d\n", dwidth, width);
	if(dwidth < width) {
		int penmode;
		struct st_box box;
		box.pos.x = x;
		box.pos.y = y;
		box.sur.width = width - dwidth;
		box.sur.height = font_height();
		penmode = get_draw_mode();
		set_draw_mode(GRP_DRAWMODE_REVERSE);
		DTPRINTF(0x01, "BOX X=%d Y=%d W=%d H=%d\n",
			 box.pos.x, box.pos.y,
			 box.sur.width, box.sur.height);
		draw_fill_box(&box);
		set_draw_mode(penmode);
	}
}

#if 0 // [TODO]
void draw_wstr(short x, short y, ushort *wstr)
{
	ushort code;

	while((*wstr) != 0) {
		code = *wstr;
		DTFPRINTF(0x02, "0x%04X\n", code);
		x += draw_char(x, y, code);
		wstr ++;
	}
}
#endif

/**
   @brief	四角形内に文字列を描画する

   @param[in]	box	描画範囲四角形
   @param[in]	hattr	横方向属性
   @param[in]	vattr	縦方向属性
   @param[in]	ch	描画文字列(UTF-16)

   @info hattr は @ref FONT_HATTR_LEFT または @ref FONT_HATTR_CENTER または @ref FONT_HATTR_RIGHT が設定可能
   @info vattr は @ref FONT_VATTR_TOP または @ref FONT_VATTR_CENTER または @ref FONT_VATTR_BOTTOM が設定可能
*/
void draw_str_in_box(struct st_box *box, int hattr, int vattr, unsigned char *str)
{
	int str_w = 0, str_h = 0;
	short x = 0, y = 0;

	str_w = str_width(str);
	str_h = font_height();

	DTPRINTF(0x02, "X= %d, Y= %d, W= %d, H= %d\n", box->pos.x, box->pos.y, box->sur.width, box->sur.height);
	DTPRINTF(0x02, "STR_W= %d, STR_H= %d\n", str_w, str_h);

	switch(hattr) {
	case FONT_HATTR_LEFT:
		x = box->pos.x;
		break;

	case FONT_HATTR_CENTER:
		x = box->pos.x + (box->sur.width/2) - (str_w/2);
		break;

	case FONT_HATTR_RIGHT:
		x = box->pos.x + box->sur.width - str_w;
		break;

	default:
		SYSERR_PRINT("Unknow hattr %d\n", hattr);
		break;
	}

	switch(vattr) {
	case FONT_VATTR_TOP:
		y = box->pos.y;
		break;

	case FONT_VATTR_CENTER:
		y = box->pos.y + (box->sur.height/2) - (str_h/2);
		break;

	case FONT_VATTR_BOTTOM:
		y = box->pos.y + box->sur.height - str_h;
		break;

	default:
		SYSERR_PRINT("Unknow vattr %d\n", vattr);
		break;
	}

	DTPRINTF(0x02, "DX= %d, DY= %d\n", x, y);

	draw_str(x, y, str);
}

/**
   @brief	カレントフォントセットの文字幅を取得する

   @param	ch	文字幅を取得する文字の文字コード(UTF-16)

   @return	文字幅
*/
unsigned short font_width(unsigned short ch)
{
	signed char *ip;
	struct st_font *ft;
	unsigned short rt = 0;

	ft = get_fontdata(ch);
	if(ft == 0) {
		return 0;
	}
	ip = get_char_bitmap(ch, ft);
	if(ip == 0) {
		return 0;
	} else {
		rt = (unsigned short)(*(ip+0));
	}

	DTPRINTF(0x01, "FWIDTH %d\n", (int)rt);

	return rt;
}

/**
   @brief	カレントフォントセットの文字列幅を取得する

   @param	str	文字列幅を取得する文字の文字コード(UTF-16)

   @return	文字列幅
*/
unsigned short str_width(unsigned char *str)
{
	ushort code;
	int slen = 0;
	int wlen = 0;

	while((*str) != 0) {
		XDUMP(0x02, str, 3);
		slen = utf8code_to_utf16code(&code, str);
		DTFPRINTF(0x02, "0x%04X\n", (int)code);
		if(slen != 0) {
			wlen += font_width(code);
			str += slen;
		} else {
			break;
		}
	}

	return (unsigned short)wlen;
}

/**
   @brief	カレントフォントセットの文字高さを取得する

   @return	文字高さ
*/
unsigned short font_height(void)
{
	return now_fontset->font->height;
}
