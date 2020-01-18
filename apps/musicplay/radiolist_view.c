/** @file
    @brief	ラジオ選曲表示

    @date	2019.01.02
    @auther	Takashi SHUDO
*/

#include "sysconfig.h"
#include "str.h"
#include "tprintf.h"
#include "graphics.h"
#include "graphics_object.h"
#include "font.h"
#include "shell.h"

#include "radio.h"
#include "musicinfo_view.h"
#include "musicplay_view.h"
#include "musicplay.h"
#include "radio_ctrl_view.h"
#include "radiolist_view.h"
#include "filelist.h"
#include "ui_selectlist.h"

#define ALBUM_TITLE_STR_WIDTH	(INFO_WIDTH - SCRBAR_WIDTH - 4)

static void draw_radio_item(struct st_box *box, int item_num, int flg_select)
{
	struct radio_item *radio = radio_list[item_num];

	if(flg_select != 0) {
		set_forecolor(back_color);
		if(item_num == select_radio_num) {
			if(radioplay_status != RADIOPLAY_STAT_STOP) {
				set_backcolor(green_color);
			} else {
				set_backcolor(fore_color);
			}
		} else {
			set_backcolor(fore_color);
		}
	} else {
		if(item_num == select_radio_num) {
			if(radioplay_status != RADIOPLAY_STAT_STOP) {
				set_forecolor(green_color);
			} else {
				set_forecolor(fore_color);
			}
		} else {
			set_forecolor(fore_color);
		}
		set_backcolor(back_color);
	}

	set_font_by_name(MPFONT);
	draw_fixed_width_str(box->pos.x, box->pos.y, (uchar *)radio->broadcaster_name,
			     ALBUM_TITLE_STR_WIDTH);
}


static void draw_list_item(struct st_ui_selectlist *selectlist, struct st_box *box, int item_num, int flg_select)
{
	draw_radio_item(box, item_num, flg_select);
}

extern const struct st_graph_object album_list_view[];
extern const struct st_graph_object sb_normal_view[];
extern const struct st_graph_object sb_select_view[];

extern struct st_ui_selectlist radio_select;

static struct st_ui_scrollbar rd_scrollbar = {
	.view_area = { {INFO_WIDTH - SCRBAR_WIDTH, TEXT_INTERVAL + TOPAREAMARGINE}, {SCRBAR_WIDTH, INFO_HEIGHT} },
	.normal_view = sb_normal_view,
	.select_view = sb_select_view,
	.status = UI_SCB_ST_STILL,
	.selectlist = &radio_select,
};

struct st_ui_selectlist radio_select = {
	.view_area = { {0, TEXT_INTERVAL + TOPAREAMARGINE}, {INFO_WIDTH - SCRBAR_WIDTH, INFO_HEIGHT} },
	.normal_view = album_list_view,
	.item_height = 24,
	.item_count = 0,
	.top_item_num = 0,
	.select_item_num = 0,
	.item_draw_func = draw_list_item,
	.scrollbar = &rd_scrollbar,
};

static void prepare_radiolist_view(void)
{
	set_param_ui_selectlist(&radio_select, radio_count, select_radio_num, select_radio_num);
}

void init_radiolist_view(void)
{
	set_font_by_name(MPFONT);
	radio_select.item_height = font_height() + LIST_ITEM_GAP;
	prepare_ui_selectlist(&radio_select);
	prepare_radiolist_view();
}

static int last_radio_num = 0;

void set_radio_num_list_view(int radio_num)
{
	switch(radio_disp_mode) {
	case MODE_RADIO_SEL:
		set_select_item_num_ui_selectlist(&radio_select, radio_num);
		draw_item_ui_selectlist(&radio_select, last_radio_num);
		draw_item_ui_selectlist(&radio_select, radio_num);
		break;

	default:
		break;
	}

	last_radio_num = radio_num;
}

void draw_radiolist_view(void)
{
	set_forecolor(back_color);
	draw_fill_rect((struct st_rect *)&info_rect);
	set_forecolor(fore_color);

	draw_ui_selectlist(&radio_select);
}

void radiolist_view_proc(struct st_sysevent *event)
{
	struct st_ui_selectlist_event sl_evt;

	if(proc_ui_selectlist(&sl_evt, &radio_select, event) != 0) {
		if(select_radio_num != sl_evt.item_num) {
			tprintf("RADIO SEL %d\n", sl_evt.item_num);
			select_radio_num = sl_evt.item_num;
			tprintf("Radio item %d\n", select_radio_num);
			set_artist_str((uchar *)radio_list[select_radio_num]->url);
//			draw_track_view();
			if(radioplay_status != RADIOPLAY_STAT_STOP) {
				off_radio_play();
//				radioplay_status = RADIOPLAY_STAT_CONNECTING;
//				draw_radio_play_button();
				on_radio_play();
				update_radio_list_view();
			}
			save_config();
		}
	}
}
