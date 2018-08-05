/** @file
    @brief	ユーザインタフェース - シークバー

    @date	2017.06.21
    @auther	Takashi SHUDO
*/

#include "ui_seekbar.h"
#include "tprintf.h"

//#define CHECK_AREA

//#define DEBUGTBITS 0x02
#include "dtprintf.h"


static int need_redraw_knob(struct st_ui_seekbar *slider, int value)
{
	struct st_ui_seekbar *sd = slider;
	int knob_length;
	int knob_start;
	short knob_x;
	short knob_y;

	switch(sd->type) {
	case UI_SKB_TYPE_VERTICAL:
		knob_length = sd->view_area.sur.height - sd->view_area.sur.width;
		knob_start = sd->view_area.pos.y + sd->view_area.sur.height - sd->view_area.sur.width/2;
		knob_y = knob_start - ((knob_length * value) / sd->max_value) - sd->view_area.sur.width/2;

		if(sd->knob_area.pos.y != knob_y) {
			return 1;
		}
		break;

	case UI_SKB_TYPE_HOLIZONTAL:
		knob_length = sd->view_area.sur.width - sd->view_area.sur.height;
		knob_start = sd->view_area.pos.x + sd->view_area.sur.height/2;
		knob_x = knob_start + ((knob_length * value) / sd->max_value) - sd->view_area.sur.height/2;

		if(sd->knob_area.pos.x != knob_x) {
			return 1;
		}
		break;

	default:
		return 0;
		break;
	}

	return 0;
}

static void calc_knob_position(struct st_ui_seekbar *slider)
{
	struct st_ui_seekbar *sd = slider;
	int knob_length;
	int knob_start;
	short knob_x;
	short knob_y;

	switch(sd->type) {
	case UI_SKB_TYPE_VERTICAL:
		knob_length = sd->view_area.sur.height - sd->view_area.sur.width;
		knob_start = sd->view_area.pos.y + sd->view_area.sur.height - sd->view_area.sur.width/2;

		knob_x = sd->view_area.pos.x + sd->view_area.sur.width/2;
		knob_y = knob_start - ((knob_length * sd->value) / sd->max_value);
		sd->knob_area_diameter = sd->view_area.sur.width/2;

		sd->knob_area.pos.x = knob_x - sd->view_area.sur.width/2;
		sd->knob_area.pos.y = knob_y - sd->view_area.sur.width/2;
		sd->knob_area.sur.width = sd->view_area.sur.width;
		sd->knob_area.sur.height = sd->view_area.sur.width;
		break;

	case UI_SKB_TYPE_HOLIZONTAL:
		knob_length = sd->view_area.sur.width - sd->view_area.sur.height;
		knob_start = sd->view_area.pos.x + sd->view_area.sur.height/2;

		knob_x = knob_start + ((knob_length * sd->value) / sd->max_value);
		knob_y = sd->view_area.pos.y + sd->view_area.sur.height/2;
		sd->knob_area_diameter = sd->view_area.sur.height/2;

		sd->knob_area.pos.x = knob_x - sd->view_area.sur.height/2;
		sd->knob_area.pos.y = knob_y - sd->view_area.sur.height/2;
		sd->knob_area.sur.width = sd->view_area.sur.height;
		sd->knob_area.sur.height = sd->view_area.sur.height;
		break;

	default:
		return;
		break;
	} 
}

static void draw_knob(struct st_ui_seekbar *slider, int flg_clear)
{
	struct st_ui_seekbar *sd = slider;

	calc_knob_position(sd);

#ifdef CHECK_AREA
	draw_box(&(sd->knob_area));
#endif

	if(sd->normal_view != 0) {
		draw_graph_object(sd->view_area.pos.x, sd->view_area.pos.y, sd->normal_view);
	}

	if(flg_clear != 0) {
		set_draw_mode(GRP_DRAWMODE_REVERSE);
	} else {
		set_draw_mode(GRP_DRAWMODE_NORMAL);
	}

	draw_round_fill_box(&(sd->knob_area), sd->knob_area_diameter);
	//draw_fill_circle(knob_x, knob_y, knob_r);
	//draw_circle(knob_x, knob_y, knob_r);
}

