/** @file
    @brief	グラフィック関連のコマンド

    @date	2008.04.08
    @date	2007.04.22

    @author	Takashi SHUDO

    @section graph_command graphコマンド

    graph コマンドには以下のサブコマンドがあります。

    | サブコマンド	| 機能					| 詳細				|
    |:------------------|:--------------------------------------|:------------------------------|
    | drawmode		| @copybrief com_graph_drawmode		| @ref com_graph_drawmode	|
    | forecolor		| @copybrief com_graph_forecolor	| @ref com_graph_forecolor	|
    | backcolor		| @copybrief com_graph_backcolor	| @ref com_graph_backcolor	|
    | cls		| @copybrief com_graph_cls		| @ref com_graph_cls		|
    | point		| @copybrief com_graph_point		| @ref com_graph_point		|
    | hline		| @copybrief com_graph_hline		| @ref com_graph_hline		|
    | vline		| @copybrief com_graph_vline		| @ref com_graph_vline		|
    | rect		| @copybrief com_graph_rect		| @ref com_graph_rect		|
    | fillrect		| @copybrief com_graph_fillrect		| @ref com_graph_fillrect	|
    | roundrect		| @copybrief com_graph_roundrect	| @ref com_graph_roundrect	|
    | roundfrect	| @copybrief com_graph_roundfillrect	| @ref com_graph_roundfillrect	|
    | cliprect		| @copybrief com_graph_cliprect		| @ref com_graph_cliprect	|
    | line		| @copybrief com_graph_line		| @ref com_graph_line		|
    | setfont		| @copybrief com_graph_setfont		| @ref com_graph_setfont	|
    | proport		| @copybrief com_graph_proportional	| @ref com_graph_proportional	|
    | drawchar		| @copybrief com_graph_drawchar		| @ref com_graph_drawchar	|
    | charcode		| @copybrief com_graph_charcode		| @ref com_graph_charcode	|
    | drawstr		| @copybrief com_graph_drawstr		| @ref com_graph_drawstr	|
    | fontpreview	| @copybrief com_graph_fontpreview	| @ref com_graph_fontpreview	|
    | circle		| @copybrief com_graph_circle		| @ref com_graph_circle		|
    | fillcircle	| @copybrief com_graph_fillcircle	| @ref com_graph_fillcircle	|
    | ellipse		| @copybrief com_graph_ellipse		| @ref com_graph_ellipse	|
    | fillellipse	| @copybrief com_graph_fillellipse	| @ref com_graph_fillellipse	|
    | scrollv		| @copybrief com_graph_scrollv		| @ref com_graph_scrollv	|
    | jpeg		| @copybrief com_graph_jpeg		| @ref com_graph_jpeg		|
    | png		| @copybrief com_graph_png		| @ref com_graph_png		|
    | vertex4		| @copybrief com_graph_vertex4		| @ref com_graph_vertex4	|
    | dispframe		| @copybrief com_graph_dispframe	| @ref com_graph_dispframe	|
    | drawframe		| @copybrief com_graph_drawframe	| @ref com_graph_drawframe	|
    | sector		| @copybrief com_graph_sector		| @ref com_graph_sector		|
    | enlbitmap		| @copybrief com_graph_enlbitmap	| @ref com_graph_enlbitmap	|
*/

#include "sysconfig.h"
#include "gadgetseed.h"
#include "shell.h"
#include "str.h"
#include "timer.h"
#include "font.h"
#include "console.h"
#include "tprintf.h"
#ifdef GSC_COMP_ENABLE_FATFS
#include "file.h"
#endif
#include "memory.h"

#include "device/video_ioctl.h"
#include "graphics.h"

#if 0
static short scr_width;
static short scr_height;

static rect frect;
static rect crect;

static void init_graph_com(void)
{
	get_screen_info(&scr_width, &scr_height);

	frect.left = 3;
	frect.top = 3;
	frect.right = scr_width-3;
	frect.bottom = scr_height-3;
	crect.left = 4;
	crect.top = 4;
	crect.right = scr_width-4;
	crect.bottom = scr_height-4;
}
#endif

static int drawmode(int argc, uchar *argv[]);

/**
   @brief	描画モードを設定する

   設定可能なモードは以下

   0:標準\n
   1:フォアカラーとバックカラーを逆に描画する\n
   2:フォアカラーのみ描画する
*/
const struct st_shell_command com_graph_drawmode = {
	.name		= "drawmode",
	.command	= drawmode,
	.usage_str	= "<mode(0|1|2)>",
	.manual_str	= "Set draw mode(0:NORMAL,1:REVERSE,2:FOREONLY)"
};

