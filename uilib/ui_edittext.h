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

#define UI_EDITTEXT_EVT_NULL	0	///< UIイベント 無し
#define UI_EDITTEXT_EVT_PUSH	1	///< UIイベント 押された
#define UI_EDITTEXT_EVT_PULL	2	///< UIイベント 離された
#define UI_EDITTEXT_EVT_DRAG	3	///< UIイベント ドラッグされた
#define UI_EDITTEXT_EVT_RELEASE	4	///< UIイベント 範囲外で離された

struct st_ui_edittext {
	struct st_box view_area;	///< 表示エリア
	int text_area_margin;		///< 文字余白
	char *font_name;		///< フォント名
	int flg_uneditable;		///< 編集不可フラグ
	int flg_active;			///< アクティブ(編集中)フラグ
	int flg_enable_multibyte_select;	///< 複数文字選択有効フラグ
	const struct st_graph_object *normal_view;	///< 通常時表示グラフィックス
	const struct st_graph_object *uneditable_view;	///< 編集不可表示グラフィックス
	const struct st_graph_object *active_view;	///< アクティブ時表示グラフィックス
	const struct st_graph_object *cursor_view;	///< カーソル表示時グラフィックス
	unsigned char *text;	///< 編集テキスト文字列ポインタ
	int max_text_length;	///< 編集テキスト文字列最大長
	int cursor_pos_top;		///< 編集テキストカーソル先頭位置
	int cursor_pos_end;		///< 編集テキストカーソル末尾位置
	int (* filter)(struct st_ui_edittext *te, unsigned char ch);	///< 文字編集フィルタ関数
}; ///< UIエディットテキスト

void draw_ui_edittext(struct st_ui_edittext *textedit);
void editable_ui_edittext(struct st_ui_edittext *textedit);
void editable_ui_edittext_list(struct st_ui_edittext **textedit);
void uneditable_ui_edittext(struct st_ui_edittext *textedit);
void uneditable_ui_edittext_list(struct st_ui_edittext **textedit);
void activate_ui_edittext(struct st_ui_edittext *textedit);
void inactivate_ui_edittext(struct st_ui_edittext *textedit);
void inactivate_ui_edittext_list(struct st_ui_edittext **textedit);
void draw_ui_edittext_list(struct st_ui_edittext **textedit);
int proc_ui_edittext(struct st_ui_edittext *textedit, struct st_sysevent *event);
int proc_ui_edittext_list(struct st_ui_edittext **textedit, struct st_sysevent *event, struct st_ui_edittext **last_te);
int set_char_ui_edittext(struct st_ui_edittext *textedit, unsigned char ch);
int set_char_ui_edittext_list(struct st_ui_edittext **textedit, unsigned char ch, int all_select);
int move_cursor_ui_edittext(struct st_ui_edittext *textedit, int direction);
int move_cursor_ui_edittext_list(struct st_ui_edittext **textedit, int direction, int all_select);
void select_all_ui_edittext(struct st_ui_edittext *textedit);
void unactive_ui_edittext(struct st_ui_edittext *textedit);
void unactive_ui_edittext_list(struct st_ui_edittext **textedit);
int backspace_char_ui_edittext(struct st_ui_edittext *textedit);
int backspace_char_ui_edittext_list(struct st_ui_edittext **textedit);
int delete_char_ui_edittext(struct st_ui_edittext *textedit);
int delete_char_ui_edittext_list(struct st_ui_edittext **textedit);

#endif // UI_EDITTEXT_H