static void calc_body_position(struct st_ui_seekbar *slider)
{
	struct st_ui_seekbar *sd = slider;

#ifdef CHECK_AREA
	draw_box(&(sd->view_area));
#endif

	switch(sd->type) {
	case UI_SKB_TYPE_VERTICAL:
		sd->body_area.pos.x = sd->view_area.pos.x + sd->view_area.sur.width/4;
		sd->body_area.pos.y = sd->view_area.pos.y + sd->view_area.sur.width/4;
		sd->body_area.sur.width = sd->view_area.sur.width/2;
		sd->body_area.sur.height = sd->view_area.sur.height - sd->view_area.sur.width/2;
		sd->body_area_diameter = sd->view_area.sur.width/4;
		break;

	case UI_SKB_TYPE_HOLIZONTAL:
		sd->body_area.pos.x = sd->view_area.pos.x + sd->view_area.sur.height/4;
		sd->body_area.pos.y = sd->view_area.pos.y + sd->view_area.sur.height/4;
		sd->body_area.sur.width = sd->view_area.sur.width - sd->view_area.sur.height/2;
		sd->body_area.sur.height = sd->view_area.sur.height/2;
		sd->body_area_diameter = sd->view_area.sur.height/4;
		break;

	default:
		return;
		break;
	}
}

static void draw_body(struct st_ui_seekbar *slider)
{
	struct st_ui_seekbar *sd = slider;

	calc_body_position(sd);

	if(sd->normal_view != 0) {
		draw_graph_object(sd->view_area.pos.x, sd->view_area.pos.y, sd->normal_view);
	}

	set_draw_mode(GRP_DRAWMODE_NORMAL);
	draw_round_box(&(sd->body_area), sd->body_area_diameter);
}

static void calc_bar_position(struct st_ui_seekbar *slider)
{
	struct st_ui_seekbar *sd = slider;

	calc_knob_position(sd);

	switch(sd->type) {
	case UI_SKB_TYPE_VERTICAL:
		sd->bar_area.pos.x = sd->body_area.pos.x + 1;
		sd->bar_area.pos.y = sd->knob_area.pos.y + sd->view_area.sur.width/2 + 1;
		sd->bar_area.sur.width = sd->body_area.sur.width - 2;
		sd->bar_area.sur.height = sd->body_area.sur.height - (sd->knob_area.pos.y - sd->body_area.pos.y) - sd->view_area.sur.width/2 - 2;
		sd->bar_area_diameter = sd->body_area_diameter - 2;

		sd->bar_left_area.pos.x = sd->bar_area.pos.x;
		sd->bar_left_area.pos.y = sd->body_area.pos.y + 1;
		sd->bar_left_area.sur.width = sd->bar_area.sur.width;
		sd->bar_left_area.sur.height = sd->body_area.sur.height - sd->bar_area.sur.height - 2;
		break;

	case UI_SKB_TYPE_HOLIZONTAL:
		sd->bar_area.pos.x = sd->body_area.pos.x + 1;
		sd->bar_area.pos.y = sd->body_area.pos.y + 1;
		sd->bar_area.sur.width = (sd->knob_area.pos.x - sd->body_area.pos.x) + sd->view_area.sur.height/2 - 2;
		sd->bar_area.sur.height = sd->body_area.sur.height - 2;
		sd->bar_area_diameter = sd->body_area_diameter - 2;

		sd->bar_left_area.pos.x = sd->knob_area.pos.x;
		sd->bar_left_area.pos.y = sd->bar_area.pos.y;
		sd->bar_left_area.sur.width = sd->body_area.sur.width - sd->bar_area.sur.width - 2;
		sd->bar_left_area.sur.height = sd->body_area.sur.height - 2;
		break;

	default:
		return;
		break;
	}
}

static void draw_bar(struct st_ui_seekbar *slider)
{
	struct st_ui_seekbar *sd = slider;

	calc_bar_position(sd);

	if(sd->normal_view != 0) {
		draw_graph_object(sd->view_area.pos.x, sd->view_area.pos.y, sd->normal_view);
	}

	set_draw_mode(GRP_DRAWMODE_REVERSE);
	//set_draw_mode(GRP_DRAWMODE_NORMAL); // DEBUG
	//set_forecolor(RGB(255,0,0)); // DEBUF
	draw_round_fill_box(&(sd->bar_left_area), sd->bar_area_diameter);

	set_draw_mode(GRP_DRAWMODE_NORMAL);
	set_forecolor(sd->bar_color);
	draw_round_fill_box(&(sd->bar_area), sd->bar_area_diameter);
}