static int drawmode(int argc, uchar *argv[])
{
	if(argc < 2) {
		print_command_usage(&com_graph_drawmode);
		return 0;
	}

	set_draw_mode(dstou(argv[1]));

	return 0;
}


static int forecolor(int argc, uchar *argv[]);

/**
   @brief	フォアカラーを設定する
*/
static const struct st_shell_command com_graph_forecolor = {
	.name		= "forecolor",
	.command	= forecolor,
	.usage_str	= "<r(0-255)> <g(0-255)> <b(0-255)>",
	.manual_str	= "Set fore color"
};

static int forecolor(int argc, uchar *argv[])
{
	unsigned int color;

	if(argc < 4) {
		print_command_usage(&com_graph_forecolor);
		return 0;
	}

	color = RGB(dstoi(argv[1]), dstoi(argv[2]), dstoi(argv[3]));

	set_forecolor(color);

	return 0;
}


static int backcolor(int argc, uchar *argv[]);

/**
   @brief	バックカラーを設定する
*/
static const struct st_shell_command com_graph_backcolor = {
	.name		= "backcolor",
	.command	= backcolor,
	.usage_str	= "<r(0-255)> <g(0-255)> <b(0-255)>",
	.manual_str	= "Set back color"
};

static int backcolor(int argc, uchar *argv[])
{
	unsigned int color;

	if(argc < 4) {
		print_command_usage(&com_graph_backcolor);
		return 0;
	}

	color = RGB(dstoi(argv[1]), dstoi(argv[2]), dstoi(argv[3]));

	set_backcolor(color);

	return 0;
}


static int cls(int argc, uchar *argv[])
{
	clear_screen();

	return 0;
}

/**
   @brief	画面を初期化する
*/
static const struct st_shell_command com_graph_cls = {
	.name		= "cls",
	.command	= cls,
	.manual_str	= "Clear screen"
};


static int point(int argc, uchar *argv[]);

/**
   @brief	点を描画する
*/
static const struct st_shell_command com_graph_point = {
	.name		= "point",
	.command	= point,
	.usage_str	= "<x> <y>",
	.manual_str	= "Draw point"
};

static int point(int argc, uchar *argv[])
{
	if(argc < 2) {
		print_command_usage(&com_graph_point);
		return 0;
	}

	draw_point(dstoi(argv[1]), dstoi(argv[2]));

	return 0;
}


static int hline(int argc, uchar *argv[]);

/**
   @brief	水平線を描画する
*/
static const struct st_shell_command com_graph_hline = {
	.name		= "hline",
	.command	= hline,
	.usage_str	= "<x> <y> <w>",
	.manual_str	= "Draw horizontal line"
};

static int hline(int argc, uchar *argv[])
{
	if(argc < 4) {
		print_command_usage(&com_graph_hline);
		return 0;
	}

	draw_h_line(dstoi(argv[1]), dstoi(argv[2]), dstoi(argv[3]));

	return 0;
}


static int vline(int argc, uchar *argv[]);

/**
   @brief	垂直線を描画する
*/
static const struct st_shell_command com_graph_vline = {
	.name		= "vline",
	.command	= vline,
	.usage_str	= "<x> <y> <h>",
	.manual_str	= "Draw virtical line"
};

static int vline(int argc, uchar *argv[])
{
	if(argc < 4) {
		print_command_usage(&com_graph_vline);
		return 0;
	}

	draw_v_line(dstoi(argv[1]), dstoi(argv[2]), dstoi(argv[3]));

	return 0;
}


static int drawrect(int argc, uchar *argv[]);

/**
   @brief	矩形を描画する
*/
static const struct st_shell_command com_graph_rect = {
	.name		= "rect",
	.command	= drawrect,
	.usage_str	= "<x> <y> <xe> <ye>",
	.manual_str	= "Draw rectangle"
};

static int drawrect(int argc, uchar *argv[])
{
	struct st_rect rect;

	if(argc < 5) {
		print_command_usage(&com_graph_rect);
		return 0;
	}

	rect.left = dstoi(argv[1]);
	rect.top = dstoi(argv[2]);
	rect.right = dstoi(argv[3]);
	rect.bottom = dstoi(argv[4]);

	draw_rect(&rect);

	return 0;
}



