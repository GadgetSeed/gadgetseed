/** @file
    @brief	音楽リスト表示

    @date	2017.05.14
    @auther	Takashi SHUDO
*/

#include "sysconfig.h"
#include "str.h"
#include "tprintf.h"
#include "graphics.h"
#include "graphics_object.h"
#include "font.h"
#include "ui_button.h"

#include "musicplay_view.h"
#include "musicplay.h"
#include "list_view.h"
#include "filelist.h"
#include "ui_selectlist.h"

#define LIST_ITEM_GAP	8

extern const struct st_rect info_rect;
extern const unsigned long fore_color;
extern const unsigned long back_color;
extern const unsigned long green_color;

static int select_album_num = 0;

#define ALBUM_TITLE_STR_WIDTH	(INFO_WIDTH - SCRBAR_WIDTH - 4)
static int album_track_count_str_width;
static int music_track_num_str_width;
static int music_play_time_str_width;

static void draw_album_item(struct st_box *box, int item_num, int flg_select)
{
	struct album_item *album = album_list[item_num];
	unsigned char str[MAX_TITLE_LEN + 1];

	if(flg_select != 0) {
		set_forecolor(back_color);
		if(item_num == play_album_num) {
			if(musicplay_status != MUSICPLAY_STAT_STOP) {
				set_backcolor(green_color);
			} else {
				set_backcolor(fore_color);
			}
		} else {
			set_backcolor(fore_color);
		}
	} else {
		if(item_num == play_album_num) {
			if(musicplay_status != MUSICPLAY_STAT_STOP) {
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
	tsprintf((char *)str, " %3d", album->track_count);
	draw_fixed_width_str(box->pos.x, box->pos.y, album->name,
			     ALBUM_TITLE_STR_WIDTH - album_track_count_str_width);
	draw_fixed_width_str(box->pos.x + ALBUM_TITLE_STR_WIDTH - album_track_count_str_width,
			     box->pos.y, str,
			     album_track_count_str_width);
}

static void draw_music_item(struct st_box *box, int item_num, int flg_select)
{
	struct album_item *album = album_list[select_album_num];
	unsigned char str[MAX_TITLE_LEN + 1];
	struct track_filenum *music = &(album->file_num_list[item_num]);
	int file_num = music->file_num;
	struct file_item *music_file = item_list[file_num];

	//tprintf("Music file num %d\n", file_num);

	if(flg_select != 0) {
		set_forecolor(back_color);
		if((select_album_num == play_album_num) && (item_num == play_track_num)) {
			if(musicplay_status != MUSICPLAY_STAT_STOP) {
				set_backcolor(green_color);
			} else {
				set_backcolor(fore_color);
			}
		} else {
			set_backcolor(fore_color);
		}
	} else {
		if((select_album_num == play_album_num) && (item_num == play_track_num)) {
			if(musicplay_status != MUSICPLAY_STAT_STOP) {
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

	tsprintf((char *)str, "%2d ", music_file->track);
	draw_fixed_width_str(box->pos.x, box->pos.y, str,
			     music_track_num_str_width);

	draw_fixed_width_str(box->pos.x + music_track_num_str_width,
			     box->pos.y, music_file->title,
			     ALBUM_TITLE_STR_WIDTH -
			     music_track_num_str_width -
			     music_play_time_str_width);

	tsprintf((char *)str, " %3d:%02d",
		 music_file->time/1000/60,
		 (music_file->time/1000) % 60);
	draw_fixed_width_str(box->pos.x + ALBUM_TITLE_STR_WIDTH -
			     music_play_time_str_width,
			     box->pos.y, str, music_play_time_str_width);
}

static void draw_list_item(struct st_ui_selectlist *selectlist, struct st_box *box, int item_num, int flg_select)
{
	switch(disp_mode) {
	case MODE_ALBUM_SEL:
		draw_album_item(box, item_num, flg_select);
		break;

	case MODE_MUSIC_SEL:
		draw_music_item(box, item_num, flg_select);
		break;

	default:
		break;
	}
}

const struct st_graph_object album_list_view[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ MP_BACK_COLOR } },
	{ 0, { 0, 0, 0, 0 }}
};

extern struct st_ui_selectlist music_select;

const struct st_graph_object sb_normal_view[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ MP_BACK_COLOR } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object sb_selected_view[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	{ 0, { 0, 0, 0, 0 }}
};

struct st_ui_scrollbar ms_scrollbar = {
	.view_area = { {INFO_WIDTH - SCRBAR_WIDTH, 0}, {SCRBAR_WIDTH, INFO_HEIGHT} },
	.normal_view = sb_normal_view,
	.selected_view = sb_selected_view,
	.status = UI_SCB_ST_STILL,
	.selectlist = &music_select,
};

struct st_ui_selectlist music_select = {
	.view_area = { {0, 0}, {INFO_WIDTH - SCRBAR_WIDTH, INFO_HEIGHT} },
	.normal_view = album_list_view,
	.item_height = 24,
	.item_count = 0,
	.top_item_num = 0,
	.select_item_num = 0,
	.item_draw_func = draw_list_item,
	.scrollbar = &ms_scrollbar,
};

void prepare_album_view(void)
{
	set_param_ui_selectlist(&music_select, music_album_count, select_album_num, select_album_num);
}

void prepare_music_view(void)
{
	set_param_ui_selectlist(&music_select,
				album_list[select_album_num]->track_count, 0, 0);
}

void draw_list_view(void)
{
	set_forecolor(back_color);
	draw_fill_rect((struct st_rect *)&info_rect);
	set_forecolor(fore_color);

	draw_ui_selectlist(&music_select);
}

static int last_album_num = 0;
static int last_music_num = 0;

void set_play_album_music_num_list_view(int album_num, int music_num)
{
	switch(disp_mode) {
	case MODE_ALBUM_SEL:
		draw_item_ui_selectlist(&music_select, last_album_num);
		draw_item_ui_selectlist(&music_select, album_num);
		break;

	case MODE_MUSIC_SEL:
		draw_item_ui_selectlist(&music_select, last_music_num);
		draw_item_ui_selectlist(&music_select, music_num);
		break;

	default:
		break;
	}

	last_album_num = album_num;
	last_music_num = music_num;
}

void set_album_num_album_view(int album_num)
{
	set_select_item_num_ui_selectlist(&music_select, album_num);
}

void list_view_proc(struct st_sysevent *event)
{
	struct st_ui_selectlist_event sl_evt;

	if(proc_ui_selectlist(&sl_evt, &music_select, event) != 0) {
		switch(disp_mode) {
		case MODE_ALBUM_SEL:
			tprintf("ALBUM SEL %d\n", sl_evt.item_num);
			if(select_album_num == sl_evt.item_num) {
				play_album_num = sl_evt.item_num;
				tprintf("Album item %d\n", play_album_num);
				play_track_num = 0;
				set_play_file_num();
				if(musicplay_status != MUSICPLAY_STAT_STOP) {
					stop_music_play();
				}
			}
			select_album_num = sl_evt.item_num;
			break;

		case MODE_MUSIC_SEL:
			play_album_num = select_album_num;
			play_track_num = sl_evt.item_num;
			tprintf("Track number %d\n", play_track_num);
			set_play_file_num();
			if(musicplay_status != MUSICPLAY_STAT_STOP) {
				stop_music_play();
			}
			break;

		default:
			break;
		}
	}
}

void init_list_view(void)
{
	set_font_by_name(MPFONT);
	music_select.item_height = font_height() + LIST_ITEM_GAP;
	prepare_ui_selectlist(&music_select);

	album_track_count_str_width = str_width((uchar *)" 000");
	music_track_num_str_width = str_width((uchar *)"00 ");
	music_play_time_str_width = str_width((uchar *)" 000:00");
}
