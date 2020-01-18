/** @file
    @brief	グラフィックス描画

    @date	2015.08.15
    @date	2013.06.20
    @date	2007.03.20
    @author	Takashi SHUDO

    @page graphics グラフィックス

    GadgetSeedは標準化された表示デバイスと表示デバイスへのグラフィックス描画APIを持ちます。

    グラフィックスを使用するには、以下のコンフィグ項目を有効にして下さい。

    * COMP_ENABLE_GRAPHICS


    ---
    @section graphics_struct グラフィックス関連データ構造体

    グラフィックス関連APIは以下の構造体を使用します。

    - @ref st_rect @copybrief st_rect
    - @ref st_box  @copybrief st_box
    - @ref st_bitmap @copybrief st_bitmap


    ---
    @section graphics_api グラフィックスAPI

    @subsection graphics_dev_api グラフィックスデバイス制御API

    以下のAPIはグラフィックスデバイスに対する設定を行います。

    include ファイル : graphics.h

    | API名			| 機能					|
    |:--------------------------|:--------------------------------------|
    | register_graphics_dev()	| @copybrief register_graphics_dev	|
    | get_frame_num()		| @copybrief get_frame_num		|
    | set_display_frame()	| @copybrief set_display_frame		|
    | get_display_frame()	| @copybrief get_display_frame		|
    | set_draw_frame()		| @copybrief set_draw_frame		|
    | get_draw_frame()		| @copybrief get_draw_frame		|
    | init_graphics()		| @copybrief init_graphics		|
    | get_screen_info()		| @copybrief get_screen_info		|

    @subsection graphics_draw_set_api グラフィックス描画設定API

    以下のAPIはグラフィックス描画に関する設定を行います。

    include ファイル : graphics.h

    | API名			| 機能				|
    |:--------------------------|:------------------------------|
    | clear_clip_rect()		| @copybrief clear_clip_rect	|
    | set_clip_rect()		| @copybrief set_clip_rect	|
    | set_clip_box()		| @copybrief set_clip_box	|
    | get_clip_rect()		| @copybrief get_clip_rect	|
    | clear_screen()		| @copybrief clear_screen	|
    | fill_screen()		| @copybrief fill_screen	|
    | set_forecolor()		| @copybrief set_forecolor	|
    | get_forecolor()		| @copybrief get_forecolor	|
    | set_backcolor()		| @copybrief set_backcolor	|
    | get_backcolor()		| @copybrief get_backcolor	|
    | set_draw_mode()		| @copybrief set_draw_mode	|
    | get_draw_mode()		| @copybrief get_draw_mode	|

    @subsection graphics_draw_api グラフィックス描画API

    以下のAPIはグラフィックス描画を行います。

    include ファイル : graphics.h

    | API名			| 機能					|
    |:--------------------------|:--------------------------------------|
    | draw_point()		| @copybrief draw_point			|
    | draw_h_line()		| @copybrief draw_h_line		|
    | draw_v_line()		| @copybrief draw_v_line		|
    | draw_line()		| @copybrief draw_line			|
    | draw_rect()		| @copybrief draw_rect			|
    | draw_round_rect()		| @copybrief draw_round_rect		|
    | draw_fill_rect()		| @copybrief draw_fill_rect		|
    | draw_round_fill_rect()	| @copybrief draw_round_fill_rect	|
    | draw_bitdata()		| @copybrief draw_bitdata		|
    | draw_enlarged_bitdata()	| @copybrief draw_enlarged_bitdata	|
    | draw_bitmap()		| @copybrief draw_bitmap		|
    | draw_enlarged_bitmap()	| @copybrief draw_enlarged_bitmap	|
    | draw_circle()		| @copybrief draw_circle		|
    | draw_quarter_circle()	| @copybrief draw_quarter_circle	|
    | draw_fill_circle()	| @copybrief draw_fill_circle		|
    | draw_ellipse()		| @copybrief draw_ellipse		|
    | draw_fill_ellipse()	| @copybrief draw_fill_ellipse		|
    | draw_box()		| @copybrief draw_box			|
    | draw_round_box()		| @copybrief draw_round_box		|
    | draw_round_fill_box()	| @copybrief draw_round_fill_box	|
    | draw_fill_box()		| @copybrief draw_fill_box		|
    | draw_vertex4_region()	| @copybrief draw_vertex4_region	|
    | draw_sector()		| @copybrief draw_sector		|
    | draw_image()		| @copybrief draw_image			|

    @subsection graphics_calc_api グラフィックス演算API

    以下のAPIはグラフィックスデータの演算を行います。

    include ファイル : graphics.h

    | API名			| 機能				|
    |:--------------------------|:------------------------------|
    | and_rect()		| @copybrief and_rect		|
    | empty_rect()		| @copybrief empty_rect		|
    | correct_rect()		| @copybrief correct_rect	|
    | is_point_in_rect()	| @copybrief is_point_in_rect	|
    | is_point_in_box()		| @copybrief is_point_in_box	|
    | box2rect()		| @copybrief box2rect		|
    | resize_image()		| @copybrief resize_image	|
*/

#include "graphics.h"
#include "device.h"
#include "device/video_ioctl.h"
#include "tkprintf.h"
#include "tprintf.h"

//#define DEBUGTBITS 0x02
#include "dtprintf.h"

#ifdef GSC_GRAPHIC_ENABLE_DEV_MUTEX	/// $gsc グラフィックデバイスのMUTEXを有効にする
#define MAXDEVLOCK_TIME	10000
#define GDEV_LOCK()	lock_device(fb_dev,MAXDEVLOCK_TIME)
#define GDEV_UNLOCK()	unlock_device(fb_dev)
#else
#define GDEV_LOCK()
#define GDEV_UNLOCK()
#endif

static struct st_device *fb_dev;
static short gdev_type;
static short screen_width;
static short screen_height;
static unsigned short color_depth;
static unsigned int screen_mem_size;
static unsigned short frame_num;
static unsigned int fore_color;
static unsigned int back_color;
static unsigned char pen_mode;
static struct st_rect i_clip_rect;
static struct st_rect clip_rect;

/**
   @brief	グラフィックライブラリにデバイスを登録する

   @param[in]	dev	グラフィックデバイス

   @return	!=0:エラー
*/
int register_graphics_dev(struct st_device *dev)
{
	struct st_video_info *v_info = (struct st_video_info *)(dev->info);

	static const char color_dep_str[5][20] = {
		"1 bit monochrome",
		"8 bit 256 colors",
		"16 bit color",
		"24 bit color",
		"32 bit color"
	};

	static const char graph_dev_type_str[5][20] = {
		"Monochrome",
		"No frame buffer",
		"Frame buffer"
	};

	if(v_info == 0) {
		SYSERR_PRINT("Cannot find video info\n");
		return -1;
	}

	fb_dev = dev;

	gdev_type	= v_info->type;
	screen_width	= v_info->width;
	screen_height	= v_info->height;
	color_depth	= v_info->color_depth;
	screen_mem_size	= v_info->mem_size;
	frame_num	= v_info->frame_num;
	tkprintf("Graphics device \"%s\" Type : %s, Screen size %dx%d(%d), %s\n", dev->name,
		 graph_dev_type_str[gdev_type],
		 screen_width, screen_height, frame_num,
		 color_dep_str[color_depth]);

	pen_mode = GRP_DRAWMODE_NORMAL;
	set_forecolor(RGB(255, 255, 255));
	set_backcolor(RGB(0, 0, 0));

	i_clip_rect.top = 0;
	i_clip_rect.left = 0;
	i_clip_rect.right = screen_width;
	i_clip_rect.bottom = screen_height;
	clip_rect = i_clip_rect;

	return 0;
}

/**
   @brief	表示するフレームバッファ番号を設定する

   @param[in]	fnum	表示するフレームバッファ番号

   @return	!=0:エラー
*/
int set_display_frame(int fnum)
{
	GDEV_LOCK();
	return ioctl_device(fb_dev, IOCMD_VIDEO_SETDISPFRAME, fnum, 0);
	GDEV_UNLOCK();
}