static int fillrect(int argc, uchar *argv[]);

/**
   @brief	塗りつぶした矩形を描画する
*/
static const struct st_shell_command com_graph_fillrect = {
	.name	 	= "fillrect",
	.command	= fillrect,
	.usage_str	= "<x> <y> <xe> <ye>",
	.manual_str	= "Draw fill rectangle"
};

static int fillrect(int argc, uchar *argv[])
{
	struct st_rect rect;

	if(argc < 5) {
		print_command_usage(&com_graph_fillrect);
		return 0;
	}

	rect.left = dstoi(argv[1]);
	rect.top = dstoi(argv[2]);
	rect.right = dstoi(argv[3]);
	rect.bottom = dstoi(argv[4]);

	draw_fill_rect(&rect);

	return 0;
}


static int roundrect(int argc, uchar *argv[]);

/**
   @brief	角の丸い矩形を描画する
*/
static const struct st_shell_command com_graph_roundrect = {
	.name		= "roundrect",
	.command	= roundrect,
	.usage_str	= "<x> <y> <xe> <ye> <r>",
	.manual_str	= "Draw round rectangle"
};

static int roundrect(int argc, uchar *argv[])
{
	struct st_rect rect;
	short r;

	if(argc < 6) {
		print_command_usage(&com_graph_roundrect);
		return 0;
	}

	rect.left = dstoi(argv[1]);
	rect.top = dstoi(argv[2]);
	rect.right = dstoi(argv[3]);
	rect.bottom = dstoi(argv[4]);
	r = dstoi(argv[5]);

	draw_round_rect(&rect, r);

	return 0;
}


static int roundfillrect(int argc, uchar *argv[]);

/**
   @brief	角の丸い塗りつぶした矩形を描画する
*/
static const struct st_shell_command com_graph_roundfillrect = {
	.name		= "roundfrect",
	.command	= roundfillrect,
	.usage_str	= "<x> <y> <xe> <ye> <r>",
	.manual_str	= "Draw round fill rectangle"
};

static int roundfillrect(int argc, uchar *argv[])
{
	struct st_rect rect;
	short r;

	if(argc < 6) {
		print_command_usage(&com_graph_roundfillrect);
		return 0;
	}

	rect.left = dstoi(argv[1]);
	rect.top = dstoi(argv[2]);
	rect.right = dstoi(argv[3]);
	rect.bottom = dstoi(argv[4]);
	r = dstoi(argv[5]);

	draw_round_fill_rect(&rect, r);

	return 0;
}


static int cliprect(int argc, uchar *argv[]);

/**
   @brief	クリッピング領域を設定する
*/
static const struct st_shell_command com_graph_cliprect = {
	.name		= "cliprect",
	.command	= cliprect,
	.usage_str	= "[<x> <y> <xe> <ye>]",
	.manual_str	= "Set clipping area"
};

static int cliprect(int argc, uchar *argv[])
{
	struct st_rect rect;

	if(argc < 2) {
		clear_clip_rect();
		tprintf("clip clear\n");
		return 0;
	} else if(argc < 5) {
		print_command_usage(&com_graph_cliprect);
		return 0;
	}

	rect.left = dstoi(argv[1]);
	rect.top = dstoi(argv[2]);
	rect.right = dstoi(argv[3]);
	rect.bottom = dstoi(argv[4]);

	set_clip_rect(&rect);

	return 0;
}


static int drawline(int argc, uchar *argv[]);

/**
   @brief	直線を描画する
*/
static const struct st_shell_command com_graph_line = {
	.name		= "line",
	.command	= drawline,
	.usage_str	= "<x> <y> <xe> <ye>",
	.manual_str	= "Set line"
};

static int drawline(int argc, uchar *argv[])
{
	struct st_rect rect;

	if(argc < 5) {
		print_command_usage(&com_graph_line);
		return 0;
	}

	rect.left = dstoi(argv[1]);
	rect.top = dstoi(argv[2]);
	rect.right = dstoi(argv[3]);
	rect.bottom = dstoi(argv[4]);

	draw_line(rect.left, rect.top, rect.right, rect.bottom);

	return 0;
}


#ifdef GSC_COMP_ENABLE_FONTS
#include "font.h"

static int setfont(int argc, uchar *argv[]);

