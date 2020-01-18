/** @file
    @brief	ユーザインタフェース - スイッチ

    @date	2018.02.04
    @auther	Takashi SHUDO
*/

#ifndef UI_SWITCH_H
#define UI_SWITCH_H

#include "sysevent.h"
#include "graphics.h"
#include "graphics_object.h"

#define UI_SWITCH_ST_INACTIVE	0
#define UI_SWITCH_ST_NORMAL	1
#define UI_SWITCH_ST_SELECT	2

#define UI_SWITCH_EVT_NULL	0
#define UI_SWITCH_EVT_OFF	1
#define UI_SWITCH_EVT_ON	2


struct st_switch_event {
	unsigned short id;	///< イベントが発生したスイッチID
	unsigned short what;	///< UIスイッチイベント種類
}; ///< UIスイッチイベント

struct st_ui_switch {
	int id;		///< スイッチID
	struct st_box text_area;
	struct st_box switch_area;
	struct st_box knob_area;
	const struct st_graph_object *normal_color;	///< 通常時表示色
	const struct st_graph_object *active_color;	///< アクティブ時表示色
	const struct st_graph_object *inactive_color;	///< インアクティブ時表示色
	const struct st_graph_object *on_color;		///< On色
	const struct st_graph_object *off_color;	///< Off色
	char *font_name;
	unsigned char *name;
	int status;
	int value;
}; ///< UIスイッチ

void draw_ui_switch(struct st_ui_switch *ui_switch);
void draw_ui_switch_list(struct st_ui_switch **ui_switchs);
int proc_ui_switch(struct st_ui_switch *ui_switch, struct st_sysevent *event);
int proc_ui_switch_list(struct st_switch_event *ui_switch_event, struct st_ui_switch **ui_switchs, struct st_sysevent *event);

#endif // UI_SWITCH_H