/**
   @brief	表示しているフレームバッファ番号を取得する

   @return	表示しているフレームバッファ番号
*/
int get_display_frame(void)
{
	GDEV_LOCK();
	return ioctl_device(fb_dev, IOCMD_VIDEO_GETDISPFRAME, 0, 0);
	GDEV_UNLOCK();
}

/**
   @brief	フレームバッファ数を取得する

   @return	フレームバッファ数
*/
int get_frame_num(void)
{
	return frame_num;
}

/**
   @brief	描画するフレームバッファ番号を設定する

   @param[in]	fnum	描画するフレームバッファ番号

   @return	!=0:エラー
*/
int set_draw_frame(int fnum)
{
	GDEV_LOCK();
	return ioctl_device(fb_dev, IOCMD_VIDEO_SETDRAWFRAME, fnum, 0);
	GDEV_UNLOCK();
}

/**
   @brief	描画するフレームバッファ番号を取得する

   @return	描画するフレームバッファ番号
*/
int get_draw_frame(void)
{
	return ioctl_device(fb_dev, IOCMD_VIDEO_GETDRAWFRAME, 0, 0);
}

/**
   @brief	クリッピングエリアを無効にする
*/
void clear_clip_rect(void)
{
	GDEV_LOCK();
	clip_rect = i_clip_rect;
	GDEV_UNLOCK();
}

/**
   @brief	クリッピングエリアを矩形で指定する

   @param[in]	rect	クリッピングエリア矩形
*/
void set_clip_rect(struct st_rect *rect)
{
	GDEV_LOCK();
	clip_rect = *rect;

	if(clip_rect.left < 0)	clip_rect.left = 0;
	if(clip_rect.top < 0)	clip_rect.top = 0;
	if(clip_rect.right > screen_width) clip_rect.right = screen_width;
	if(clip_rect.bottom > screen_height) clip_rect.bottom = screen_height;
	GDEV_UNLOCK();
}

/**
   @brief	クリッピングエリアを四角形で指定する

   @param[in]	box	クリッピングエリア四角形
*/
void set_clip_box(struct st_box *box)
{
	struct st_rect rect;

	box2rect(&rect, box);

	set_clip_rect(&rect);
}

/**
   @brief	クリッピングエリアを矩形で取得する

   @param[out]	rect	取得したクリッピングエリア矩形
*/
void get_clip_rect(struct st_rect *rect)
{
	*rect = clip_rect;
}

/**
   @brief	グラフィックスライブラリを初期化する

   @param[in]	devname	グラフィックスデバイス名

   @return	!=0:エラー
*/
int init_graphics(char *devname)
{
	struct st_device *dev;

	if(devname == 0) goto err;

	dev = open_device(devname);
	if(dev == 0) {
		SYSERR_PRINT("Cannot open device \"%s\"\n", devname);
		goto err;
	}

	return register_graphics_dev(dev);

 err:
	return -1;
}

/**
   @brief	スクリーンのサイズ情報を取得する

   @param[out]	width	スクリーン幅ピクセル数
   @param[out]	height	スクリーン高さピクセル数
*/
void get_screen_info(short *width, short *height)
{
	*width = screen_width;
	*height = screen_height;
}

/**
   @brief	画面を全て0で描画する
*/
void clear_screen(void)
{
	GDEV_LOCK();
	ioctl_device(fb_dev, IOCMD_VIDEO_CLEAR, 0, 0);
	GDEV_UNLOCK();
}

/**
   @brief	画面を全て背景色で描画する
*/
void fill_screen(void)
{
	GDEV_LOCK();
	ioctl_device(fb_dev, IOCMD_VIDEO_FILL, back_color, 0);
	GDEV_UNLOCK();
}

static void prepare_color(void)
{
	unsigned int fcolor;
	unsigned int bcolor;

	switch(pen_mode) {
	case GRP_DRAWMODE_REVERSE:
		fcolor = back_color;
		bcolor = fore_color;
		break;

	case GRP_DRAWMODE_NORMAL:
	default:
		fcolor = fore_color;
		bcolor = back_color;
		break;
	}

	ioctl_device(fb_dev, IOCMD_VIDEO_SET_FORECOLOR, fcolor, 0);
	ioctl_device(fb_dev, IOCMD_VIDEO_SET_BACKCOLOR, bcolor, 0);
}

/**
   @brief	描画の色を設定する

   @param[in]	color	描画の色
*/
void set_forecolor(unsigned int color)
{
	fore_color = color;

	prepare_color();
}

/**
   @brief	描画の色を取得する

   @return	描画の色
*/
unsigned int get_forecolor(void)
{
	return fore_color;
}

/**
   @brief	描画の背景色を設定する

   @param	color	背景色
*/
void set_backcolor(unsigned int color)
{
	GDEV_LOCK();
	back_color = color;

	prepare_color();
	GDEV_UNLOCK();
}

/**
   @brief	描画の背景色を取得する

   @return	背景色
*/
unsigned int get_backcolor(void)
{
	return back_color;
}

/**
   @brief	描画モードを設定する

   @param[in]	mode	描画モード
*/
void set_draw_mode(unsigned char mode)
{
	GDEV_LOCK();
	pen_mode = mode;

	prepare_color();
	GDEV_UNLOCK();
}

/**
   @brief	描画モードを取得する

   @return	描画モード
*/
unsigned char get_draw_mode(void)
{
	return pen_mode;
}

/*
 *
 */

static void _draw_point(short x, short y)
{
	ioctl_device(fb_dev, IOCMD_VIDEO_DRAW_PIXEL, (((int)y) << 16) | (x & 0xffff), 0);
}

/**
   @brief	点を描画する

   @param[in]	x	点のX座標
   @param[in]	y	点のY座標
*/
void draw_point(short x, short y)
{
	if(clip_rect.top > y) return;
	if(clip_rect.bottom <= y) return;
	if(clip_rect.left > x) return;
	if(clip_rect.right <= x) return;

	GDEV_LOCK();
	_draw_point(x, y);
	GDEV_UNLOCK();
}

static void _draw_h_line(short x, short y, short xe)
{
	struct st_rect rect;

	rect.left = x;
	rect.top = y;
	rect.right = xe - 1;
	rect.bottom = y;

	ioctl_device(fb_dev, IOCMD_VIDEO_SETRECT, 0, (void *)&rect);
	ioctl_device(fb_dev, IOCMD_VIDEO_REPEAT_DATA, xe - x, 0);
}

/**
   @brief	水平線を描画する

   @param[in]	x	水平線のX座標
   @param[in]	y	水平線上のY座標
   @param[in]	width	水平線の長さ
*/
void draw_h_line(short x, short y, short width)
{
	short xe = x + width;

	if(clip_rect.top > y) return;
	if(clip_rect.bottom <= y) return;
	if(clip_rect.right <= x) return;
	if(clip_rect.left > x) x = clip_rect.left;
	if(clip_rect.right < xe) xe = clip_rect.right;
	if(x >= xe) return;

	GDEV_LOCK();
	_draw_h_line(x, y, xe);
	GDEV_UNLOCK();
}

static void _draw_v_line(short x, short y, short ye)
{
	struct st_rect rect;

	rect.left = x;
	rect.top = y;
	rect.right = x;
	rect.bottom = ye - 1;

	ioctl_device(fb_dev, IOCMD_VIDEO_SETRECT, 0, (void *)&rect);
	ioctl_device(fb_dev, IOCMD_VIDEO_REPEAT_DATA, ye - y, 0);
}

/**
   @brief	垂直線を描画する
   
   @param[in]	x	垂直線左のX座標
   @param[in]	y	垂直線のY座標
   @param[in]	height	垂直線の長さ
*/
void draw_v_line(short x, short y, short height)
{
	short ye = y + height;

	if(clip_rect.left > x) return;
	if(clip_rect.right <= x) return;
	if(clip_rect.bottom <= y) return;
	if(clip_rect.top > y) y = clip_rect.top;
	if(clip_rect.bottom < ye) ye = clip_rect.bottom;

	GDEV_LOCK();
	_draw_v_line(x, y, ye);
	GDEV_UNLOCK();
}

