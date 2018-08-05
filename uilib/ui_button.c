/** @file
    @brief	ユーザインタフェース - ボタン

    @date	2017.05.28
    @auther	Takashi SHUDO
*/

#include "graphics.h"
#include "tprintf.h"
#include "ui_button.h"
#include "sysevent.h"

//#define DEBUGTBITS 0x02
#include "dtprintf.h"


void draw_ui_button(struct st_ui_button *object)
{
	const struct st_graph_object *gobj;

	switch(object->status) {
	case UI_BUTTON_ST_NORMAL:
		gobj = object->view->normal_view;
		break;

	case UI_BUTTON_ST_SELECT:
		gobj = object->view->selected_view;
		break;

	default:
		return;
		break;
	}

	draw_graph_object(object->view->box.pos.x, object->view->box.pos.y, gobj);
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
				   (struct st_box *)&(object->view->box)) != 0) {
			if(object->status == UI_BUTTON_ST_NORMAL) {
				object->status = UI_BUTTON_ST_SELECT;
				draw_ui_button(object);
				return UI_BUTTON_EVT_PUSH;
			} else {
				return UI_BUTTON_EVT_NULL;
			}
		}
		break;

	case EVT_TOUCHEND:
		if(is_point_in_box(event->pos_x, event->pos_y,
				   (struct st_box *)&(object->view->box)) != 0) {
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
		return UI_BUTTON_EVT_NULL;
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
