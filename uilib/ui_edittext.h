/** @file
    @brief	ユーザインタフェース - エディットテキスト

    @date	2018.01.29
    @auther	Takashi SHUDO
*/

#ifndef UI_EDITTEXT_H
#define UI_EDITTEXT_H

#include "sysevent.h"
#include "graphics.h"
#include "graphics_object.h"

#define TEXT_AREA_MARGINE	6

struct st_ui_edittext {
	struct st_box view_area;	///< 表示エリア
	int flg_active;			///< アクティブフラグ
	const struct st_graph_object *normal_view;	///< 通常時表示グラフィックス
	const struct st_graph_object *active_view;	///< アクティブ時表示グラフィックス
	const struct st_graph_object *cursor_view;	///< カーソル表示時グラフィックス
	unsigned char *text;	///< 編集テキスト文字列ポインタ
	int max_text_length;	///< 編集テキスト文字列最大長
	int cursor_pos;		///< 編集テキストカーソル位置
	int (* filter)(struct st_ui_edittext *te, unsigned char ch);	///< 文字編集フィルタ関数
}; ///< UIエディットテキスト

void draw_ui_edittext(struct st_ui_edittext *textedit);
void draw_ui_edittext_list(struct st_ui_edittext **textedit);
int proc_ui_edittext(struct st_ui_edittext *textedit, struct st_sysevent *event);
int proc_ui_edittext_list(struct st_ui_edittext **textedit, struct st_sysevent *event);
int set_char_ui_edittext(struct st_ui_edittext *textedit, unsigned char ch);
int set_char_ui_edittext_list(struct st_ui_edittext **textedit, unsigned char ch);
int move_cursor_ui_edittext(struct st_ui_edittext *textedit, int direction);
int move_cursor_ui_edittext_list(struct st_ui_edittext **textedit, int direction);

#endif // UI_EDITTEXT_H