/**
   @brief	描画するフォントを設定する
*/
static const struct st_shell_command com_graph_setfont = {
	.name		= "setfont",
	.command	= setfont,
	.usage_str	= "<font_name>",
	.manual_str	= "Set font"
};

static int setfont(int argc, uchar *argv[])
{
	if(argc < 2) {
		int i;
		int fc = fontset_count();
		struct st_fontset *fs = get_fontset();;

		print_command_usage(&com_graph_setfont);

		tprintf("Font list\n");

		for(i=0; i<fc; i++) {
			tprintf("  %s\n", fontset_name(i));
		}

		tprintf("Now font name : %s\n", fs->name);
		return 0;
	}

	if(set_font_by_name((char *)argv[1]) == 0) {
		tprintf("Font name \"%s\" not found.\n", argv[1]);
	}

	return 0;
}


static int proportional(int argc, uchar *argv[]);

/**
   @brief	フォントの描画モード(固定幅/プロポーショナル)を設定する
*/
static const struct st_shell_command com_graph_proportional = {
	.name		= "proport",
	.command	= proportional,
	.usage_str	= "<0:FIXED | 1:PROPORT>",
	.manual_str	= "Set font draw mode"
};

static int proportional(int argc, uchar *argv[])
{
	if(argc < 2) {
		print_command_usage(&com_graph_proportional);
		return 0;
	}

	set_font_drawmode(dstoi(argv[1]));

	return 0;
}


static int drawchar(int argc, uchar *argv[]);

/**
   @brief	1文字描画する
*/
static const struct st_shell_command com_graph_drawchar = {
	.name		= "drawchar",
	.command	= drawchar,
	.usage_str	= "<x> <y> <char>",
	.manual_str	= "Draw charctor"
};

static int drawchar(int argc, uchar *argv[])
{
	if(argc < 4) {
		print_command_usage(&com_graph_drawchar);
		return 0;
	}

	draw_char(dstoi(argv[1]), dstoi(argv[2]), argv[3][0]);

	return 0;
}


static int drawcode(int argc, uchar *argv[]);

/**
   @brief	指定文字コードの文字を描画する
*/
static const struct st_shell_command com_graph_charcode = {
	.name		= "charcode",
	.command	= drawcode,
	.usage_str	= "<x> <y> <char_code>",
	.manual_str	= "Draw charctor code charctor"
};

static int drawcode(int argc, uchar *argv[])
{
	if(argc < 4) {
		print_command_usage(&com_graph_charcode);
		return 0;
	}

	draw_char(dstoi(argv[1]), dstoi(argv[2]), hstoi(argv[3]));

	return 0;
}


static int drawstr(int argc, uchar *argv[]);

/**
   @brief	文字列を描画する
*/
static const struct st_shell_command com_graph_drawstr = {
	.name		= "drawstr",
	.command	= drawstr,
	.usage_str	= "<x> <y> <strings> [width]",
	.manual_str	= "Draw charctor code charctor"
};

static int drawstr(int argc, uchar *argv[])
{
	if(argc < 4) {
		print_command_usage(&com_graph_drawstr);
		return 0;
	}

	if(argc > 4) {
		draw_fixed_width_str(dstoi(argv[1]), dstoi(argv[2]),
				     (unsigned char *)argv[3], dstoi(argv[4]));
	} else {
		draw_str(dstoi(argv[1]), dstoi(argv[2]), (unsigned char *)argv[3]);
	}

	return 0;
}


static int fontpreview(int argc, uchar *argv[]);

/**
   @brief	指定フォントの半角文字をプレビュー描画する
*/
static const struct st_shell_command com_graph_fontpreview = {
	.name		= "fontpreview",
	.command	= fontpreview,
	.usage_str	= "[<x> <y>]",
	.manual_str	= "Draw font preview"
};

static int fontpreview(int argc, uchar *argv[])
{
	short sx = 0, sy = 0;
	short x = 0, y = 0;
	short width, height;
	unsigned short fw;
	unsigned short fh = font_height();
	unsigned short ch;
	unsigned short sc = ' ';

	if(argc >= 2) {
		sx = dstoi(argv[1]);
		x = sx;
	}

	if(argc >= 3) {
		sy = dstoi(argv[2]);
		y = sy;
	}

	if(argc >= 4) {
		sc = *argv[3];
	}

	get_screen_info(&width, &height);

	for(ch = sc; ch <= '~'; ch ++) {
		fw = font_width(ch);
		if(x > (width - fw)) {
			x = sx;
			y += fh;
		}
		draw_char(x, y, ch);
		x += fw;
	}

	return 0;
}


