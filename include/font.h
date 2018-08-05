/** @file
    @brief	フォント
 
    @date	2007.03.25
    @author	Takashi SHUDO
*/

#ifndef INCLUDE_FONT_H
#define INCLUDE_FONT_H

#include "str.h"
#include "graphics.h"
#include "fontdata.h"

#define FONT_FIXEDWIDTH		0	///< 全フォントを固定幅で描画
#define FONT_PROPORTIONAL	1	///< 各フォントの幅で描画

#define FONT_HATTR_LEFT		0	///< 左寄せ
#define FONT_HATTR_CENTER	1	///< 中心
#define FONT_HATTR_RIGHT	2	///< 右寄せ

#define FONT_VATTR_TOP		0	///< 上寄せ
#define FONT_VATTR_CENTER	1	///< 中心
#define FONT_VATTR_BOTTOM	2	///< 下寄せ

extern void init_font(void);
extern void set_fontset(struct st_fontset *fontset);
extern struct st_fontset * get_fontset(void);
extern int fontset_count(void);
extern const char * fontset_name(int num);
extern struct st_fontset * get_fontptr_by_name(char *name);
extern struct st_fontset * set_font_by_name(char *name);
extern void set_font_drawmode(int mode);
extern unsigned short draw_char(short x, short y, ushort ch);
extern void draw_str(short x, short y, uchar *str);
extern void draw_fixed_width_str(short x, short y, uchar *str, short width);
extern void draw_str_in_box(struct st_box *box, int hattr, int vattr, unsigned char *str);
extern unsigned short font_width(unsigned short ch);
extern unsigned short str_width(uchar *str);
extern unsigned short font_height(void);

#endif // INCLUDE_FONT_H
