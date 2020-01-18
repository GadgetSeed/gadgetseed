/** @file
    @brief	ユーザインタフェース - ボタン

    @date	2017.05.28
    @auther	Takashi SHUDO
*/

#include "graphics.h"
#include "tprintf.h"
#include "font.h"

#include "ui_style.h"
#include "ui_button.h"
#include "sysevent.h"

#ifndef GSC_UI_BUTTON_REPEAT_TIME	/// $gsc UIボタン長押し時のリピートイベント発生までの時間(ms)
#define GSC_UI_BUTTON_REPEAT_TIME	200
#endif

//#define DEBUGTBITS 0x04
#include "dtprintf.h"


//#define BUTTON_ROUND	8
#define BUTTON_ROUND	(object->view_area.sur.height/8)

static const struct st_graph_object bt_normal_view[] = {
	{ GO_TYPE_FORECOLOR,	{ UI_NORMAL_FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ UI_NORMAL_BACK_COLOR } },
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_graph_object bt_active_view[] = {
	{ GO_TYPE_FORECOLOR,	{ UI_ACTIVE_FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ UI_ACTIVE_BACK_COLOR } },
	{ 0, { 0, 0, 0, 0 }}
};

static void draw_normal_button(struct st_ui_button *object)
{
	draw_graph_object(object->view_area.pos.x, object->view_area.pos.y, bt_normal_view);

	set_draw_mode(GRP_DRAWMODE_REVERSE);
	draw_round_fill_box((struct st_box *)&(object->view_area), BUTTON_ROUND);

	set_draw_mode(GRP_DRAWMODE_NORMAL);
	draw_round_box((struct st_box *)&(object->view_area), BUTTON_ROUND);

	draw_str_in_box((struct st_box *)&(object->view_area), FONT_HATTR_CENTER, FONT_VATTR_CENTER,
			(uchar *)(object->name));
}

static void draw_select_button(struct st_ui_button *object)
{
	draw_graph_object(object->view_area.pos.x, object->view_area.pos.y, bt_active_view);

	set_draw_mode(GRP_DRAWMODE_REVERSE);
	draw_round_fill_box((struct st_box *)&(object->view_area), BUTTON_ROUND);

	set_draw_mode(GRP_DRAWMODE_NORMAL);
	draw_round_box((struct st_box *)&(object->view_area), BUTTON_ROUND);

	draw_str_in_box((struct st_box *)&(object->view_area), FONT_HATTR_CENTER, FONT_VATTR_CENTER,
			(uchar *)(object->name));
}

void draw_ui_button(struct st_ui_button *object)
{
	if(object->font_name != 0) {
		(void)set_font_by_name(object->font_name);
	} else {
		(void)set_font_by_name(GSC_FONTS_DEFAULT_FONT);
	}

	switch(object->status) {
	case UI_BUTTON_ST_NORMAL:
		if(object->view == 0) {
			draw_normal_button(object);
		} else if(object->view->normal_view == 0) {
			draw_normal_button(object);
		} else {
			draw_graph_object(object->view_area.pos.x, object->view_area.pos.y, object->view->normal_view);
		}
		break;

	case UI_BUTTON_ST_SELECT:
		if(object->view == 0) {
			draw_select_button(object);
		} else if(object->view->select_view == 0) {
			draw_normal_button(object);
		} else {
			draw_graph_object(object->view_area.pos.x, object->view_area.pos.y, object->view->select_view);
		}
		break;

	default:
		return;
		break;
	}
}

void draw_ui_button_list(struct st_ui_button **objects)
{
	struct st_ui_button **uo = objects;

	while(*uo != 0) {
		draw_ui_button(*uo);
		uo ++;
	}
}

int proc_ui_button(struct st_ui_button *object, struct st_sysevent *event)
{
	switch(event->what) {
	case EVT_TOUCHSTART:
		if(is_point_in_box(event->pos_x, event->pos_y,
				   (struct st_box *)&(object->view_area)) != 0) {
			if(object->status == UI_BUTTON_ST_NORMAL) {
				object->status = UI_BUTTON_ST_SELECT;
				object->when = event->when;
				draw_ui_button(object);
				return UI_BUTTON_EVT_PUSH;
			} else {
				return UI_BUTTON_EVT_NULL;
			}
		}
		break;

	case EVT_TOUCHEND:
		if(is_point_in_box(event->pos_x, event->pos_y,
				   (struct st_box *)&(object->view_area)) != 0) {
			if(object->status == UI_BUTTON_ST_SELECT) {
				object->status = UI_BUTTON_ST_NORMAL;
				draw_ui_button(object);
				return UI_BUTTON_EVT_PULL;
			} else {
				return UI_BUTTON_EVT_NULL;
			}
		} else {
			if(object->status == UI_BUTTON_ST_SELECT) {
				object->status = UI_BUTTON_ST_NORMAL;
				draw_ui_button(object);
				return UI_BUTTON_EVT_RELEASE;
			} else {
				return UI_BUTTON_EVT_NULL;
			}
		}
		break;

	default:
		{
			int rt = UI_BUTTON_EVT_NULL;
			if(object->status == UI_BUTTON_ST_SELECT) {
				unsigned long long intv = event->when - object->when;
				DTPRINTF(0x04, "btn = %llu, evt = %llu\n", object->when, event->when);
				if(intv >= GSC_UI_BUTTON_REPEAT_TIME) {
					object->when -= GSC_UI_BUTTON_REPEAT_TIME;
					rt = UI_BUTTON_EVT_REPEAT;
				}
			}
			return rt;
		}
		break;
	}

	return UI_BUTTON_EVT_NULL;
}

int proc_ui_button_list(struct st_button_event *obj_event, struct st_ui_button **objects, struct st_sysevent *event)
{
	struct st_ui_button **uo = objects;
	int flg_evt = 0;

	while(*uo != 0) {
		int obj_ev = proc_ui_button(*uo, event);
		if(obj_ev != UI_BUTTON_EVT_NULL) {
			// VIWE範囲が重なっていない限り同時にイベントは発生しないはず
			obj_event->id = (*uo)->id;
			obj_event->what = obj_ev;
			flg_evt = 1;
			DTPRINTF(0x01, "UI_EVT ID=%d, WHAT=%d\n", obj_event->id, obj_event->what);
		}
		uo ++;
	}

	return flg_evt;
}
