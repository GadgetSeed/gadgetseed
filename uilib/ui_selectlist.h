/** @file
    @brief	ユーザインタフェース - 選択リスト

    @date	2017.05.29
    @auther	Takashi SHUDO
*/

#ifndef UI_LIST_H
#define UI_LIST_H

#include "graphics.h"
#include "graphics_object.h"
#include "sysevent.h"
#include "ui_scrollbar.h"

#define UI_SL_EVT_NULL		0
#define UI_SL_EVT_ITEM_SEL	1

#define UI_SL_ST_STILL		0	///< UI選択リストイベント 静止状態
#define UI_SL_ST_DRAG		1	///< UI選択リストイベント ドラッグ(スクロール)中
#define UI_SL_ST_INERTIA	2	///< UI選択リストイベント 慣性(スクロール)中

struct st_ui_selectlist_event {
	int item_num;		///< イベントが発生したアイテム番号
	unsigned short what;	///< UI選択リストイベント種類
}; ///< UI選択リストイベント

struct st_ui_selectlist {
	struct st_box view_area;	///< 表示エリア
	int item_height;
	const struct st_graph_object *normal_view;
	const struct st_graph_object *select_view;

	int status;
	//int scr_speed;

	short touch_px;
	short touch_py;
	int flg_scroll;

	short drag_px;
	short drag_py;
	//unsigned long long time;

	int drag_top_item_num;

	//int item_width;
	int max_item_disp_count;	///< 表示エリア内に表示可能な最大アイテム数
	int item_disp_count;		///< 表示エリア内に表示されているアイテム数

	int item_count;
	int top_item_num;
	int select_item_num;		///< 選択されているアイテム番号

	void (* item_draw_func)(struct st_ui_selectlist *selectlist, struct st_box *box, int item_num, int flg_select);
	struct st_ui_scrollbar *scrollbar;

	void *userdata;
}; ///< UI選択リスト

void prepare_ui_selectlist(struct st_ui_selectlist *selectlist);
void draw_item_ui_selectlist(struct st_ui_selectlist *selectlist, int item_num);
void draw_ui_selectlist(struct st_ui_selectlist *selectlist);
void set_select_item_num_ui_selectlist(struct st_ui_selectlist *selectlist, int top_num);
void set_top_item_num_ui_selectlist(struct st_ui_selectlist *selectlist, int top_num);
void set_param_ui_selectlist(struct st_ui_selectlist *selectlist, int item_count, int top_num, int select_num);
void scroll_ui_selectlist(struct st_ui_selectlist *selectlist, int direction);
int proc_ui_selectlist(struct st_ui_selectlist_event *sl_event, struct st_ui_selectlist *selectlist, struct st_sysevent *event);

#endif // UI_LIST_H