#endif // GSC_COMP_ENABLE_FONTS

static int circle(int argc, uchar *argv[]);

/**
   @brief	円を描画する
*/
static const struct st_shell_command com_graph_circle = {
	.name		= "circle",
	.command	= circle,
	.usage_str	= "<x> <y> <r>",
	.manual_str	= "Draw circle"
};

static int circle(int argc, uchar *argv[])
{
	if(argc < 4) {
		print_command_usage(&com_graph_circle);
		return 0;
	}

	draw_circle(dstoi(argv[1]), dstoi(argv[2]), dstoi(argv[3]));

	return 0;
}


static int fcircle(int argc, uchar *argv[]);

/**
   @brief	塗りつぶした円を描画する
*/
static const struct st_shell_command com_graph_fillcircle = {
	.name		= "fillcircle",
	.command	= fcircle,
	.usage_str	= "<x> <y> <r>",
	.manual_str	= "Draw fill circle"
};

static int fcircle(int argc, uchar *argv[])
{
	if(argc < 4) {
		print_command_usage(&com_graph_fillcircle);
		return 0;
	}

	draw_fill_circle(dstoi(argv[1]), dstoi(argv[2]), dstoi(argv[3]));

	return 0;
}


static int ellipse(int argc, uchar *argv[]);

/**
   @brief	楕円を描画する
*/
static const struct st_shell_command com_graph_ellipse = {
	.name		= "ellipse",
	.command	= ellipse,
	.usage_str	= "<x> <y> <rx> <ry>",
	.manual_str	= "Draw ellipse"
};

static int ellipse(int argc, uchar *argv[])
{
	if(argc < 5) {
		print_command_usage(&com_graph_ellipse);
		return 0;
	}

	draw_ellipse(dstoi(argv[1]), dstoi(argv[2]),
		     dstoi(argv[3]), dstoi(argv[4]));

	return 0;
}


static int fellipse(int argc, uchar *argv[]);

/**
   @brief	塗りつぶした楕円を描画する
*/
static const struct st_shell_command com_graph_fillellipse = {
	.name		= "fillellipse",
	.command	= fellipse,
	.usage_str	= "<x> <y> <rx> <ry>",
	.manual_str	= "Draw fill ellipse"
};

static int fellipse(int argc, uchar *argv[])
{
	if(argc < 5) {
		print_command_usage(&com_graph_fillellipse);
		return 0;
	}

	draw_fill_ellipse(dstoi(argv[1]), dstoi(argv[2]),
			  dstoi(argv[3]), dstoi(argv[4]));

	return 0;
}


static int scrollv(int argc, uchar *argv[]);

/**
   @brief	縦方向に矩形領域をスクロースする
*/
static const struct st_shell_command com_graph_scrollv = {
	.name		= "scrollv",
	.command	= scrollv,
	.usage_str	= "<x> <y> <xe> <ye> <v>",
	.manual_str	= "Scroll the area virtically"
};

static int scrollv(int argc, uchar *argv[])
{
	struct st_rect rect;
	short v;

	if(argc < 6) {
		print_command_usage(&com_graph_scrollv);
		return 0;
	}

	rect.left = dstoi(argv[1]);
	rect.top = dstoi(argv[2]);
	rect.right = dstoi(argv[3]);
	rect.bottom = dstoi(argv[4]);
	v = dstoi(argv[5]);

	scroll_rect_v(&rect, v);

	return 0;
}


#ifdef GSC_COMP_ENABLE_FATFS

#ifdef GSC_LIB_ENABLE_PICOJPEG

#include "jpegdec.h"

static int jpeg(int argc, uchar *argv[]);

/**
   @brief	JPEGファイルを描画する
*/
static const struct st_shell_command com_graph_jpeg = {
	.name		= "jpeg",
	.command	= jpeg,
	.usage_str	= "<file_name> [<x> <y>]",
	.manual_str	= "Draw JPEG file"
};