#define LEFT 1
#define RIGHT 2
#define TOP 4
#define BOTTOM 8

static short calc_seq_code(short x, short y)
{
	short code;

	code = 0;
	if(x < clip_rect.left) code += LEFT;
	if(x >= clip_rect.right) code += RIGHT;
	if(y < clip_rect.top) code += TOP;
	if(y >= clip_rect.bottom) code += BOTTOM;

	return code;
}

static short calc_clipped_point(short code,
				short x0, short y0,
				short x1, short y1,
				short *x, short *y)
{
	short cx, cy;

	if((code & LEFT) != 0) {
		cy = (y1 - y0) * (clip_rect.left - x0) / (x1 - x0) + y0;
		if((cy >= clip_rect.top) && (cy <= clip_rect.bottom-1)) {
			*x = clip_rect.left;
			*y = cy;
			return 1;
		}
	}

	if((code & RIGHT) != 0) {
		cy = (y1 - y0) * (clip_rect.right-1 - x0) / (x1 - x0) + y0;
		if((cy >= clip_rect.top) && (cy <= clip_rect.bottom-1)) {
			*x = clip_rect.right-1;
			*y = cy;
			return 1;
		}
	}

	if((code & TOP) != 0) {
		cx = (x1 - x0) * (clip_rect.top - y0) / (y1 - y0) + x0;
		if((cx >= clip_rect.left) && (cx <= clip_rect.right-1)) {
			*x = cx;
			*y = clip_rect.top;
			return 1;
		}
	}

	if((code & BOTTOM) != 0) {
		cx = (x1 - x0) * (clip_rect.bottom-1 - y0) / (y1 - y0) + x0;
		if((cx >= clip_rect.left) && (cx <= clip_rect.right-1)) {
			*x = cx;
			*y = clip_rect.bottom-1;
			return 1;
		}
	}

	return -1;
}

static short clipping(short *x0, short *y0, short *x1, short *y1)
{
	short code0,code1;

	code0 = calc_seq_code(*x0, *y0);
	code1 = calc_seq_code(*x1, *y1);

	if((code0 == 0) && (code1 == 0)) {
		return 0;
	}

	if((code0 & code1) != 0) {
		return -1;
	}

	if(code0 != 0) {
		if(calc_clipped_point(code0, *x0, *y0, *x1, *y1, x0, y0) < 0) {
			return -1;
		}
	}

	if(code1 != 0) {
		if(calc_clipped_point(code1, *x0, *y0, *x1, *y1, x1, y1) < 0) {
			return -1;
		}
	}

	return 1;
}

static void line(short x0, short y0, short x1, short y1)
{
	short E,x,y;
	short dx,dy,sx,sy,i;

	sx = (x1 > x0) ? 1 : -1;
	dx = (x1 > x0) ? x1 - x0 : x0 - x1;
	sy = (y1 > y0) ? 1 : -1;
	dy = (y1 > y0) ? y1 - y0 : y0 - y1;

	x = x0;
	y = y0;

	if(dx >= dy) {
		E = -dx;
		for(i=0; i<=dx; i++) {
			_draw_point(x, y);
			x += sx;
			E += 2 * dy;
			if(E >= 0) {
				y += sy;
				E -= 2 * dx;
			}
		}
	} else {
		E = -dy;
		for(i=0; i<=dy; i++) {
			_draw_point(x, y);
			y += sy;
			E += 2 * dx;
			if(E >= 0) {
				x += sx;
				E -= 2 * dy;
			}
		}
	}
}

/**
   @brief	直線を描画する

   @param[in]	x	直線描画開始X座標
   @param[in]	y	直線描画開始Y座標
   @param[in]	xe	直線描画終了X座標
   @param[in]	ye	直線描画終了Y座標
*/
void draw_line(short x, short y, short xe, short ye)
{
	if((x == xe) && (y == ye)) {
		return;
	}

	if(x > xe) {
		x --;
	} else if(xe > x){
		xe --;
	}

	if(y > ye) {
		y --;
	} else if(ye > y){
		ye --;
	}

	if(clipping(&x, &y, &xe, &ye) < 0) {
		return;
	}

	GDEV_LOCK();
	line(x, y, xe, ye);
	GDEV_UNLOCK();
}

/**
   @brief	矩形を描画する

   @param[in]	rect	矩形
*/
void draw_rect(struct st_rect *rect)
{
	struct st_rect drect;
	short tw = 0;
	short lw = 0;
	short bw = 0;

	and_rect(&drect, rect, &clip_rect);

	if(empty_rect(&drect) == 0) {
		GDEV_LOCK();
		// 上
		if((clip_rect.top <= rect->top) &&
		   (rect->top <= clip_rect.bottom)) {
			tw = 1;
			_draw_h_line(drect.left, drect.top, drect.right);
		}

		// 左
		if((clip_rect.left <= rect->left) &&
		   (rect->left <= clip_rect.right)) {
			if((drect.top+tw) < drect.bottom) {
				lw = 1;
				_draw_v_line(drect.left, drect.top+tw,
					     drect.bottom);
			}
		}

		// 下
		if((clip_rect.top <= rect->bottom) &&
		   (rect->bottom <= clip_rect.bottom)) {
			if((drect.left+lw) < drect.right) {
				bw = 1;
				_draw_h_line(drect.left+lw, drect.bottom-1,
					     drect.right);
			}
		}

		// 右
		if((clip_rect.left <= rect->right) &&
		   (rect->right <= clip_rect.right)) {
			if((drect.top+tw) < (drect.bottom-bw)) {
				_draw_v_line(drect.right-1, drect.top+tw,
					     drect.bottom-bw);
			}
		}
		GDEV_UNLOCK();
	}
}

/**
   @brief	角の丸い矩形を描画する

   @param[in]	rect	矩形
   @param[in]	r	角丸の半径
*/
void draw_round_rect(struct st_rect *rect, short r)
{
	short l_w = rect->right - rect->left - (r * 2);
	short l_h = rect->bottom - rect->top - (r * 2);

	// 図形が成り立つ？
	if(l_w < 0) {
		return;
	}

	if(l_h < 0) {
		return;
	}

	GDEV_LOCK();
	// 左上
	draw_quarter_circle(rect->left + r, rect->top + r, r, 1);

	// 上
	if((clip_rect.top <= rect->top) &&
	   (rect->top <= clip_rect.bottom)) {
		draw_h_line(rect->left + r, rect->top, l_w);
	}

	// 右上
	draw_quarter_circle(rect->right - 1 - r, rect->top + r, r, 0);

	// 左
	if((clip_rect.left <= rect->left) &&
	   (rect->left <= clip_rect.right)) {
		draw_v_line(rect->left, rect->top + 1 + r, l_h);
	}

	// 左下
	draw_quarter_circle(rect->left + r, rect->bottom - 1 - r, r, 2);

	// 下
	if((clip_rect.top <= rect->bottom) &&
	   (rect->bottom <= clip_rect.bottom)) {
		draw_h_line(rect->left + r, rect->bottom - 1, l_w);
	}

	// 右下
	draw_quarter_circle(rect->right - 1 - r, rect->bottom - 1 - r, r, 3);

	// 右
	if((clip_rect.left <= rect->right) &&
	   (rect->right <= clip_rect.right)) {
		draw_v_line(rect->right - 1, rect->top + r, l_h);
	}
	GDEV_UNLOCK();
}

static void _draw_fill_rect(struct st_rect *rect)
{
	struct st_rect tmp = *rect;

	if(tmp.left < tmp.right) {
		tmp.right --;
	}
	if(tmp.top < tmp.bottom) {
		tmp.bottom --;
	}

	ioctl_device(fb_dev, IOCMD_VIDEO_SETRECT, 0, (void *)&tmp);
	ioctl_device(fb_dev, IOCMD_VIDEO_REPEAT_DATA,
		     (unsigned int)((rect->right - rect->left/* + 1*/) *
				    (rect->bottom - rect->top/* + 1*/)), 0);
}

