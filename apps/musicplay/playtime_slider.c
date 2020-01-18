/** @file
    @brief	音楽再生表示(再生時間スライダー)

    @date	2017.05.02
    @auther	Takashi SHUDO
*/

#include "ui_seekbar.h"
#include "musicplay_view.h"
#include "tprintf.h"
#include "shell.h"
#include "music_info.h"
#include "soundplay.h"
#include "settings_view.h"

#include "playtime_slider.h"

const struct st_graph_object inactive_color_view[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_INACTIVE_FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ MP_INACTIVE_BACK_COLOR } },
	{ 0, { 0, 0, 0, 0 }}
};

extern const struct st_graph_object normal_color_view[];
extern struct st_music_info *minfo;
int flg_frame_seek = 0;

extern const struct st_graph_object active_bar_color[];
extern const struct st_graph_object inactive_bar_color[];


static struct st_ui_seekbar ui_playtime_slider = {
	.view_area = {{0, H_BTN_TOP - SCRBAR_WIDTH - (SCRBAR_WIDTH/4)}, {INFO_WIDTH, SCRBAR_WIDTH} },
	.type = UI_SKB_TYPE_HOLIZONTAL,
	.attr = UI_SKB_ATTR_DROP_VALUE_CHANGE,
	.flg_active = 1,
	.normal_view = normal_color_view,
	.inactive_view = inactive_color_view,
	.bar_color = active_bar_color,
	.bar_inactive_color = inactive_bar_color,
	.value = 0,
	.max_value = INFO_WIDTH,
};

void draw_playtime_slider(void)
{
	draw_graph_object(0, 0, normal_color_view);
	set_draw_mode(GRP_DRAWMODE_REVERSE);
	draw_fill_box(&(ui_playtime_slider.view_area));
	set_draw_mode(GRP_DRAWMODE_NORMAL);

	draw_ui_seekbar(&ui_playtime_slider);
}

void activate_playtime_slider(int active)
{
	activate_ui_seekbar(&ui_playtime_slider, active);
}

void set_playtime_slider(unsigned int frame_num)
{
	if(minfo == 0) {
		return;
	}

	if(flg_setting != 0) {
		return;
	}

	if(minfo->sample_count != 0) {
		set_value_ui_seekbar(&ui_playtime_slider,
				    (frame_num * INFO_WIDTH)/minfo->sample_count);
	}
}

static void set_frame(unsigned long frame)
{
	flg_frame_seek = 1;
	soundplay_seek(frame);
}

void playtime_slider_proc(struct st_sysevent *event)
{
	int new_time;

	if(proc_ui_seekbar(&ui_playtime_slider, event, &new_time) == UI_SKB_EVT_TOUCHEND) {
		unsigned int next_frame;
		next_frame = (minfo->sample_count * new_time) / INFO_WIDTH;
		//tprintf("NF:%d %ld\n", new_time, next_frame);

		set_frame(next_frame);
	}
}