static int jpeg(int argc, uchar *argv[])
{
#define BUFSIZE	256
	int fd;
	int fsize;
	short x = 0, y = 0;
	int flg_posfix = 0;
	int denomi = 1;
	static pjpeg_image_info_t jpeginfo;

	if(argc < 2) {
		print_command_usage(&com_graph_jpeg);
		return 0;
	}

	if(argc > 3) {
		x = dstoi(argv[2]);
		y = dstoi(argv[3]);
		flg_posfix = 1;
	}

	fd = open_file((unsigned char *)argv[1], FA_READ);
	if(fd < 0) {
		tprintf("Cannot open file \"%s\".\n", argv[1]);
		return 0;
	}
	fsize = seek_file(fd, 0, SEEK_END);
	(void)seek_file(fd, 0, SEEK_SET);
	tprintf("size %d\n", fsize);
	tprintf("x = %d, y = %d\n", x, y);

	get_jpeg_file_info(fd, &jpeginfo, 0);
	tprintf("Width = %d, Height = %d\n", jpeginfo.m_width, jpeginfo.m_height);

	if((jpeginfo.m_width > GSC_GRAPHICS_DISPLAY_WIDTH) || (jpeginfo.m_height > GSC_GRAPHICS_DISPLAY_HEIGHT)) {
		close_file(fd);
		fd = open_file((unsigned char *)argv[1], FA_READ);
		get_jpeg_file_info(fd, &jpeginfo, 1);
		denomi = 8;
	}

	if(flg_posfix == 0) {
		x = (GSC_GRAPHICS_DISPLAY_WIDTH - (jpeginfo.m_width / denomi)) / 2;
		y = (GSC_GRAPHICS_DISPLAY_HEIGHT - (jpeginfo.m_height / denomi)) / 2;
	}

	draw_jpeg(x, y);

	close_file(fd);

	return 0;
}
#endif

#ifdef GSC_LIB_ENABLE_LIBPNG

#include "pngdec.h"

static int png(int argc, uchar *argv[]);

/**
   @brief	PNGファイルを描画する
*/
static const struct st_shell_command com_graph_png = {
	.name		= "png",
	.command	= png,
	.usage_str	= "<file_name> [<x> <y>]",
	.manual_str	= "Draw PNG file"
};

static int png(int argc, uchar *argv[])
{
#define BUFSIZE	256
	int fd;
	int fsize;
	short x = 0, y = 0;
	int flg_posfix = 0;
	short width, height;
	int rt = 0;
	void *image;

	if(argc < 2) {
		print_command_usage(&com_graph_png);
		return 0;
	}

	if(argc > 3) {
		x = dstoi(argv[2]);
		y = dstoi(argv[3]);
		flg_posfix = 1;
	}

	fd = open_file((unsigned char *)argv[1], FA_READ);
	if(fd < 0) {
		tprintf("Cannot open file \"%s\".\n", argv[1]);
		return 0;
	}
	fsize = seek_file(fd, 0, SEEK_END);
	(void)seek_file(fd, 0, SEEK_SET);
	tprintf("size %d\n", fsize);
	tprintf("x = %d, y = %d\n", x, y);

	rt = get_png_file_info(fd, &width, &height);
	if(rt != 0) {
		tprintf("PNG decode error(%d)\n", rt);
		goto error;
	}
	tprintf("Width = %d, Height = %d\n", width, height);

	if(flg_posfix == 0) {
		x = (GSC_GRAPHICS_DISPLAY_WIDTH - width) / 2;
		y = (GSC_GRAPHICS_DISPLAY_HEIGHT - height) / 2;
	}

	image = alloc_memory(width * height * sizeof(PIXEL_DATA));
	if(image != 0) {
		decode_png(image);
		draw_image(x, y, width, height, image, width);
		free_memory(image);
	}

error:
	close_file(fd);

	return 0;
}
#endif // GSC_LIB_ENABLE_LIBPNG

#endif // GSC_COMP_ENABLE_FATFS


static int vertex4_region(int argc, uchar *argv[]);

/**
   @brief	4頂点ポリゴンを描画する
*/
static const struct st_shell_command com_graph_vertex4 = {
	.name		= "vertex4",
	.command	= vertex4_region,
	.usage_str	= "<x0> <y0> <x1> <y1> <x2> <y2> <x3> <y3>",
	.manual_str	= "Draw 4 vertex porigon"
};

static int vertex4_region(int argc, uchar *argv[])
{
	if(argc < 9) {
		print_command_usage(&com_graph_vertex4);
		return 0;
	}

	draw_vertex4_region(dstoi(argv[1]), dstoi(argv[2]),
			    dstoi(argv[3]), dstoi(argv[4]),
			    dstoi(argv[5]), dstoi(argv[6]),
			    dstoi(argv[7]), dstoi(argv[8]));

	return 0;
}