/**
   @brief	塗りつぶした矩形を描画する

   @param[in]	rect	矩形
*/
void draw_fill_rect(struct st_rect *rect)
{
	struct st_rect drect;

	and_rect(&drect, rect, &clip_rect);

	if(empty_rect(&drect) == 0) {
		GDEV_LOCK();
		_draw_fill_rect(&drect);
		GDEV_UNLOCK();
	}
}

/**
   @brief	角の丸い塗りつぶした矩形を描画する

   @param[in]	rect	矩形
   @param[in]	r	角丸の半径
*/
void draw_round_fill_rect(struct st_rect *rect, short r)
{
	struct st_rect rrect = *rect;
	struct st_rect drect;
	short l_w = rect->right - rect->left - (r * 2);
	short l_h = rect->bottom - rect->top - (r * 2);
	short x0, y0;
	short x, y, F;

	// 図形が成り立つ？
	if(l_w < 0) {
		return;
	}

	if(l_h < 0) {
		return;
	}

	// 角丸上
	x0 = rect->left + r;
	y0 = rect->top + r;
	x = r;
	y = 0;
	F = -2 * r + 3;

	while(x >= y) {
		draw_h_line(x0 - x, y0 - y, l_w + x * 2);
		draw_h_line(x0 - y, y0 - x, l_w + y * 2);
		if(F >= 0) {
			x--;
			F -= 4 * x;
		}
		y++;
		F += 4 * y + 2;
	}

	// 中の長方形
	rrect.top += r;
	rrect.bottom -= r;

	and_rect(&drect, &rrect, &clip_rect);

	GDEV_LOCK();
	if(empty_rect(&drect) == 0) {
		_draw_fill_rect(&drect);
	}

	// 角丸下
	y0 = rect->bottom - r - 1;
	x = r;
	y = 0;
	F = -2 * r + 3;

	while(x >= y) {
		draw_h_line(x0 - x, y0 + y, l_w + x * 2);
		draw_h_line(x0 - y, y0 + x, l_w + y * 2);
		if(F >= 0) {
			x--;
			F -= 4 * x;
		}
		y++;
		F += 4 * y + 2;
	}
	GDEV_UNLOCK();
}

static void draw_bits(short x, short y, short width, short height,
		      short offset, unsigned char *data, short dw)
{
	struct st_rect rect;
	int j;
	unsigned char *dp;
#ifdef BITS_WRITE_WORD
	unsigned char pat;
	int i;
#endif

	DTFPRINTF(0x01, "\n");
	DTPRINTF(0x01, "x=%d y=%d\n", x, y);
	DTPRINTF(0x01, "w=%d h=%d o=%d dw=%d\n", width, height, offset, dw);
	DTPRINTF(0x01, "data=%p\n", data);

	rect.left = x;
	rect.top = y;
	rect.right = x + width - 1;
	rect.bottom = y + height - 1;

	ioctl_device(fb_dev, IOCMD_VIDEO_SETRECT, 0, (void *)&rect);

	for(j=0; j<height; j++) {
		dp = data + (dw * j) + (offset/8);
		DTPRINTF(0x02, "dp=%p\n", dp);
#ifdef BITS_WRITE_WORD
		pat = (0x80 >> (offset & 7));
		for(i=0; i<width; i++) {
			if(pen_mode == GRP_DRAWMODE_REVERSE) {
				if(*dp & pat) {
					ioctl_device(fb_dev, IOCMD_VIDEO_WRITE_WORD, back_color, 0);
				} else {
					ioctl_device(fb_dev, IOCMD_VIDEO_WRITE_WORD, fore_color, 0);
				}
			} else {
				if(*dp & pat) {
					ioctl_device(fb_dev, IOCMD_VIDEO_WRITE_WORD, fore_color, 0);
				} else {
					ioctl_device(fb_dev, IOCMD_VIDEO_WRITE_WORD, back_color, 0);
				}
			}
			if(pat == 0x01) {
				dp ++;
				pat = 0x80;
			} else {
				pat >>= 1;
			}
		}
#else
		if(pen_mode == GRP_DRAWMODE_FOREONLY) {
			unsigned char pat;
			int i;
			pat = (0x80 >> (offset & 7));
			for(i=0; i<width; i++) {
				if((*dp & pat) != 0) {
					ioctl_device(fb_dev, IOCMD_VIDEO_DRAW_PIXEL,
						     (((int)(y + j)) << 16) | ((x + i) & 0xffff), 0);
				}
				if(pat == 0x01) {
					dp ++;
					pat = 0x80;
				} else {
					pat >>= 1;
				}
			}
		} else {
			ioctl_device(fb_dev, IOCMD_VIDEO_DRAW_BITS | ((offset & 7) << 12) | (width & 0x0fff), 0, (void *)dp);
		}
#endif
	}
}

static void draw_enlarged_bits(short x, short y,
			       short width, short height,
			       short offset, unsigned char *data, short dw,
			       int rate, short dox, short doy)
{
	struct st_rect rect;
	int i, j, n, m = doy;
	unsigned char pat, *dp;

	DTFPRINTF(0x01, "\n");
	DTPRINTF(0x01, "x=%d y=%d\n", x, y);
	DTPRINTF(0x01, "w=%d h=%d o=%d dw=%d\n", width, height, offset, dw);
	DTPRINTF(0x01, "data=%p\n", data);
	DTPRINTF(0x01, "dox=%d doy=%d\n", dox, doy);

	rect.left = x;
	rect.top = y;
	rect.right = x + width - 1;
	rect.bottom = y + height - 1;

	ioctl_device(fb_dev, IOCMD_VIDEO_SETRECT, 0, (void *)&rect);

	for(j=0; j<height; j++) {
		pat = (0x80 >> (offset & 7));
		dp = data + (dw * ((j+m)/rate));
		n = dox;
		ioctl_device(fb_dev, IOCMD_VIDEO_LOCK_DEVICE, 0, 0);
		for(i=0; i<width; i++) {
			if(pen_mode == GRP_DRAWMODE_REVERSE) {
				if((*dp & pat) != 0) {
					ioctl_device(fb_dev, IOCMD_VIDEO_NOLOCK_WRITE_WORD, back_color, 0);
				} else {
					ioctl_device(fb_dev, IOCMD_VIDEO_NOLOCK_WRITE_WORD, fore_color, 0);
				}
			} else {
				if((*dp & pat) != 0) {
					ioctl_device(fb_dev, IOCMD_VIDEO_NOLOCK_WRITE_WORD, fore_color, 0);
				} else {
					ioctl_device(fb_dev, IOCMD_VIDEO_NOLOCK_WRITE_WORD, back_color, 0);
				}
			}
			n++;
			if(n >= rate) {
				if(pat == 0x01) {
					dp ++;
					pat = 0x80;
				} else {
					pat >>= 1;
				}
				n = 0;
			}
		}
		ioctl_device(fb_dev, IOCMD_VIDEO_UNLOCK_DEVICE, 0, 0);
	}
}

/**
   @brief	ビットデータを描画する

   @param[in]	px	描画開始X座標
   @param[in]	py	描画開始Y座標
   @param[in]	width	描画幅
   @param[in]	height	描画高さ
   @param[in]	data	ビットデータポインタ
   @param[in]	dw	ビットデータ幅
*/
void draw_bitdata(short px, short py, short width, short height,
		  unsigned char *data, short dw)
{
	struct st_rect wrect;
	struct st_rect drect;

	wrect.left	= px;
	wrect.top	= py;
	wrect.right	= px + width;
	wrect.bottom	= py + height;

	and_rect(&drect, &wrect, &clip_rect);

	if(empty_rect(&drect) == 0) {
		short offset = 0;
		if((clip_rect.top <= wrect.top) &&
		   (wrect.top < clip_rect.bottom)) {
		} else {
			data += (clip_rect.top - wrect.top) * dw;
		}

		if((clip_rect.left <= wrect.left) &&
		   (wrect.left < clip_rect.right)) {
		} else {
			data += (clip_rect.left-wrect.left)/8;
			offset = (clip_rect.left - wrect.left) & 7;
		}
		DTPRINTF(0x02, "data = %p\n", data);
		GDEV_LOCK();
		draw_bits(drect.left, drect.top,
			  drect.right-drect.left, drect.bottom-drect.top,
			  offset, data, dw);
		GDEV_UNLOCK();
	}
}

