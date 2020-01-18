/** @file
    @brief	ユーザインタフェース - 静的テキスト

    @date	2019.11.17
    @auther	Takashi SHUDO
*/

#ifndef UI_STATICTEXT_H
#define UI_STATICTEXT_H

#include "graphics.h"

#define UI_STATICTEXT_FILLARRT_NOFILL	0
#define UI_STATICTEXT_FILLARRT_FILL	1

struct st_ui_statictext {
	struct st_box view_area;	///< 表示エリア
	const unsigned int *fore_color;
	const unsigned int *back_color;
	char *font_name;		///< フォント名
	unsigned char hattr;		///< 横方向位置属性
	unsigned char vattr;		///< 縦方向位置属性
	unsigned char fillattr;		///< 塗りつぶし属性
	unsigned char *text;		///< 編集テキスト文字列ポインタ
}; ///< UIテキスト

void draw_ui_statictext(struct st_ui_statictext *statictext);

#endif // UI_STATICTEXT_H
