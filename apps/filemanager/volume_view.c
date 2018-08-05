/** @file
    @brief	ファイルマネージャボリューム表示

    @date	2017.12.10
    @author	Takashi SHUDO
*/

#include "sysconfig.h"
#include "device/audio_ioctl.h"
#include "str.h"
#include "graphics.h"
#include "graphics_object.h"
#include "font.h"
#include "tprintf.h"
#include "shell.h"
#include "ui_button.h"
#include "ui_seekbar.h"

#include "filemanager.h"
#include "volume_view.h"

static int volume = VOL_DEF;

const struct st_graph_object normal_color_view[] = {
	{ GO_TYPE_FORECOLOR,	{ FM_CTRL_FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ FM_CTRL_BACK_COLOR } },
	{ 0, { 0, 0, 0, 0 }}
};

static struct st_ui_seekbar ui_volume_slider = {
	.view_area = {
		.pos.x = GSC_GRAPHICS_DISPLAY_WIDTH - VOL_WIDTH,
		.pos.y = GSC_GRAPHICS_DISPLAY_HEIGHT - CONTROL_HEIGHT + (CONTROL_HEIGHT - VOL_HEIGHT)/2,
		.sur.width  = VOL_WIDTH,
		.sur.height = VOL_HEIGHT,
	},
	.type = UI_SKB_TYPE_HOLIZONTAL,
	.attr = UI_SKB_ATTR_REALTIME_VALUE_CAHNGE,
	.normal_view = normal_color_view,
	.bar_color = RGB(0,100,100),
	.value = VOL_DEF,
	.max_value = 99,
};

static void set_disp_volume(int vol)
{
	volume = vol;

	set_value_ui_seekbar(&ui_volume_slider, volume);
}

/*
 *
 */

void draw_volume_view(void)
{
	draw_ui_seekbar(&ui_volume_slider);
}

static void set_volume(unsigned short vol)
{
	uchar cmd[32];

	volume = vol;
	tsprintf((char *)cmd, "sound volume %d", volume);
	exec_command(cmd);
}

void init_volume_view(void)
{
	set_volume(VOL_DEF);
}

void volume_proc(struct st_sysevent *event)
{
	int new_volume;

	if(proc_ui_seekbar(&ui_volume_slider, event, &new_volume) == UI_SKB_EVT_CHANGE) {
		set_volume(new_volume);
	}

	switch(event->what) {
	case EVT_SOUND_VOLUME:
		set_disp_volume(event->arg);
		break;
	}
}