/**
   @brief	拡大したビットデータを描画する

   @param[in]	px	描画開始X座標
   @param[in]	py	描画開始Y座標
   @param[in]	width	描画幅
   @param[in]	height	描画高さ
   @param[in]	data	ビットデータポインタ
   @param[in]	dw	ビットデータ幅
   @param[in]	rate	拡大率
*/
void draw_enlarged_bitdata(short px, short py,
			   short width, short height,
			   unsigned char *data, short dw,
			   int rate)
{
	struct st_rect wrect;
	struct st_rect drect;
	short dox = 0, doy = 0;

	if(rate == 0) {
		return;
	}

	wrect.left	= px;
	wrect.top	= py;
	wrect.right	= px + width * rate;
	wrect.bottom	= py + height * rate;

	and_rect(&drect, &wrect, &clip_rect);

	DTPRINTF(0x01, "CLIP  %d %d %d %d\n", clip_rect.left, clip_rect.top, clip_rect.right, clip_rect.bottom);
	DTPRINTF(0x01, "WRECT %d %d %d %d\n", wrect.left, wrect.top, wrect.right, wrect.bottom);
	DTPRINTF(0x01, "DRECT %d %d %d %d\n", drect.left, drect.top, drect.right, drect.bottom);

	if(empty_rect(&drect) == 0) {
		short offset = 0;

		if((clip_rect.top <= wrect.top) && (wrect.top < clip_rect.bottom)) {
			// データの開始位置は変わらない
			DTPRINTF(0x02, "V no change\n");
		} else {
			data += ((clip_rect.top - wrect.top)/rate) * dw;
			doy = (clip_rect.top - py) % rate;
		}

		if((clip_rect.left <= wrect.left) && (wrect.left < clip_rect.right)) {
			// データの開始位置は変わらない
			DTPRINTF(0x02, "H no change\n");
		} else {
			data += (clip_rect.left-wrect.left)/rate/8;
			offset = (clip_rect.left - wrect.left)/rate & 7;
			dox = (clip_rect.left - px) % rate;
		}
		DTPRINTF(0x02, "data = %p\n", data);

		GDEV_LOCK();
		draw_enlarged_bits(drect.left, drect.top,
				   drect.right-drect.left, drect.bottom-drect.top,
				   offset, data, dw,
				   rate, dox, doy);
		GDEV_UNLOCK();
	}
}


/**
   @brief	ビットマップデータを描画する

   @param[in]	px	描画X座標
   @param[in]	py	描画Y座標
   @param[in]	bitmap	ビットマップデータ
*/
void draw_bitmap(short px, short py, struct st_bitmap *bitmap)
{
	draw_bitdata(px, py, bitmap->width, bitmap->height, bitmap->data,
		     (bitmap->width + 7)/8);
}

/**
   @brief	拡大したビットマップデータを描画する

   @param[in]	px	描画X座標
   @param[in]	py	描画Y座標
   @param[in]	bitmap	ビットマップデータ
   @param[in]	rate	拡大率
*/
void draw_enlarged_bitmap(short px, short py, struct st_bitmap *bitmap, int rate)
{
	draw_enlarged_bitdata(px, py, bitmap->width, bitmap->height, bitmap->data,
			      (bitmap->width + 7)/8, rate);
}

/**
   @brief	円を描画する

   @param[in]	x0	円の中心X座標
   @param[in]	y0	円の中心Y座標
   @param[in]	r	円の半径
*/
void draw_circle(short x0, short y0, short r)
{
	short x, y, F;

	x = r;
	y = 0;
	F = -2 * r + 3;

	GDEV_LOCK();
	while(x >= y) {
		draw_point(x0 + x, y0 + y);
		draw_point(x0 - x, y0 + y);
		draw_point(x0 + x, y0 - y);
		draw_point(x0 - x, y0 - y);
		draw_point(x0 + y, y0 + x);
		draw_point(x0 - y, y0 + x);
		draw_point(x0 + y, y0 - x);
		draw_point(x0 - y, y0 - x);
		if(F >= 0) {
			x--;
			F -= 4 * x;
		}
		y++;
		F += 4 * y + 2;
	}
	GDEV_UNLOCK();
}

/**
   @brief	1/4の円を描画する

   @param[in]	x0	円の中心X座標
   @param[in]	y0	円の中心Y座標
   @param[in]	r	円の半径
   @param[in]	q	象限(0:第1象限 〜 3:4象限)
*/
void draw_quarter_circle(short x0, short y0, short r, char q)
{
	short x, y, F;

	x = r;
	y = 0;
	F = -2 * r + 3;

	GDEV_LOCK();
	while(x >= y) {
		switch(q) {
		case 0:
			draw_point(x0 + x, y0 - y);
			draw_point(x0 + y, y0 - x);
			break;

		case 1:
			draw_point(x0 - x, y0 - y);
			draw_point(x0 - y, y0 - x);
			break;

		case 2:
			draw_point(x0 - x, y0 + y);
			draw_point(x0 - y, y0 + x);
			break;

		case 3:
			draw_point(x0 + x, y0 + y);
			draw_point(x0 + y, y0 + x);
			break;
		}
		if(F >= 0) {
			x--;
			F -= 4 * x;
		}
		y++;
		F += 4 * y + 2;
	}
	GDEV_UNLOCK();
}

/**
   @brief	塗りつぶした円を描画する

   @param[in]	x0	円の中心X座標
   @param[in]	y0	円の中心Y座標
   @param[in]	r	円の半径
*/
void draw_fill_circle(short x0, short y0, short r)
{
	short x, y, F;

	x = r;
	y = 0;
	F = -2 * r + 3;

	GDEV_LOCK();
	while(x >= y) {
		draw_h_line(x0 - x, y0 + y, x * 2 + 1);
		draw_h_line(x0 - x, y0 - y, x * 2 + 1);
		draw_h_line(x0 - y, y0 + x, y * 2 + 1);
		draw_h_line(x0 - y, y0 - x, y * 2 + 1);
		if(F >= 0) {
			x--;
			F -= 4 * x;
		}
		y++;
		F += 4 * y + 2;
	}
	GDEV_UNLOCK();
}

/**
   @brief	楕円を描画する

   @param[in]	xc	楕円の中心X座標
   @param[in]	yc	楕円の中心Y座標
   @param[in]	rx	縦の半径
   @param[in]	ry	横の半径
*/
void draw_ellipse(short xc, short yc, short rx, short ry)
{
	int x, x1, y, y1, r;

	if(rx == 0) return;
	if(ry == 0) return;

	GDEV_LOCK();
	if(rx > ry) {
		x = r = rx; y = 0;
		while(x >=y ) {
			x1 = x*ry/rx;
			y1 = y*ry/rx;
			draw_point(xc+x, yc+y1);
			draw_point(xc+x, yc-y1);
			draw_point(xc-x, yc+y1);
			draw_point(xc-x, yc-y1);
			draw_point(xc+y, yc+x1);
			draw_point(xc+y, yc-x1);
			draw_point(xc-y, yc+x1);
			draw_point(xc-y, yc-x1);
			if((r -= y++*2+1)<=0) {
				r+= --x*2;
			}
		}
	} else {
		x = r = ry; y = 0;
		while(x>=y) {
			x1 = x*rx/ry;
			y1 = y*rx/ry;
			draw_point(xc+x1, yc+y);
			draw_point(xc+x1, yc-y);
			draw_point(xc-x1, yc+y);
			draw_point(xc-x1, yc-y);
			draw_point(xc+y1, yc+x);
			draw_point(xc+y1, yc-x);
			draw_point(xc-y1, yc+x);
			draw_point(xc-y1, yc-x);
			if((r -= y++*2+1)<=0) {
				r += --x*2;
			}
		}
	}
	GDEV_UNLOCK();
}

