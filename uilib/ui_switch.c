/** @file
    @brief	ユーザインタフェース - スイッチ

    @date	2018.02.04
    @auther	Takashi SHUDO
*/

#include "graphics.h"
#include "tkprintf.h"
#include "font.h"

#include "ui_style.h"
#include "ui_switch.h"

//#define DEBUGTBITS 0x01
#include "dtprintf.h"

static const struct st_graph_object sw_normal_color[] = {
	{ GO_TYPE_FORECOLOR,	{ UI_NORMAL_FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ UI_BACK_COLOR } },
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_graph_object sw_active_color[] = {
	{ GO_TYPE_FORECOLOR,	{ UI_ACTIVE_FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ UI_BACK_COLOR } },
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_graph_object sw_inactive_color[] = {
	{ GO_TYPE_FORECOLOR,	{ UI_INACTIVE_FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ UI_BACK_COLOR } },
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_graph_object sw_on_color[] = {
	{ GO_TYPE_FORECOLOR,	{ UI_ON_COLOR } },
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_graph_object sw_off_color[] = {
	{ GO_TYPE_FORECOLOR,	{ UI_OFF_COLOR } },
	{ 0, { 0, 0, 0, 0 }}
};


static void draw_knob(struct st_ui_switch *ui_switch)
{
	struct st_ui_switch *sw = ui_switch;

	sw->knob_area.pos.x = sw->text_area.pos.x + sw->text_area.sur.width + sw->text_area.sur.height/8;
	sw->knob_area.pos.y = sw->text_area.pos.y + sw->text_area.sur.height/8;
	sw->knob_area.sur.width = sw->text_area.sur.height/8*6;
	sw->knob_area.sur.height = sw->knob_area.sur.width;

	if(sw->value == 0) {
		if(sw->off_color == 0) {
			draw_graph_object(sw->switch_area.pos.x, sw->switch_area.pos.y, sw_off_color);
		} else {
			draw_graph_object(sw->switch_area.pos.x, sw->switch_area.pos.y, sw->off_color);
		}
	} else {
		if(sw->on_color == 0) {
			draw_graph_object(sw->switch_area.pos.x, sw->switch_area.pos.y, sw_on_color);
		} else {
			draw_graph_object(sw->switch_area.pos.x, sw->switch_area.pos.y, sw->on_color);
		}
		sw->knob_area.pos.x += sw->knob_area.sur.width;
	}

	draw_round_fill_box(&sw->knob_area, sw->knob_area.sur.width/2);
}

static void draw_switch(struct st_ui_switch *ui_switch)
{
	struct st_ui_switch *sw = ui_switch;

	if(sw->switch_area.pos.x == 0) {
		sw->switch_area.pos.x = sw->text_area.pos.x + sw->text_area.sur.width;
	}
	if(sw->switch_area.pos.y == 0) {
		sw->switch_area.pos.y = sw->text_area.pos.y;
	}
	if(sw->switch_area.sur.height == 0) {
		sw->switch_area.sur.height = sw->text_area.sur.height;
	}
	if(sw->switch_area.sur.width == 0) {
		sw->switch_area.sur.width = sw->switch_area.sur.height*2/8*7;
	}

	switch(sw->status) {
	case UI_SWITCH_ST_INACTIVE:
		if(sw->inactive_color == 0) {
			draw_graph_object(sw->switch_area.pos.x, sw->switch_area.pos.y, sw_inactive_color);
		} else {
			draw_graph_object(sw->switch_area.pos.x, sw->switch_area.pos.y, sw->inactive_color);
		}
		break;

	case UI_SWITCH_ST_NORMAL:
		if(sw->normal_color == 0) {
			draw_graph_object(sw->switch_area.pos.x, sw->switch_area.pos.y, sw_normal_color);
		} else {
			draw_graph_object(sw->switch_area.pos.x, sw->switch_area.pos.y, sw->normal_color);
		}
		break;

	case UI_SWITCH_ST_SELECT:
		if(sw->active_color == 0) {
			draw_graph_object(sw->switch_area.pos.x, sw->switch_area.pos.y, sw_active_color);
		} else {
			draw_graph_object(sw->switch_area.pos.x, sw->switch_area.pos.y, sw->active_color);
		}
		break;

	default:
		SYSERR_PRINT("Invalid status %d\n", sw->status);
		break;
	}

	set_draw_mode(GRP_DRAWMODE_NORMAL);
	draw_round_fill_box(&sw->switch_area, sw->text_area.sur.height/2);
	draw_knob(sw);
}

void draw_ui_switch(struct st_ui_switch *ui_switch)
{
	struct st_ui_switch *sw = ui_switch;

#ifdef DEBUG
	set_forecolor(RGB(255,0,0));
	draw_box(&(sw->text_area));
#endif

	switch(sw->status) {
	case UI_SWITCH_ST_INACTIVE:
		if(sw->inactive_color == 0) {
			draw_graph_object(sw->switch_area.pos.x, sw->switch_area.pos.y, sw_inactive_color);
		} else {
			draw_graph_object(sw->switch_area.pos.x, sw->switch_area.pos.y, sw->inactive_color);
		}
		break;

	default:
		if(sw->active_color == 0) {
			draw_graph_object(sw->switch_area.pos.x, sw->switch_area.pos.y, sw_active_color);
		} else {
			draw_graph_object(sw->switch_area.pos.x, sw->switch_area.pos.y, sw->active_color);
		}
		break;
	}

	if(sw->font_name == 0) {
		// フォント未指定
	} else {
		(void)set_font_by_name(sw->font_name);
	}
	draw_str_in_box(&(sw->text_area), FONT_HATTR_LEFT, FONT_VATTR_CENTER, sw->name);

	draw_switch(sw);
}

void draw_ui_switch_list(struct st_ui_switch **ui_switchs)
{
	struct st_ui_switch **sw = ui_switchs;

	while(*sw != 0) {
		draw_ui_switch(*sw);
		sw ++;
	}
}

int proc_ui_switch(struct st_ui_switch *ui_switch, struct st_sysevent *event)
{
	struct st_ui_switch *sw = ui_switch;
	int rt = UI_SWITCH_EVT_NULL;

	switch(event->what) {
	case EVT_TOUCHSTART:
		if(is_point_in_box(event->pos_x, event->pos_y,
				   (struct st_box *)&(sw->switch_area)) != 0) {
			if(sw->status != UI_SWITCH_ST_SELECT) {
				sw->status = UI_SWITCH_ST_SELECT;
				draw_switch(sw);
			}
			rt = UI_SWITCH_EVT_NULL;
		}
		break;

	case EVT_TOUCHEND:
		if(is_point_in_box(event->pos_x, event->pos_y,
				   (struct st_box *)&(sw->switch_area)) != 0) {
			if(sw->status == UI_SWITCH_ST_SELECT) {
				sw->status = UI_SWITCH_ST_NORMAL;
				if(sw->value == 0) {
					sw->value = 1;
					rt = UI_SWITCH_EVT_ON;
				} else {
					sw->value = 0;
					rt = UI_SWITCH_EVT_OFF;
				}
				draw_switch(sw);
			} else {
				return UI_SWITCH_EVT_NULL;
			}
		} else {
			if(sw->status == UI_SWITCH_ST_SELECT) {
				sw->status = UI_SWITCH_ST_NORMAL;
				draw_switch(sw);
			}
			rt = UI_SWITCH_EVT_NULL;
		}
		break;

	default:
		break;
	}

	return rt;
}

int proc_ui_switch_list(struct st_switch_event *ui_switch_event, struct st_ui_switch **ui_switchs, struct st_sysevent *event)
{
	struct st_ui_switch **sw = ui_switchs;
	int flg_evt = 0;

	while(*sw != 0) {
		int obj_ev = proc_ui_switch(*sw, event);
		if(obj_ev != UI_SWITCH_EVT_NULL) {
			// VIWE範囲が重なっていない限り同時にイベントは発生しないはず
			ui_switch_event->id = (*sw)->id;
			ui_switch_event->what = obj_ev;
			flg_evt = 1;
			DTPRINTF(0x01, "UI_EVT ID=%d, WHAT=%d\n", ui_switch_event->id, ui_switch_event->what);
		}
		sw ++;
	}

	return flg_evt;
}
