/** @file
    @brief	ユーザインタフェース - ボタン

    @date	2017.05.28
    @auther	Takashi SHUDO
*/

#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include "sysevent.h"
#include "graphics.h"
#include "graphics_object.h"

#define UI_BUTTON_ST_NORMAL	0	///< UIボタン状態 通常
#define UI_BUTTON_ST_SELECT	1	///< UIボタン状態 選択

#define UI_BUTTON_EVT_NULL	0	///< UIボタンイベント 無し
#define UI_BUTTON_EVT_PUSH	1	///< UIボタンイベント 押された
#define UI_BUTTON_EVT_PULL	2	///< UIボタンイベント 離された
#define UI_BUTTON_EVT_DRAG	3	///< UIボタンイベント ドラッグされた
#define UI_BUTTON_EVT_RELEASE	4	///< UIボタンイベント 範囲外で離された
#define UI_BUTTON_EVT_REPEAT	5	///< UIボタンイベント 押しっぱなしでリピート

struct st_ui_button_image {
	const struct st_graph_object *normal_view;	///< ボタン通常表示グラフィックス
	const struct st_graph_object *select_view;	///< ボタン選択時表示グラフィックス
}; ///< UIボタングラフィックス

struct st_button_event {
	unsigned short id;	/// イベントが発生したボタンID
	unsigned short what;	/// ボタンイベント種類
}; ///< UIボタンイベント

struct st_ui_button {
	int id;		///< ボタンID
	const struct st_box view_area;		///< ボタン位置、範囲
	const struct st_ui_button_image *view;	///< ボタングラフィックス
	char *font_name;		///< フォント名
	char *name;
	int status;
	unsigned long long when;
}; ///< UIボタン

void draw_ui_button(struct st_ui_button *object);
void draw_ui_button_list(struct st_ui_button **objects);
int proc_ui_button(struct st_ui_button *object, struct st_sysevent *event);
int proc_ui_button_list(struct st_button_event *obj_event, struct st_ui_button **objects, struct st_sysevent *event);

#endif // UI_BUTTON_H
