/** @file
    @brief	グラフィックライブラリ

    @date	2007.03.20
    @author	Takashi SHUDO
*/

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "sysconfig.h"
#include "device.h"

#define MAX_FRAMEBUF	2	///< 最大フレームバッファ数

#ifdef GSC_GRAPHICS_COLOR_16BIT	///< $gsc グラフィックデバイスは16ビットカラー
typedef unsigned short	PIXEL_DATA;
#endif
#ifdef GSC_GRAPHICS_COLOR_24BIT	///< $gsc グラフィックデバイスは24ビットカラー
typedef unsigned int	PIXEL_DATA;
#endif
#ifdef GSC_GRAPHICS_COLOR_32BIT	///< $gsc グラフィックデバイスは32ビットカラー
typedef unsigned int	PIXEL_DATA;
#endif

// 16bitカラー変換
#define RGB16(r,g,b)	(unsigned int)(((b) >> 3) | (((g) >> 2) << 5) | (((r) >> 3) << 11))

// 24bitカラー変換
#define RGB24(r,g,b)	(unsigned int)((b) | ((g) << 8) | ((r) << 16))
#define RGB24_TO_R(c)	(unsigned char)(((c) >> 16) & 0xff)
#define RGB24_TO_G(c)	(unsigned char)(((c) >>  8) & 0xff)
#define RGB24_TO_B(c)	(unsigned char)(((c) >>  0) & 0xff)

// 32bitカラー変換(ARGB)
#define RGB32(r,g,b)	(unsigned int)(0xff000000 | (b) | ((g) << 8) | ((r) << 16))
#define RGB32_TO_R(c)	(unsigned char)(((c) >> 16) & 0xff)
#define RGB32_TO_G(c)	(unsigned char)(((c) >>  8) & 0xff)
#define RGB32_TO_B(c)	(unsigned char)(((c) >>  0) & 0xff)

// 色変換
#if defined(GSC_GRAPHICS_COLOR_16BIT)
#define RGB RGB16
#elif defined(GSC_GRAPHICS_COLOR_24BIT)
#define RGB RGB24
#define RGB_TO_R	RGB24_TO_R
#define RGB_TO_G	RGB24_TO_G
#define RGB_TO_B	RGB24_TO_B
#elif defined(GSC_GRAPHICS_COLOR_32BIT)
#define RGB RGB32
#define RGB_TO_R	RGB32_TO_R
#define RGB_TO_G	RGB32_TO_G
#define RGB_TO_B	RGB32_TO_B
#endif

// set_draw_mode(mode)
/*
  "fore color" & "back color"
 */
#define GRP_DRAWMODE_NORMAL	0	///< Draw the foreground with fore color. Draw the background with back color.
#define GRP_DRAWMODE_REVERSE	1	///< Draw the foreground with back color. Draw the background with fore color.
#define GRP_DRAWMODE_FOREONLY	2	///< Draw the foreground with back color. No draw the background.

struct st_rect {
	short left;	///< 左上頂点のX座標
	short top;	///< 左上頂点のY座標
	short right;	///< 右下頂点のX座標
	short bottom;	///< 右下頂点のY座標
};	///< 矩形

struct st_position {
	short x;	///< X座標
	short y;	///< Y座標
};	///< 位置

struct st_surface {
	short width;	///< 幅
	short height;	///< 高さ
};	///< 面

struct st_box {
	struct st_position pos;	///< 左上頂点の位置
	struct st_surface sur;	///< 面の大きさ
};	///< 長(正)方形

struct st_bitmap {
	short width;	///< 幅
	short height;	///< 高さ
	unsigned char *data;	///< ビットマップデータポインタ
};	///< ビットマップグラフィックデータ

struct st_graphics_context {
	struct st_device *fb_dev;
	short screen_width;
	short screen_height;
	unsigned short color_depth;
	unsigned short frame_num;
	unsigned char *frame_buf_ptr[MAX_FRAMEBUF];
	unsigned int mem_size;
	unsigned int pen_color;
	unsigned int back_color;
	unsigned char pen_mode;
	struct st_rect i_clip_rect;
	struct st_rect clip_rect;
	unsigned char pen_pat[8];
};	///< [TODO] 複数グラフィックデバイスに対応したい

extern int register_graphics_dev(struct st_device *dev);
extern int init_graphics(char *devname);

extern int get_frame_num(void);
extern int set_display_frame(int fnum);
extern int get_display_frame(void);
extern int set_draw_frame(int fnum);
extern int get_draw_frame(void);
extern void get_screen_info(short *width, short *height);

extern void clear_screen(void);
extern void fill_screen(void);
extern void set_forecolor(unsigned int color);
extern unsigned int get_forecolor(void);
extern void set_backcolor(unsigned int color);
extern unsigned int get_backcolor(void);
extern void set_draw_mode(unsigned char mode);
extern unsigned char get_draw_mode(void);

extern void and_rect(struct st_rect *a, struct st_rect *s1, struct st_rect *s2);
extern void or_rect(struct st_rect *a, struct st_rect *s1, struct st_rect *s2);
extern short empty_rect(struct st_rect *rect);
extern short empty_box(struct st_box *box);
extern void correct_rect(struct st_rect *rect);
extern void clear_clip_rect(void);
extern void set_clip_rect(struct st_rect *rect);
extern void set_clip_box(struct st_box *box);
extern void get_clip_rect(struct st_rect *rect);

extern void draw_point(short x, short y);
extern void draw_h_line(short x, short y, short width);
extern void draw_v_line(short x, short y, short height);
extern void draw_line(short x, short y, short xe, short ye);
extern void draw_rect(struct st_rect *rect);
extern void draw_fill_rect(struct st_rect *rect);
extern void draw_bitdata(short px, short py, short width, short height, unsigned char *data, short dw);

extern void draw_enlarged_bitdata(short px, short py, short width, short height, unsigned char *data, short dw, int rate);
extern void draw_bitmap(short px, short py, struct st_bitmap *bitmap);
extern void draw_enlarged_bitmap(short px, short py, struct st_bitmap *bitmap, int rate);

extern void draw_circle(short x0, short y0, short r);
extern void draw_fill_circle(short x0, short y0, short r);
extern void draw_ellipse(short xc, short yc, short rx, short ry);
extern void draw_fill_ellipse(short xc, short yc, short rx, short ry);
#if 0
extern void scroll_screen(short x, short y);
#endif

extern void draw_box(struct st_box *box);
extern void draw_round_box(struct st_box *box, short r);
extern void draw_fill_box(struct st_box *box);
extern void draw_round_fill_box(struct st_box *box, short r);
extern void draw_vertex4_region(short x0, short y0, short x1, short y1, short x2, short y2, short x3, short y3);
extern void draw_triangle_region(short x0, short y0, short x1, short y1, short x2, short y2);

extern void draw_round_rect(struct st_rect *rect, short r);
extern void draw_round_fill_rect(struct st_rect *rect, short r);
extern void draw_quarter_circle(short x0, short y0, short r, char q);
extern void draw_sector(short x, short y, short er, short ir, char q);

extern int is_point_in_rect(short x, short y, struct st_rect *rect);
extern int is_point_in_box(short x, short y, struct st_box *box);

extern void box2rect(struct st_rect *rect, struct st_box *box);

extern void draw_image(short px, short py, short width, short height, void *image, short dw);

extern void scroll_rect_v(struct st_rect *rect, short pixcel);

extern void resize_image(void *dst_image, short dwidth, short dheight,
			 void *src_image, short swidth, short sheight);

#endif // GRAPHICS_H