void draw_ui_seekbar(struct st_ui_seekbar *slider)
{
	struct st_ui_seekbar *sd = slider;

	draw_body(sd);
	draw_bar(sd);
	draw_knob(sd, 0);
}

static void set_value(struct st_ui_seekbar *slider, int value)
{
	struct st_ui_seekbar *sd = slider;

	if(value == sd->value) {
		return;
	}

	if(value < 0) {
		return;
	}

	if(value > sd->max_value) {
		return;
	}

	// [TODO] 再描画が必要か判定
	if(need_redraw_knob(sd, value) == 0) {
		// 分解能以下であればノブの位置が変わらないので再描画しない
		return;
	}

	draw_knob(sd, 1);

	set_clip_box(&(sd->knob_area));
	draw_body(sd);
	clear_clip_rect();

	slider->value = value;

	calc_knob_position(sd);
	draw_bar(sd);
	draw_knob(sd, 0);
}

void set_value_ui_seekbar(struct st_ui_seekbar *slider, int value)
{
	struct st_ui_seekbar *sd = slider;

	if(sd->status == UI_SKB_ST_SEL_KNOB) {
		// つまみをドラッグ中は値を設定できない
		return;
	}

	set_value(sd, value);
}

int proc_ui_seekbar(struct st_ui_seekbar *slider, struct st_sysevent *event, int *value)
{
	struct st_ui_seekbar *sd = slider;
	int rtn = UI_SKB_EVT_NULL;

	switch(event->what) {
	case EVT_TOUCHSTART:
		if(is_point_in_box(event->pos_x, event->pos_y, &(sd->knob_area)) != 0) {
			DTPRINTF(0x01, "SEEKBAR KNOB\n");
			sd->touch_px = event->pos_x - sd->knob_area.pos.x - sd->knob_area.sur.width/2;
			sd->touch_py = event->pos_y - sd->knob_area.pos.y - sd->knob_area.sur.height/2;
			sd->status = UI_SKB_ST_SEL_KNOB;
			sd->last_value = sd->value;
			DTPRINTF(0x01, "DY = %d\n", sd->touch_py);
		}
		break;

	case EVT_TOUCHMOVE:
		if(sd->status == UI_SKB_ST_SEL_KNOB) {
			int new_value;
			int max_pos;
			int now_pos;

			switch(sd->type) {
			case UI_SKB_TYPE_VERTICAL:
				max_pos = sd->view_area.sur.height - sd->view_area.sur.width;
				now_pos = (sd->view_area.pos.y + sd->view_area.sur.height - sd->view_area.sur.width/2)
						- (event->pos_y - sd->touch_py);
				break;

			case UI_SKB_TYPE_HOLIZONTAL:
				max_pos = sd->view_area.sur.width - sd->view_area.sur.height;
				now_pos = (0 - sd->view_area.pos.x - sd->view_area.sur.height/2)
						+ (event->pos_x - sd->touch_px);
				break;

			default:
				return rtn;
				break;
			}

			new_value = (now_pos * sd->max_value) / max_pos;
			DTPRINTF(0x01, "SY = %d/%d %d\n", now_pos, max_pos, new_value);
			if(sd->value != new_value) {
				if((new_value >= 0) && (new_value <= sd->max_value)) {
					*value = new_value;
					set_value(sd, new_value);
					if(sd->attr == UI_SKB_ATTR_REALTIME_VALUE_CAHNGE) {
						rtn = UI_SKB_EVT_CHANGE;
					}
				}
			}
		}
		break;

	case EVT_TOUCHEND:
		if(sd->status == UI_SKB_ST_SEL_KNOB) {
			sd->status = UI_SKB_ST_STILL;
			//draw_ui_seekbar(sd);
			DTPRINTF(0x01, "SEEKBAR STILL\n");
			if(sd->attr == UI_SKB_ATTR_DROP_VALUE_CHANGE) {
				if(sd->last_value != sd->value) {
					*value = sd->value;
					rtn = UI_SKB_EVT_CHANGE;
				}
			}
		}
		break;

	default:
		break;
	}

	return rtn;
}
