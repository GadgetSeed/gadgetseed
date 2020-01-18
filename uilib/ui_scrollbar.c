/** @file
    @brief	ユーザインタフェース - スクロールバー

    @date	2017.06.11
    @auther	Takashi SHUDO
*/

#include "graphics.h"
#include "tprintf.h"
#include "sysevent.h"

#include "ui_style.h"
#include "ui_scrollbar.h"

//#define DEBUGTBITS 0x02
#include "dtprintf.h"


#define SCRBAR_H	3

static const struct st_graph_object sb_normal_view[] = {
	{ GO_TYPE_FORECOLOR,	{ UI_NORMAL_FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ UI_NORMAL_BACK_COLOR } },
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_graph_object sb_select_view[] = {
	{ GO_TYPE_FORECOLOR,	{ UI_ACTIVE_FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ UI_ACTIVE_BACK_COLOR } },
	{ 0, { 0, 0, 0, 0 }}
};


void draw_ui_scrollbar(struct st_ui_scrollbar *scrollbar)
{
	struct st_ui_scrollbar *sb = scrollbar;
	short px, py;
	short b_top, b_bottom;

	px = sb->view_area.pos.x + 1;
	py = sb->view_area.pos.y + 1;

	if(sb->item_count == 0) {
		b_top = 0;
		b_bottom = sb->view_area.sur.height - 2;
	} else if(sb->item_count < sb->disp_count) {
		b_top = 0;
		b_bottom = sb->view_area.sur.height - 2;
	} else {
		b_top = ((sb->top_num
			  * (sb->view_area.sur.height - SCRBAR_H)) /
			 sb->item_count);

		b_bottom = (((sb->top_num + sb->disp_count)
			     * (sb->view_area.sur.height - SCRBAR_H)) /
			    sb->item_count);

		b_bottom = b_bottom + SCRBAR_H - 2;
	}

	sb->upon_knob_area.pos.x = px;
	sb->upon_knob_area.sur.width = sb->view_area.sur.width - 2;
	sb->upon_knob_area.pos.y = py;
	sb->upon_knob_area.sur.height = b_top;

	sb->knob_area.pos.x = px;
	sb->knob_area.sur.width = sb->view_area.sur.width - 2;
	sb->knob_area.pos.y = py + b_top;
	sb->knob_area.sur.height = b_bottom - b_top;

	sb->under_knob_area.pos.x = px;
	sb->under_knob_area.sur.width = sb->view_area.sur.width - 2;
	sb->under_knob_area.pos.y = py + b_bottom;
	sb->under_knob_area.sur.height = sb->view_area.sur.height - b_bottom - 2;

	if(sb->normal_view == 0) {
		draw_graph_object(sb->view_area.pos.x, sb->view_area.pos.y, sb_normal_view);
	} else {
		draw_graph_object(sb->view_area.pos.x, sb->view_area.pos.y, sb->normal_view);
	}

	draw_box(&(sb->view_area));

	if(sb->status == UI_SCB_ST_SEL_UPONKNOB) {
		if(sb->select_view == 0) {
			draw_graph_object(sb->view_area.pos.x, sb->view_area.pos.y, sb_select_view);
		} else {
			draw_graph_object(sb->view_area.pos.x, sb->view_area.pos.y, sb->select_view);
		}
	} else {
		if(sb->normal_view == 0) {
			draw_graph_object(sb->view_area.pos.x, sb->view_area.pos.y, sb_normal_view);
		} else {
			draw_graph_object(sb->view_area.pos.x, sb->view_area.pos.y, sb->normal_view);
		}
	}
	set_draw_mode(GRP_DRAWMODE_REVERSE);
	draw_fill_box(&(sb->upon_knob_area));

	if(sb->status == UI_SCB_ST_SEL_KNOB) {
		if(sb->select_view == 0) {
			draw_graph_object(sb->view_area.pos.x, sb->view_area.pos.y, sb_select_view);
		} else {
			draw_graph_object(sb->view_area.pos.x, sb->view_area.pos.y, sb->select_view);
		}
	} else {
		if(sb->normal_view == 0) {
			draw_graph_object(sb->view_area.pos.x, sb->view_area.pos.y, sb_normal_view);
		} else {
			draw_graph_object(sb->view_area.pos.x, sb->view_area.pos.y, sb->normal_view);
		}
	}
	set_draw_mode(GRP_DRAWMODE_NORMAL);
	draw_fill_box(&(sb->knob_area));

	if(sb->status == UI_SCB_ST_SEL_UNDERKNOB) {
		if(sb->select_view == 0) {
			draw_graph_object(sb->view_area.pos.x, sb->view_area.pos.y, sb_select_view);
		} else {
			draw_graph_object(sb->view_area.pos.x, sb->view_area.pos.y, sb->select_view);
		}
	} else {
		if(sb->normal_view == 0) {
			draw_graph_object(sb->view_area.pos.x, sb->view_area.pos.y, sb_normal_view);
		} else {
			draw_graph_object(sb->view_area.pos.x, sb->view_area.pos.y, sb->normal_view);
		}
	}
	set_draw_mode(GRP_DRAWMODE_REVERSE);
	draw_fill_box(&(sb->under_knob_area));

	set_draw_mode(GRP_DRAWMODE_NORMAL);
}

void setup_ui_scrollbar(struct st_ui_scrollbar *scrollbar, int item_count, int disp_count, int top_num)
{
	scrollbar->item_count = item_count;
	scrollbar->disp_count = disp_count;
	scrollbar->top_num = top_num;

	draw_ui_scrollbar(scrollbar);
}

int proc_ui_scrollbar(struct st_ui_scrollbar *scrollbar, struct st_sysevent *event)
{
	struct st_ui_scrollbar *sb = scrollbar;
	int rtn = UI_SCB_EVT_NULL;

	switch(event->what) {
	case EVT_TOUCHSTART:
		if(is_point_in_box(event->pos_x, event->pos_y, &(sb->knob_area)) != 0) {
			DTPRINTF(0x01, "SB KNOB\n");
			sb->touch_px = event->pos_x;
			sb->touch_py = event->pos_y;
			sb->touch_top_item_num = sb->top_num;
			sb->status = UI_SCB_ST_SEL_KNOB;
		} else
		if(is_point_in_box(event->pos_x, event->pos_y, &(sb->upon_knob_area)) != 0) {
			DTPRINTF(0x01, "SB UPON_KNOB\n");
			sb->status = UI_SCB_ST_SEL_UPONKNOB;
			if(sb->selectlist != 0) {
				scroll_ui_selectlist(sb->selectlist, -1);
			}
		} else
		if(is_point_in_box(event->pos_x, event->pos_y, &(sb->under_knob_area)) != 0) {
			DTPRINTF(0x01, "SB UNDER_KNOB\n");
			sb->status = UI_SCB_ST_SEL_UNDERKNOB;
			if(sb->selectlist != 0) {
				scroll_ui_selectlist(sb->selectlist, 1);
			}
		}

		if(sb->status != UI_SCB_ST_STILL) {
			draw_ui_scrollbar(sb);
			rtn = UI_SCB_EVT_MOVE;
		}
		break;

	case EVT_TOUCHMOVE:
		switch(sb->status) {
		case UI_SCB_ST_STILL:
			break;

		case UI_SCB_ST_SEL_KNOB:
			DTPRINTF(0x01, "SB KNOB DRAG\n");
			{
				int dy = 0;
				int top_num = 0;
				int last_top_num = 0;

				if(sb->disp_count >= sb->item_count) {
					break;
				}

				last_top_num = sb->top_num;
				dy = event->pos_y - sb->touch_py;
				DTPRINTF(0x01, "DY = %d\n", dy);
				top_num = sb->touch_top_item_num +
						(dy * (sb->item_count - sb->disp_count)) /
						(sb->view_area.sur.height - sb->knob_area.sur.height);
				DTPRINTF(0x01, "TOP_NUM = %d\n", top_num);

				if(top_num != last_top_num) {
					set_top_item_num_ui_selectlist(sb->selectlist, top_num);
					rtn = UI_SCB_EVT_MOVE;
				}
			}
			break;

		case UI_SCB_ST_SEL_UPONKNOB:
			DTPRINTF(0x01, "SB UPONKNOB DRAG\n");
			break;

		case UI_SCB_ST_SEL_UNDERKNOB:
			DTPRINTF(0x01, "SB UNDERKNOB DRAG\n");
			break;

		default:
			break;
		}
		break;

	case EVT_TOUCHEND:
		DTPRINTF(0x01, "SB STILL\n");
		switch(sb->status) {
		case UI_SCB_ST_SEL_KNOB:
			sb->status = UI_SCB_ST_STILL;
			draw_ui_scrollbar(sb);
			break;

		default:
			break;
		}
		break;

	default:
		return UI_SCB_EVT_NULL;
	}

	return rtn;
}
