/** @file
    @brief	ユーザインタフェース - シークバー

    @date	2017.06.21
    @auther	Takashi SHUDO
*/

#ifndef UI_SEEKBAR_H
#define UI_SEEKBAR_H

#include "sysevent.h"
#include "graphics.h"
#include "graphics_object.h"

#define UI_SKB_ST_STILL		0
#define UI_SKB_ST_SEL_KNOB	1
#define UI_SKB_ST_SEL_UPONKNOB	2
#define UI_SKB_ST_SEL_UNDERKNOB	3

#define UI_SKB_TYPE_VERTICAL	0
#define UI_SKB_TYPE_HOLIZONTAL	1

#define UI_SKB_ATTR_REALTIME_VALUE_CAHNGE	0
#define UI_SKB_ATTR_DROP_VALUE_CHANGE	1

#define UI_SKB_EVT_NULL		0
#define UI_SKB_EVT_CHANGE	1

struct st_ui_seekbar {
	struct st_box view_area;	///< 表示エリア
	int type;
	int attr;
	const struct st_graph_object *normal_view;
	const struct st_graph_object *selected_view;
	unsigned int bar_color;

	int status;

	int max_value;
	int value;
	int last_value;

	struct st_box body_area;
	short body_area_diameter;

	struct st_box bar_area;
	struct st_box bar_left_area;
	short bar_area_diameter;

	struct st_box knob_area;
	short knob_area_diameter;

	short touch_px;
	short touch_py;
}; ///< UIシークバー

void draw_ui_seekbar(struct st_ui_seekbar *slider);
void set_value_ui_seekbar(struct st_ui_seekbar *slider, int value);
int proc_ui_seekbar(struct st_ui_seekbar *slider, struct st_sysevent *event, int *value);

#endif // UI_SEEKBAR_H
