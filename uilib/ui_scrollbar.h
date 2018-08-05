/** @file
    @brief	ユーザインタフェース - スクロールバー

    @date	2017.06.11
    @auther	Takashi SHUDO
*/

#ifndef UI_SCROLLBAR_H
#define UI_SCROLLBAR_H

#include "sysevent.h"
#include "graphics.h"
#include "graphics_object.h"
#include "ui_selectlist.h"

#define UI_SCB_ST_STILL		0
#define UI_SCB_ST_SEL_KNOB	1
#define UI_SCB_ST_SEL_UPONKNOB	2
#define UI_SCB_ST_SEL_UNDERKNOB	3

#define UI_SCB_EVT_NULL		0
#define UI_SCB_EVT_MOVE		1

struct st_ui_scrollbar {
	struct st_box view_area;	///< 表示エリア
	const struct st_graph_object *normal_view;	///< 通常時表示グラフィックス
	const struct st_graph_object *selected_view;	///< 選択時表示グラフィックス

	struct st_box upon_knob_area;	///< つまみの上エリア
	struct st_box knob_area;	///< つまみのエリア
	struct st_box under_knob_area;	///< つまみの下エリア

	int status;	///< 状態
	short touch_px;	///< タッチされた位置X座標
	short touch_py;	///< タッチされた位置Y座標
	int touch_top_item_num;	///< 表示される一番上のアイテム番号

	int item_count;	///< アイテム数
	int disp_count;	///< 表示アイテム数
	int top_num;	///< 表示される一番上のアイテム番号

	struct st_ui_selectlist *selectlist;
}; ///< UIスクロールバー

void draw_ui_scrollbar(struct st_ui_scrollbar *scrollbar);
void setup_ui_scrollbar(struct st_ui_scrollbar *scrollbar, int item_count, int disp_count, int top_num);
int proc_ui_scrollbar(struct st_ui_scrollbar *scrollbar, struct st_sysevent *event);

#endif // UI_SCROLLBAR_H