/**
   @brief	塗りつぶした楕円を描画する

   @param[in]	xc	楕円の中心X座標
   @param[in]	yc	楕円の中心Y座標
   @param[in]	rx	縦の半径
   @param[in]	ry	横の半径
*/
void draw_fill_ellipse(short xc, short yc, short rx, short ry)
{
	int x, x1, y, y1, r;

	if(rx == 0) return;
	if(ry == 0) return;

	GDEV_LOCK();
	if(rx > ry) {
		x = r = rx;
		y = 0;
		while(x >=y ) {
			x1 = x*ry/rx;
			y1 = y*ry/rx;
			draw_h_line(xc-x, yc+y1, x*2 + 1);
			draw_h_line(xc-x, yc-y1, x*2 + 1);
			draw_h_line(xc-y, yc+x1, y*2 + 1);
			draw_h_line(xc-y, yc-x1, y*2 + 1);
			if((r -= y++*2+1)<=0) {
				r+= --x*2;
			}
		}
	} else {
		x = r = ry;
		y = 0;
		while(x>=y) {
			x1 = x*rx/ry;
			y1 = y*rx/ry;
			draw_h_line(xc-x1, yc+y, x1*2 + 1);
			draw_h_line(xc-x1, yc-y, x1*2 + 1);
			draw_h_line(xc-y1, yc+x, y1*2 + 1);
			draw_h_line(xc-y1, yc-x, y1*2 + 1);
			if((r -= y++*2+1)<=0) {
				r += --x*2;
			}
		}
	}
	GDEV_UNLOCK();
}

#if 0
/**
   @brief	スクリーンを上にx, 横にyドット分スクロールする

   @param[in]	x	横スクロールドット数
   @param[in]	y	縦スクロールドット数
*/
void scroll_screen(short x, short y)
{
	ioctl_device(fb_dev, IOCMD_VIDEO_SCROLL, (((long)y) << 16) | (x & 0xffff), 0);
}
#endif

/**
   @brief	四角を描画する

   @param[in]	box	四角
*/
void draw_box(struct st_box *box)
{
	struct st_rect rect;

	box2rect(&rect, box);

	draw_rect(&rect);
}

/**
   @brief	角の丸い四角を描画する

   @param[in]	box	四角
   @param[in]	r	角丸の半径
*/
void draw_round_box(struct st_box *box, short r)
{
	struct st_rect rect;

	box2rect(&rect, box);

	draw_round_rect(&rect, r);
}

/**
   @brief	塗りつぶした角の丸い四角を描画する

   @param[in]	box	四角
   @param[in]	r	角丸の半径
*/
void draw_round_fill_box(struct st_box *box, short r)
{
	struct st_rect rect;

	box2rect(&rect, box);

	draw_round_fill_rect(&rect, r);
}

/**
   @brief	塗りつぶした四角を描画する

   @param[in]	box	四角
*/
void draw_fill_box(struct st_box *box)
{
	struct st_rect rect;

	box2rect(&rect, box);

	draw_fill_rect(&rect);
}

#include <limits.h>

#define TOP_MAX SHRT_MAX
#define LEFT_MAX SHRT_MAX
#define RIGHT_MIN SHRT_MIN
#define BOTTOM_MIN SHRT_MIN

static short region_pos[2][GSC_GRAPHICS_DISPLAY_HEIGHT];	// [0][y] : left ,[1][y] : right
// グラフィックデバイスの高さ分必要

static void set_region_pos(short x0, short y0, short x1, short y1, int lr)
{
	short E,x,y;
	short dx,dy,sx,sy,i;

	sx = (x1 > x0) ? 1 : -1;
	dx = (x1 > x0) ? x1 - x0 : x0 - x1;
	sy = (y1 > y0) ? 1 : -1;
	dy = (y1 > y0) ? y1 - y0 : y0 - y1;

	x = x0;
	y = y0;

	if(dx >= dy) {
		E = -dx;
		for(i=0; i<=dx; i++) {
			if((clip_rect.top <= y) && (y < clip_rect.bottom)) {
				region_pos[lr][y] = x;
			}
			DTPRINTF(0x02, "%d [L] %d, %d\n", i, x, y);
			x += sx;
			E += 2 * dy;
			if(E >= 0) {
				y += sy;
				E -= 2 * dx;
			}
		}
	} else {
		E = -dy;
		for(i=0; i<=dy; i++) {
			if((clip_rect.top <= y) && (y < clip_rect.bottom)) {
				region_pos[lr][y] = x;
			}
			DTPRINTF(0x02, "%d [R] %d, %d\n", i, x, y);
			y += sy;
			E += 2 * dx;
			if(E >= 0) {
				x += sx;
				E -= 2 * dy;
			}
		}
	}
}

