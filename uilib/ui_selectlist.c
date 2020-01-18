/** @file
    @brief	ユーザインタフェース - 選択リスト

    @date	2017.05.29
    @auther	Takashi SHUDO
*/

#include "tprintf.h"
//#include "timer.h"
#include "key.h"

#include "ui_style.h"
#include "ui_selectlist.h"
#include "ui_scrollbar.h"

//#define DEBUGTBITS 0x02
#include "dtprintf.h"


static const struct st_graph_object sl_normal_view[] = {
	{ GO_TYPE_FORECOLOR,	{ UI_NORMAL_FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ UI_NORMAL_BACK_COLOR } },
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_graph_object sl_select_view[] = {
	{ GO_TYPE_FORECOLOR,	{ UI_ACTIVE_FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ UI_ACTIVE_BACK_COLOR } },
	{ 0, { 0, 0, 0, 0 }}
};

void prepare_ui_selectlist(struct st_ui_selectlist *selectlist)
{
	struct st_ui_selectlist *sl = selectlist;

	sl->max_item_disp_count = sl->view_area.sur.height / sl->item_height;

	if(sl->max_item_disp_count < sl->item_count) {
		sl->item_disp_count = sl->max_item_disp_count;
	} else {
		sl->item_disp_count = sl->item_count;
	}
}

void draw_item_ui_selectlist(struct st_ui_selectlist *selectlist, int item_num)
{
	struct st_ui_selectlist *sl = selectlist;

	if((item_num >= sl->top_item_num) && (item_num < (sl->top_item_num + sl->max_item_disp_count))) {
		struct st_box vbox;
		int flg_sel = 0;
		vbox.pos.x = sl->view_area.pos.x;
		vbox.pos.y = sl->view_area.pos.y + (sl->item_height * (item_num - sl->top_item_num));
		vbox.sur.width = sl->view_area.sur.width;
		vbox.sur.height = sl->item_height;

		if(item_num == sl->select_item_num) {
			flg_sel = 1;
			// 選択アイテム
			if(sl->select_view == 0) {
				draw_graph_object(vbox.pos.x, vbox.pos.y, sl_select_view);
			} else {
				draw_graph_object(vbox.pos.x, vbox.pos.y, sl->select_view);
			}
		} else {
			// 非選択アイテム
			if(sl->normal_view == 0) {
				draw_graph_object(vbox.pos.x, vbox.pos.y, sl_normal_view);
			} else {
				draw_graph_object(vbox.pos.x, vbox.pos.y, sl->normal_view);
			}
		}

		sl->item_draw_func(sl, &vbox, item_num, flg_sel);
	}
}

void draw_ui_selectlist(struct st_ui_selectlist *selectlist)
{
	struct st_ui_selectlist *sl = selectlist;
	int i;

	if(sl->normal_view == 0) {
		draw_graph_object(sl->view_area.pos.x, sl->view_area.pos.y, sl_normal_view);
	} else {
		draw_graph_object(sl->view_area.pos.x, sl->view_area.pos.y, sl->normal_view);
	}

	prepare_ui_selectlist(sl);

	DTPRINTF(0x01, "MAX_ITEM_DISP_COUNT %d\n", sl->max_item_disp_count);
	DTPRINTF(0x01, "ITEM_DISP_COUNT %d\n", sl->item_disp_count);

	for(i=0; i<sl->item_disp_count; i++) {
		int flg_sel = 0;
		struct st_box vbox;
		int item_num = sl->top_item_num + i;
		vbox.pos.x = sl->view_area.pos.x;
		vbox.pos.y = sl->view_area.pos.y + (sl->item_height * i);
		vbox.sur.width = sl->view_area.sur.width;
		vbox.sur.height = sl->item_height;

		if(item_num == sl->select_item_num) {
			flg_sel = 1;
			// 選択アイテム
			if(sl->select_view == 0) {
				draw_graph_object(vbox.pos.x, vbox.pos.y, sl_select_view);
			} else {
				draw_graph_object(vbox.pos.x, vbox.pos.y, sl->select_view);
			}
		} else {
			// 非選択アイテム
			if(sl->normal_view == 0) {
				draw_graph_object(vbox.pos.x, vbox.pos.y, sl_normal_view);
			} else {
				draw_graph_object(vbox.pos.x, vbox.pos.y, sl->normal_view);
			}
		}

		sl->item_draw_func(sl, &vbox, item_num, flg_sel);
	}

	if(sl->scrollbar != 0) {
		setup_ui_scrollbar(sl->scrollbar, sl->item_count, sl->item_disp_count, sl->top_item_num);
	}
}

static void calc_layout_selectlist(struct st_ui_selectlist *selectlist, int last_sin)
{
	struct st_ui_selectlist *sl = selectlist;

	if((sl->select_item_num - sl->top_item_num) >= sl->max_item_disp_count) {
		// 上スクロール
		DTPRINTF(0x01, "SCROLL UP\n");
		sl->top_item_num = sl->select_item_num - sl->max_item_disp_count + 1;
		if(sl->top_item_num < 0) {
			sl->top_item_num = 0;
		}
		draw_ui_selectlist(sl);
	} else if((sl->select_item_num - sl->top_item_num) < 0) {
		// 下スクロール
		DTPRINTF(0x01, "SCROLL DOWN\n");
		sl->top_item_num = sl->select_item_num;
		draw_ui_selectlist(sl);
	} else {
		struct st_box vbox;

		vbox.pos.x = sl->view_area.pos.x;
		vbox.sur.width = sl->view_area.sur.width;
		vbox.sur.height = sl->item_height;

		// 非選択アイテム
		if(sl->normal_view == 0) {
			draw_graph_object(vbox.pos.x, vbox.pos.y, sl_normal_view);
		} else {
			draw_graph_object(vbox.pos.x, vbox.pos.y, sl->normal_view);
		}

		if((last_sin >= sl->top_item_num) && (last_sin < (sl->top_item_num + sl->max_item_disp_count))) {
			vbox.pos.y = sl->view_area.pos.y + (sl->item_height * (last_sin - sl->top_item_num));
			sl->item_draw_func(sl, &vbox, last_sin, 0);
		}

		vbox.pos.y = sl->view_area.pos.y + (sl->item_height * (sl->select_item_num - sl->top_item_num));

		// 選択アイテム
		if(sl->select_view == 0) {
			draw_graph_object(vbox.pos.x, vbox.pos.y, sl_select_view);
		} else {
			draw_graph_object(vbox.pos.x, vbox.pos.y, sl->select_view);
		}

		sl->item_draw_func(sl, &vbox, sl->select_item_num, 1);
	}
}

void set_select_item_num_ui_selectlist(struct st_ui_selectlist *selectlist, int select_num)
{
	struct st_ui_selectlist *sl = selectlist;
	int last_sin = sl->select_item_num;

	if(select_num < 0) {
		return;
	}

	if(select_num >= sl->item_count) {
		return;
	}

	if(select_num == sl->select_item_num) {
		return;
	}

	sl->select_item_num = select_num;

	calc_layout_selectlist(sl, last_sin);
}

void set_top_item_num_ui_selectlist(struct st_ui_selectlist *selectlist, int top_num)
{
	struct st_ui_selectlist *sl = selectlist;

	if(top_num < 0) {
		return;
	}

	if(top_num == sl->top_item_num) {
		return;
	}

	sl->top_item_num = top_num;

	if(sl->top_item_num > (sl->item_count - sl->max_item_disp_count)) {
		sl->top_item_num = sl->item_count - sl->max_item_disp_count;
	}

	draw_ui_selectlist(sl);
}

void set_param_ui_selectlist(struct st_ui_selectlist *selectlist, int item_count, int top_num, int select_num)
{
	struct st_ui_selectlist *sl = selectlist;

	sl->item_count = item_count;

	sl->top_item_num = top_num;
	if(sl->top_item_num > (sl->item_count - sl->max_item_disp_count)) {
		sl->top_item_num = sl->item_count - sl->max_item_disp_count;
	}
	if(sl->top_item_num < 0) {
		sl->top_item_num = 0;
	}

	sl->select_item_num = select_num;
}

void scroll_ui_selectlist(struct st_ui_selectlist *selectlist, int direction)
{
	struct st_ui_selectlist *sl = selectlist;

	if(direction < 0) {
		sl->top_item_num -= sl->item_disp_count;
		if(sl->top_item_num < 0) {
			sl->top_item_num = 0;
		}
		draw_ui_selectlist(sl);
	} else
	if(direction > 0) {
		sl->top_item_num += sl->item_disp_count;
		if(sl->top_item_num > (sl->item_count - sl->max_item_disp_count)) {
			sl->top_item_num = sl->item_count - sl->max_item_disp_count;
		}
		draw_ui_selectlist(sl);
	}
}

int proc_ui_selectlist(struct st_ui_selectlist_event *sl_event, struct st_ui_selectlist *selectlist, struct st_sysevent *event)
{
	struct st_ui_selectlist *sl = selectlist;
	int last_sin = sl->select_item_num;
	int last_tin = sl->top_item_num;
	int rt_evt = 0;

	switch(event->what) {
	case EVT_KEYDOWN:
	case EVT_KEYDOWN_REPEAT:
		switch(event->arg) {
		case KEY_UP:
		case KEY_GB_UP:
			DTPRINTF(0x01, "UP\n");
			if((sl->select_item_num - 1) >= 0) {
				sl->select_item_num -= 1;
			}
			break;

		case KEY_DOWN:
		case KEY_GB_DOWN:
			DTPRINTF(0x01, "DOWN\n");
			if((sl->select_item_num + 1) < sl->item_count) {
				sl->select_item_num += 1;
			}
			break;
		}
		break;

	case EVT_TOUCHSTART:
		if(is_point_in_box(event->pos_x, event->pos_y, &(sl->view_area)) == 0) {
			break;
		}

		switch(sl->status) {
		case UI_SL_ST_STILL:
			sl->status = UI_SL_ST_DRAG;
			sl->touch_px = event->pos_x;
			sl->touch_py = event->pos_y;
			sl->drag_px = event->pos_x;
			sl->drag_py = event->pos_y;
			//sl->time = get_kernel_time();
			sl->drag_top_item_num = sl->top_item_num;
			break;

		case UI_SL_ST_INERTIA:
			// 慣性スクロールを止める
			break;

		default:
			break;
		}
		break;

	case EVT_TOUCHMOVE:
		if(is_point_in_box(event->pos_x, event->pos_y, &(sl->view_area)) == 0) {
			break;
		}

		switch(sl->status) {
		case UI_SL_ST_STILL:
			//DTPRINTF(0x01, "STATUS ERROR ? status UI_SL_ST_STILL in EVT_TOUCHMOVE\n");
			break;

		case UI_SL_ST_DRAG:
			{
				int scr_y;
				DTPRINTF(0x01, "DRAG X:%3d Y:%3d\n", event->pos_x, event->pos_y);

				if(sl->max_item_disp_count >= sl->item_count) {
					/* 表示可能最大アイテム数より
					 * アイテム数が同じまたは少な
					 * いのでスクロールする必要が
					 * ない */
					break;
				}

				sl->drag_px = event->pos_x;
				sl->drag_py = event->pos_y;
				//sl->time = get_kernel_time();

				scr_y = (event->pos_y - sl->touch_py) / sl->item_height;
				DTPRINTF(0x01, "DRAG_Y:%3d\n", scr_y);

				if(scr_y != 0) {
					sl->flg_scroll = 1;
				}

				sl->top_item_num = sl->drag_top_item_num - scr_y;
				if(sl->top_item_num < 0) {
					sl->top_item_num = 0;
				}
				if(sl->top_item_num > (sl->item_count - sl->max_item_disp_count)) {
					sl->top_item_num = (sl->item_count - sl->max_item_disp_count);
				}
				if(sl->top_item_num != last_tin) {
					draw_ui_selectlist(sl);
				}
			}
			break;

		case UI_SL_ST_INERTIA:
			DTPRINTF(0x01, "INERTIA X:%3d Y:%3d\n", event->pos_x, event->pos_y);
			break;

		default:
			DTPRINTF(0x01, "STATUS ERROR %d\n", sl->status);
			break;
		}
		break;

	case EVT_TOUCHEND:
		{
			int dy, ady;

			//if(sl->status != UI_SL_ST_DRAG) {
			//	DTPRINTF(0x01, "ERROR ? %d\n", sl->status);
			//}

			// 前のドラッグイベントのタッチポイントと移動位置の差が大きい？
			dy = sl->drag_py - event->pos_y;
			if(dy < 0) {
				ady = -dy;
			} else {
				ady = dy;
			}
			if(ady > 8) {
				// 時間内の動きが大きいので慣性スクロール
				DTPRINTF(0x01, "DELTA Y %d\n", dy);
				sl->status = UI_SL_ST_INERTIA;
			} else {
				// タッチ位置の動きが小さいのでアイテム選択完了
				if(is_point_in_box(event->pos_x, event->pos_y, &(sl->view_area)) != 0) {
					int i;
					struct st_box vbox;

					vbox.pos.x = sl->view_area.pos.x;
					vbox.sur.width = sl->view_area.sur.width;
					vbox.sur.height = sl->item_height;

					if(sl->flg_scroll != 0) {
						sl->flg_scroll = 0;
						sl->status = UI_SL_ST_STILL;
						break;
					}

					for(i=0; i<sl->item_disp_count; i++) {
						vbox.pos.y = sl->view_area.pos.y + (sl->item_height * i);

						if(is_point_in_box(event->pos_x, event->pos_y, &vbox) != 0) {
							sl->select_item_num = sl->top_item_num + i;
							DTPRINTF(0x01, "TOUCH SELECT_ITEM_NUM %d\n", sl->select_item_num);
							rt_evt = 1;
							break;
						}
					}
				}
			}

		}
		sl->flg_scroll = 0;
		sl->status = UI_SL_ST_STILL;
		break;

	default:
		return UI_SL_EVT_NULL;
	}

	DTPRINTF(0x01, "SELECT_ITEM_NUM %d\n", sl->select_item_num);
	if(sl->select_item_num != last_sin) {
		calc_layout_selectlist(sl, last_sin);
	}

	if(rt_evt == 0) {
		sl_event->item_num = 0;
		sl_event->what = UI_SL_EVT_NULL;
	} else {
		sl_event->item_num = sl->select_item_num;
		sl_event->what = UI_SL_EVT_ITEM_SEL;
	}

	if(sl->scrollbar != 0) {
		proc_ui_scrollbar(sl->scrollbar, event);
	}

	DTPRINTF(0x01, "UL_SL return %d\n", rt_evt);

	return rt_evt;
}
