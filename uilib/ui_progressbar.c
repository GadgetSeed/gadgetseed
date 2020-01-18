/** @file
    @brief	ユーザインタフェース - プログレスバー

    @date	2019.03.03
    @auther	Takashi SHUDO
*/

#include "graphics.h"
#include "tprintf.h"
#include "ui_scrollbar.h"

#include "ui_style.h"
#include "ui_progressbar.h"

//#define DEBUGTBITS 0x01
#include "dtprintf.h"

static const struct st_graph_object progressbar_color_view[] = {
	{ GO_TYPE_FORECOLOR,	{ UI_NORMAL_FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ UI_NORMAL_BACK_COLOR } },
	{ 0, { 0, 0, 0, 0 }}
};


static int value_width(struct st_ui_progressbar *pb, int value)
{
	int rt = 0;

	rt = (((long long)pb->view_area.sur.width-2) * value)/pb->max_value;
	DTPRINTF(0x01, "value_width=%d\n", rt);

	return rt;
}

static void update_bar(struct st_ui_progressbar *pb, int value)
{
	struct st_box bar_box;

	if(pb->value != value) {
		int old_width = value_width(pb, pb->value);
		int new_width = value_width(pb, value);

		if(pb->view == 0) {
			draw_graph_object(pb->view_area.pos.x, pb->view_area.pos.y, progressbar_color_view);
		} else {
			draw_graph_object(pb->view_area.pos.x, pb->view_area.pos.y, pb->view);
		}

		if(pb->value < value) {
			// 増える場合
			DTPRINTF(0x01, "up value=%d\n", new_width - old_width);
			bar_box.pos.x = pb->view_area.pos.x + 1 + old_width;
			bar_box.pos.y = pb->view_area.pos.y + 1;
			bar_box.sur.width = new_width - old_width;
			bar_box.sur.height = pb->view_area.sur.height - 2;
			set_draw_mode(GRP_DRAWMODE_NORMAL);
			draw_fill_box(&bar_box);
		} else {
			// 減る場合
			DTPRINTF(0x01, "down value=%d\n", old_width - new_width);
			bar_box.pos.x = pb->view_area.pos.x + 1 + new_width;
			bar_box.pos.y = pb->view_area.pos.y + 1;
			bar_box.sur.width = old_width - new_width;
			bar_box.sur.height = pb->view_area.sur.height - 2;
			set_draw_mode(GRP_DRAWMODE_REVERSE);
			draw_fill_box(&bar_box);
			set_draw_mode(GRP_DRAWMODE_NORMAL);
		}
		pb->value = value;
		//draw_ui_progressbar(pb);
	}
}

static void draw_bar(struct st_ui_progressbar *pb)
{
	struct st_box bar_box_l;
	struct st_box bar_box_h;
	int bar_width = value_width(pb, pb->value);

	// バー値描画枠を計算
	bar_box_l.pos.x = pb->view_area.pos.x + 1;
	bar_box_l.pos.y = pb->view_area.pos.y + 1;
	bar_box_l.sur.width = bar_width;
	bar_box_l.sur.height = pb->view_area.sur.height - 2;

	// バー残り描画枠を計算
	bar_box_h.pos.x = pb->view_area.pos.x + 1 + bar_width;
	bar_box_h.pos.y = pb->view_area.pos.y + 1;
	bar_box_h.sur.width = pb->view_area.sur.width - 2 - bar_width;
	bar_box_h.sur.height = pb->view_area.sur.height - 2;

	if(pb->view == 0) {
		draw_graph_object(pb->view_area.pos.x, pb->view_area.pos.y, progressbar_color_view);
	} else {
		draw_graph_object(pb->view_area.pos.x, pb->view_area.pos.y, pb->view);
	}

//#define DEBUG
#ifdef DEBUG
	set_forecolor(RGB(255,0,0));
	set_backcolor(RGB(0,0,255));
#endif
	draw_fill_box(&bar_box_l);

	// バー残り描画
	set_draw_mode(GRP_DRAWMODE_REVERSE);
	draw_fill_box(&bar_box_h);

	set_draw_mode(GRP_DRAWMODE_NORMAL);
}

void draw_ui_progressbar(struct st_ui_progressbar *progressbar)
{
	struct st_ui_progressbar *pb =progressbar;

	// 主にプログレスバーの色設定
	if(pb->view == 0) {
		draw_graph_object(pb->view_area.pos.x, pb->view_area.pos.y, progressbar_color_view);
	} else {
		draw_graph_object(pb->view_area.pos.x, pb->view_area.pos.y, pb->view);
	}

#if 0
	// 背景色で矩形を描画
	set_draw_mode(GRP_DRAWMODE_REVERSE);
	draw_fill_box(&(pb->view_area));
#endif

	// 描画色で枠を描画
	set_draw_mode(GRP_DRAWMODE_NORMAL);
	draw_box(&(pb->view_area));

	draw_bar(pb);
}

void set_value_ui_progressbar(struct st_ui_progressbar *progressbar, int value)
{
	struct st_ui_progressbar *pb =progressbar;

	if(value > pb->max_value) {
		value = pb->max_value;
	}

	if(value < 0) {
		value = 0;
	}

	update_bar(pb, value);
}