static int display_frame(int argc, uchar *argv[]);

/**
   @brief	表示フレームを設定する
*/
static const struct st_shell_command com_graph_dispframe = {
	.name		= "dispframe",
	.command	= display_frame,
	.usage_str	= "[frame_num(0|1)]",
	.manual_str	= "Set display frame"
};

static int display_frame(int argc, uchar *argv[])
{
	if(argc < 2) {
		tprintf("Display Frame %d\n", get_display_frame());
	} else {
		set_display_frame(dstoi(argv[1]));
	}

	return 0;
}


static int draw_frame(int argc, uchar *argv[]);

/**
   @brief	描画フレームを設定する
*/
static const struct st_shell_command com_graph_drawframe = {
	.name		= "drawframe",
	.command	= draw_frame,
	.usage_str	= "[frame_num(0|1)]",
	.manual_str	= "Set draw frame"
};

static int draw_frame(int argc, uchar *argv[])
{
	if(argc < 2) {
		tprintf("Draw Frame %d\n", get_draw_frame());
	} else {
		set_draw_frame(dstoi(argv[1]));
	}

	return 0;
}


static int sector(int argc, uchar *argv[]);

/**
   @brief	扇を描画する
*/
static const struct st_shell_command com_graph_sector = {
	.name		= "sector",
	.command	= sector,
	.usage_str	= "<x> <y> <er> <ir> <q>",
	.manual_str	= "Draw sector"
};

static int sector(int argc, uchar *argv[])
{
	if(argc < 6) {
		print_command_usage(&com_graph_sector);
		return 0;
	}

	draw_sector(dstoi(argv[1]), dstoi(argv[2]),
		    dstoi(argv[3]), dstoi(argv[4]),
		    dstoi(argv[5]));

	return 0;
}


static int enlargedbitmap(int argc, uchar *argv[]);

/**
   @brief	拡大したビットマップテストデータを描画する
*/
static const struct st_shell_command com_graph_enlbitmap = {
	.name		= "enlbitmap",
	.command	= enlargedbitmap,
	.usage_str	= "<x> <y> <r>",
	.manual_str	= "Draw enlarged bitmap sample data"
};

static int enlargedbitmap(int argc, uchar *argv[])
{
	extern struct st_bitmap gs_logo;

	if(argc < 4) {
		print_command_usage(&com_graph_enlbitmap);
		return 0;
	}

#if 1
	draw_enlarged_bitmap(dstoi(argv[1]), dstoi(argv[2]),
			     &gs_logo,
			     dstoi(argv[3]));
#else
	draw_bitmap(dstoi(argv[1]), dstoi(argv[2]),
		    &gs_logo);
#endif

	return 0;
}



static const struct st_shell_command * const com_grap_list[] = {
	&com_graph_drawmode,
	&com_graph_forecolor,
	&com_graph_backcolor,
	&com_graph_cls,
	&com_graph_point,
	&com_graph_hline,
	&com_graph_vline,
	&com_graph_rect,
	&com_graph_fillrect,
	&com_graph_roundrect,
	&com_graph_roundfillrect,
	&com_graph_cliprect,
	&com_graph_line,
#ifdef GSC_COMP_ENABLE_FONTS
	&com_graph_setfont,
	&com_graph_proportional,
	&com_graph_drawchar,
	&com_graph_charcode,
	&com_graph_drawstr,
	&com_graph_fontpreview,
#endif
	&com_graph_circle,
	&com_graph_fillcircle,
	&com_graph_ellipse,
	&com_graph_fillellipse,
	&com_graph_scrollv,
#ifdef GSC_COMP_ENABLE_FATFS
#ifdef GSC_LIB_ENABLE_PICOJPEG
	&com_graph_jpeg,
#endif
#ifdef GSC_LIB_ENABLE_LIBPNG
	&com_graph_png,
#endif
#endif
	&com_graph_vertex4,
	&com_graph_dispframe,
	&com_graph_drawframe,
	&com_graph_sector,
	&com_graph_enlbitmap,
	0
};

const struct st_shell_command com_graphics = {
	.name		= "graph",
	.manual_str	= "Graphics operation commands",
	.sublist	= com_grap_list
}; ///< グラフィックス描画