/**
   @brief	塗りつぶした4頂点の領域を描画する

   @param[in]	x0	頂点0のX座標
   @param[in]	y0	頂点0のY座標
   @param[in]	x1	頂点1のX座標
   @param[in]	y1	頂点1のY座標
   @param[in]	x2	頂点2のX座標
   @param[in]	y2	頂点2のY座標
   @param[in]	x3	頂点3のX座標
   @param[in]	y3	頂点3のY座標
*/
void draw_vertex4_region(short x0, short y0,
			 short x1, short y1,
			 short x2, short y2,
			 short x3, short y3)
{
	short xt = 0, yt = TOP_MAX;	// 最も上の(Y値の大きい)座標
	short xl = LEFT_MAX, yl = 0;	// 最も左の(X値の小さい)座標
	short xr = RIGHT_MIN, yr = 0;	// 最も右の(X値の大きい)座標
	short xb = 0, yb = BOTTOM_MIN;	// 最も下の(Y値の小さい)座標
	int top_pn = -1;
	int left_pn = -1;
//	int right_pn = -1;
	int bottom_pn = -1;
	short i;

	if(yt > y0) { yt = y0; xt = x0; top_pn = 0; }
	if(yt > y1) { yt = y1; xt = x1; top_pn = 1; }
	if(yt > y2) { yt = y2; xt = x2; top_pn = 2; }
	if(yt > y3) { yt = y3; xt = x3; top_pn = 3; }

	if((top_pn != 0) && (yb < y0)) { yb = y0; xb = x0; bottom_pn = 0; }
	if((top_pn != 1) && (yb < y1)) { yb = y1; xb = x1; bottom_pn = 1; }
	if((top_pn != 2) && (yb < y2)) { yb = y2; xb = x2; bottom_pn = 2; }
	if((top_pn != 3) && (yb < y3)) { yb = y3; xb = x3; bottom_pn = 3; }

	if((top_pn != 0) && (bottom_pn != 0) && (xl > x0)) { xl = x0; yl = y0; left_pn = 0; }
	if((top_pn != 1) && (bottom_pn != 1) && (xl > x1)) { xl = x1; yl = y1; left_pn = 1; }
	if((top_pn != 2) && (bottom_pn != 2) && (xl > x2)) { xl = x2; yl = y2; left_pn = 2; }
	if((top_pn != 2) && (bottom_pn != 3) && (xl > x3)) { xl = x3; yl = y3; left_pn = 3; }

	if((top_pn != 0) && (bottom_pn != 0) && (left_pn != 0) && (xr < x0)) { xr = x0; yr = y0; /*right_pn = 0;*/ }
	if((top_pn != 1) && (bottom_pn != 1) && (left_pn != 1) && (xr < x1)) { xr = x1; yr = y1; /*right_pn = 1;*/ }
	if((top_pn != 2) && (bottom_pn != 2) && (left_pn != 2) && (xr < x2)) { xr = x2; yr = y2; /*right_pn = 2;*/ }
	if((top_pn != 3) && (bottom_pn != 3) && (left_pn != 3) && (xr < x3)) { xr = x3; yr = y3; /*right_pn = 3;*/ }

	if(yt == yl) {
		DTPRINTF(0x02, "XT = %d, YT = %d\n", xt, yt);
		DTPRINTF(0x02, "XL = %d, YL = %d\n", xl, yl);
		DTPRINTF(0x02, "XR = %d, YR = %d\n", xr, yr);
		DTPRINTF(0x02, "XB = %d, YB = %d\n", xb, yb);
		if(xt < xl) {
			short xtmp = xt;
			short ytmp = yt;
			xt = xl;
			yt = yl;
			xl = xtmp;
			yl = ytmp;
		}
	}

	if(yt == yr) {
		DTPRINTF(0x02, "XT = %d, YT = %d\n", xt, yt);
		DTPRINTF(0x02, "XL = %d, YL = %d\n", xl, yl);
		DTPRINTF(0x02, "XR = %d, YR = %d\n", xr, yr);
		DTPRINTF(0x02, "XB = %d, YB = %d\n", xb, yb);
		if(xt > xr) {
			short xtmp = xt;
			short ytmp = yt;
			xt = xr;
			yt = yr;
			xr = xtmp;
			yr = ytmp;
		}
	}

	if(yb == yl) {
		DTPRINTF(0x02, "XT = %d, YT = %d\n", xt, yt);
		DTPRINTF(0x02, "XL = %d, YL = %d\n", xl, yl);
		DTPRINTF(0x02, "XR = %d, YR = %d\n", xr, yr);
		DTPRINTF(0x02, "XB = %d, YB = %d\n", xb, yb);
		if(xb < xl) {
			short xtmp = xb;
			short ytmp = yb;
			xb = xl;
			yb = yl;
			xl = xtmp;
			yl = ytmp;
		}
	}

	if(yb == yr) {
		DTPRINTF(0x02, "XT = %d, YT = %d\n", xt, yt);
		DTPRINTF(0x02, "XL = %d, YL = %d\n", xl, yl);
		DTPRINTF(0x02, "XR = %d, YR = %d\n", xr, yr);
		DTPRINTF(0x02, "XB = %d, YB = %d\n", xb, yb);
		if(xb > xr) {
			short xtmp = xb;
			short ytmp = yb;
			xb = xr;
			yb = yr;
			xr = xtmp;
			yr = ytmp;
		}
	}

	if(xl == xr) {
		DTPRINTF(0x02, "XT = %d, YT = %d\n", xt, yt);
		DTPRINTF(0x02, "XL = %d, YL = %d\n", xl, yl);
		DTPRINTF(0x02, "XR = %d, YR = %d\n", xr, yr);
		DTPRINTF(0x02, "XB = %d, YB = %d\n", xb, yb);
		if(yl > yr) {
			short xtmp = xl;
			short ytmp = yl;
			xl = xr;
			yl = yr;
			xr = xtmp;
			yr = ytmp;
		}
	}
	// もう少し良いアルゴリズムがありそう

	DTPRINTF(0x02, "T : %d, B : %d\n", top_pn, bottom_pn);
	DTPRINTF(0x02, "L : %d\n", left_pn);

	DTPRINTF(0x02, "XT = %d, YT = %d\n", xt, yt);
	DTPRINTF(0x02, "XL = %d, YL = %d\n", xl, yl);
	DTPRINTF(0x02, "XR = %d, YR = %d\n", xr, yr);
	DTPRINTF(0x02, "XB = %d, YB = %d\n", xb, yb);

#ifdef DEBUG
	draw_line(xt, yt, xl, yl);
	draw_line(xt, yt, xr, yr);
	draw_line(xl, yl, xb, yb);
	draw_line(xr, yr, xb, yb);
#endif

	GDEV_LOCK();
	if(xl == xr) {
//	if(xt < xl) {
		set_region_pos(xt, yt, xl, yl, 1);
		set_region_pos(xt, yt, xb, yb, 0);
		set_region_pos(xl, yl, xr, yr, 1);
		set_region_pos(xr, yr, xb, yb, 1);
	} else {
		set_region_pos(xt, yt, xl, yl, 0);
		set_region_pos(xt, yt, xr, yr, 1);
		set_region_pos(xl, yl, xb, yb, 0);
		set_region_pos(xr, yr, xb, yb, 1);
	}

	for(i=yt; i<yb; i++) {
		if(region_pos[0][i] > region_pos[1][i]) {
			draw_h_line(region_pos[1][i], i, region_pos[0][i] - region_pos[1][i]);
		} else {
			draw_h_line(region_pos[0][i], i, region_pos[1][i] - region_pos[0][i]);
		}
	}
	GDEV_UNLOCK();
}


/**
   @brief	塗りつぶした三角形の領域を描画する

   @param[in]	x0	頂点0のX座標
   @param[in]	y0	頂点0のY座標
   @param[in]	x1	頂点1のX座標
   @param[in]	y1	頂点1のY座標
   @param[in]	x2	頂点2のX座標
   @param[in]	y2	頂点2のY座標
*/
void draw_triangle_region(short x0, short y0,
			  short x1, short y1,
			  short x2, short y2)
{
	draw_vertex4_region(x0, y0, x1, y1, x2, y2, x2, y2);
}


static void set_arc(short x, short y, short r, char q, unsigned char lr)
{
	short px, py, F;

	px = r;
	py = 0;
	F = -2 * r + 3;

	while(px >= py) {
		switch(q) {
		case 0:
			//draw_point(x + px, y - py);
			//draw_point(x + py, y - px);
			if((0 <= (y-py)) && ((y-py) < GSC_GRAPHICS_DISPLAY_HEIGHT)) {
				region_pos[lr][y - py] = x + px;
			}
			if((0 <= (y-px)) && ((y-px) < GSC_GRAPHICS_DISPLAY_HEIGHT)) {
				region_pos[lr][y - px] = x + py;
			}
			break;

		case 1:
			//draw_point(x - px, y - py);
			//draw_point(x - py, y - px);
			if((0 <= (y-py)) && ((y-py) < GSC_GRAPHICS_DISPLAY_HEIGHT)) {
				region_pos[lr][y - py] = x - px;
			}
			if((0 <= (y-px)) && ((y-px) < GSC_GRAPHICS_DISPLAY_HEIGHT)) {
				region_pos[lr][y - px] = x - py;
			}
			break;

		case 2:
			//draw_point(x - px, y + py);
			//draw_point(x - py, y + px);
			if((0 <= (y+py)) && ((y+py) < GSC_GRAPHICS_DISPLAY_HEIGHT)) {
				region_pos[lr][y + py] = x - px;
			}
			if((0 <= (y+px)) && ((y+px) < GSC_GRAPHICS_DISPLAY_HEIGHT)) {
				region_pos[lr][y + px] = x - py;
			}
			break;

		case 3:
			//draw_point(x + px, y + py);
			//draw_point(x + py, y + px);
			if((0 <= (y+py)) && ((y+py) < GSC_GRAPHICS_DISPLAY_HEIGHT)) {
				region_pos[lr][y + py] = x + px;
			}
			if((0 <= (y+px)) && ((y+px) < GSC_GRAPHICS_DISPLAY_HEIGHT)) {
				region_pos[lr][y + px] = x + py;
			}
			break;
		}
		if(F >= 0) {
			px--;
			F -= 4 * px;
		}
		py++;
		F += 4 * py + 2;
	}
}


