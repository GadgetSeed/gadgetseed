/** @file
    @brief	ラジオバッファー

    @date	2019.03.31
    @auther	Takashi SHUDO
*/

#include "sysconfig.h"
#include "ui_progressbar.h"
#include "musicplay_view.h"
#include "tprintf.h"
#include "shell.h"

#include "music_info.h"
#include "playtime_slider.h"
#include "radiobuffer_view.h"

extern const struct st_graph_object normal_color_view[];
extern struct st_music_info *minfo;


static struct st_ui_progressbar ui_radiobuffer_bar = {
	.view_area = {{0, H_BTN_TOP - SCRBAR_WIDTH - (SCRBAR_WIDTH/4)}, {INFO_WIDTH, SCRBAR_WIDTH} },
	.view = normal_color_view,
	.max_value = GSC_PIPEFS_MAX_BUF_COUNT,
	.value = 0,
};

void init_radiobuffer_view(void)
{
	set_value_ui_progressbar(&ui_radiobuffer_bar, 0);
}

void draw_radiobuffer_view(void)
{
	draw_ui_progressbar(&ui_radiobuffer_bar);
}

void set_radiobuffer_size(unsigned int size)
{
	set_value_ui_progressbar(&ui_radiobuffer_bar, size);
}