/**
   @brief	扇形を描画する

   @param[in]	x	中心のX座標
   @param[in]	y	中心のY座標
   @param[in]	er	外径
   @param[in]	ir	内径
   @param[in]	q	象限(0:第1象限 〜 3:4象限)
*/
void draw_sector(short x, short y,	// 中心の座標
		 short er,	// 外形
		 short ir,	// 内径
		 char q	// 象限(0:第1象限 〜 3:4象限)
		 )
{
	short i, tp, bt;

	switch(q) {
	case 0:
		set_arc(x, y, er, q, 1);
		set_arc(x, y, ir, q, 0);
		tp = y-er;
		bt = y-ir;
		if(tp < 0) tp = 0;
		if(GSC_GRAPHICS_DISPLAY_HEIGHT <= bt) bt = (GSC_GRAPHICS_DISPLAY_HEIGHT - 1);
		for(i=tp; i<bt; i++) {
			region_pos[0][i] = x;
		}
		GDEV_LOCK();
		for(i=(y-er); i<y; i++) {
			draw_h_line(region_pos[0][i], i, region_pos[1][i] - region_pos[0][i]);
		}
		GDEV_UNLOCK();
		break;

	case 1:
		set_arc(x, y, er, q, 0);
		set_arc(x, y, ir, q, 1);
		tp = y-er;
		bt = y-ir;
		if(tp < 0) tp = 0;
		if(GSC_GRAPHICS_DISPLAY_HEIGHT <= bt) bt = (GSC_GRAPHICS_DISPLAY_HEIGHT - 1);
		for(i=tp; i<bt; i++) {
			region_pos[1][i] = x;
		}
		GDEV_LOCK();
		for(i=(y-er); i<y; i++) {
			draw_h_line(region_pos[0][i], i, region_pos[1][i] - region_pos[0][i]);
		}
		GDEV_UNLOCK();
		break;

	case 2:
		set_arc(x, y, er, q, 0);
		set_arc(x, y, ir, q, 1);
		tp = y+ir;
		bt = y+er;
		if(tp < 0) tp = 0;
		if(GSC_GRAPHICS_DISPLAY_HEIGHT <= bt) bt = (GSC_GRAPHICS_DISPLAY_HEIGHT - 1);
		for(i=tp; i<bt; i++) {
			region_pos[1][i] = x;
		}
		GDEV_LOCK();
		for(i=y; i<(y+er); i++) {
			draw_h_line(region_pos[0][i], i, region_pos[1][i] - region_pos[0][i]);
		}
		GDEV_UNLOCK();
		break;

	case 3:
		set_arc(x, y, er, q, 1);
		set_arc(x, y, ir, q, 0);
		tp = y+ir;
		bt = y+er;
		if(tp < 0) tp = 0;
		if(GSC_GRAPHICS_DISPLAY_HEIGHT <= bt) bt = (GSC_GRAPHICS_DISPLAY_HEIGHT - 1);
		for(i=tp; i<bt; i++) {
			region_pos[0][i] = x;
		}
		GDEV_LOCK();
		for(i=y; i<(y+er); i++) {
			draw_h_line(region_pos[0][i], i, region_pos[1][i] - region_pos[0][i]);
		}
		GDEV_UNLOCK();
		break;
	}
}


static void _draw_data(short x, short y, short width, short height,
		       short offset, void *data, short dw)
{
	struct st_rect rect;
	int j;
	PIXEL_DATA *dp;

	DTFPRINTF(0x01, "\n");
	DTPRINTF(0x01, "x=%d y=%d\n", x, y);
	DTPRINTF(0x01, "w=%d h=%d o=%d dw=%d\n", width, height, offset, dw);
	DTPRINTF(0x01, "data=%p\n", data);

	rect.left = x;
	rect.top = y;
	rect.right = x + width - 1;
	rect.bottom = y + height - 1;

	ioctl_device(fb_dev, IOCMD_VIDEO_SETRECT, 0, (void *)&rect);

	for(j=0; j<height; j++) {
		dp = (PIXEL_DATA *)data + (dw * j) + offset;
		DTPRINTF(0x02, "dp=%p\n", dp);
		write_device(fb_dev, (unsigned char *)dp, width * sizeof(*dp));
	}
}

/**
   @brief	イメージデータを描画する

   @param[in]	px	描画X座標
   @param[in]	py	描画Y座標
   @param[in]	width	描画幅
   @param[in]	height	描画高さ
   @param[in]	image	イメージデータポインタ
   @param[in]	dw	イメージデータ幅
*/
void draw_image(short px, short py, short width, short height, void *image, short dw)
{
	struct st_rect wrect;
	struct st_rect drect;

	wrect.left	= px;
	wrect.top	= py;
	wrect.right	= px + width;
	wrect.bottom	= py + height;

	and_rect(&drect, &wrect, &clip_rect);

	if(empty_rect(&drect) == 0) {
		short offset = 0;
		if((clip_rect.top <= wrect.top) &&
		   (wrect.top < clip_rect.bottom)) {
		} else {
			image += (clip_rect.top - wrect.top) * width;
			offset = (clip_rect.left - wrect.left);
		}

		if((clip_rect.left <= wrect.left) && 
		   (wrect.left < clip_rect.right)) {
		} else {
			image += (clip_rect.left-wrect.left);
		}
		DTPRINTF(0x02, "image = %p\n", image);

		GDEV_LOCK();
		_draw_data(drect.left, drect.top,
			   drect.right-drect.left, drect.bottom-drect.top,
			   offset, image, dw);
		GDEV_UNLOCK();
	}
}

static unsigned int graph_copy_buf[GSC_GRAPHICS_DISPLAY_WIDTH];

/**
   @brief	矩形範囲を縦方向にスクロールする

   @param[in]	rect	スクロール範囲矩形
   @param[in]	pixel	スクロールドット数
*/
void scroll_rect_v(struct st_rect *rect, short pixel)
{
	struct st_rect tmp = *rect;
	struct st_rect crect;
	int dwidth;
	short height;
	short pos_x = rect->top;
	int i;

	if(pixel == 0) {
		return;
	}

#if 0
	if(tmp.left < tmp.right) {
		tmp.right --;
	}
	if(tmp.top < tmp.bottom) {
		tmp.bottom --;
	}
#endif

	dwidth = (tmp.right - tmp.left) * color_depth;

	crect.left  = tmp.left;
	crect.right = tmp.right;

	GDEV_LOCK();
	if(pixel < 0) {
		height= (tmp.bottom - tmp.top) + pixel;
		for(i=0; i<height; i++) {
			crect.top = pos_x - pixel + i;
			crect.bottom = crect.top + 1;

			DTPRINTF(0x01, "- copy from rect(%d, %d, %d, %d)\n", crect.left, crect.top, crect.right, crect.bottom);
			ioctl_device(fb_dev, IOCMD_VIDEO_SETRECT, 0, (void *)&crect);
			read_device(fb_dev, (unsigned char *)graph_copy_buf, dwidth);

			crect.top = pos_x + i;
			crect.bottom = crect.top + 1;

			DTPRINTF(0x01, "- copy to   rect(%d, %d, %d, %d)\n", crect.left, crect.top, crect.right, crect.bottom);
			ioctl_device(fb_dev, IOCMD_VIDEO_SETRECT, 0, (void *)&crect);
			write_device(fb_dev, (unsigned char *)graph_copy_buf, dwidth);
		}
	} else {
		height= (tmp.bottom - tmp.top) - pixel;
		for(i=height; i>=0; i--) {
			crect.top = pos_x + i;
			crect.bottom = crect.top + 1;

			DTPRINTF(0x01, "+ copy from rect(%d, %d, %d, %d)\n", crect.left, crect.top, crect.right, crect.bottom);
			ioctl_device(fb_dev, IOCMD_VIDEO_SETRECT, 0, (void *)&crect);
			read_device(fb_dev, (unsigned char *)graph_copy_buf, dwidth);

			crect.top = pos_x + pixel + i;
			crect.bottom = crect.top + 1;

			DTPRINTF(0x01, "+ copy to   rect(%d, %d, %d, %d)\n", crect.left, crect.top, crect.right, crect.bottom);
			ioctl_device(fb_dev, IOCMD_VIDEO_SETRECT, 0, (void *)&crect);
			write_device(fb_dev, (unsigned char *)graph_copy_buf, dwidth);
		}
	}
	GDEV_UNLOCK();
}
