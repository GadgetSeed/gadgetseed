/** @file
    @brief	musicplay/internetradio各種構成設定

    @date	2019.12.21
    @auther	Takashi SHUDO
*/

#include "sysconfig.h"
#include "str.h"
#include "graphics.h"
#include "graphics_object.h"
#include "font.h"
#include "tprintf.h"
#include "ui_button.h"

#include "soundplay.h"
#include "sdmusic.h"
#include "musicplay.h"
#include "mode_view.h"
#include "musicinfo_view.h"
#include "volume_view.h"
#include "radio.h"
#include "filelist.h"

#include "musicplay_ui_style.h"
#include "dialogbox/timeset/timeset.h"
#include "dialogbox/netset/netset.h"

#include "settings_view.h"

//#define DEBUGTBITS 0x01
#include "dtprintf.h"

#if GSC_GRAPHICS_DISPLAY_WIDTH >= 800
#define SET_BUTTON_X		96
#define SET_BUTTON_Y		64
#define SET_BUTTON_VINT		16
#define SET_BUTTON_WIDTH	(GSC_GRAPHICS_DISPLAY_WIDTH - (SET_BUTTON_X * 2))
#define SET_BUTTON_HEIGHT	48
#else
#define SET_BUTTON_X		64
#define SET_BUTTON_Y		16
#define SET_BUTTON_VINT		8
#define SET_BUTTON_WIDTH	(GSC_GRAPHICS_DISPLAY_WIDTH - (SET_BUTTON_X * 2))
#define SET_BUTTON_HEIGHT	32
#endif

#define BTN_ID_INITSET		0
#define BTN_ID_SEARCHMUSIC	1
#define BTN_ID_TIMESET		2
#define BTN_ID_NETWORKSET	3
#define BTN_ID_NTPSET		4
#define BTN_ID_CANCEL		5

int flg_setting = 0;

static struct st_ui_button uibt_init = {
	.id	= BTN_ID_INITSET,
	.view_area = {
		.pos.x	= SET_BUTTON_X,
		.pos.y	= SET_BUTTON_Y + (SET_BUTTON_VINT + SET_BUTTON_HEIGHT) * 0,
		.sur.width	= SET_BUTTON_WIDTH,
		.sur.height	= SET_BUTTON_HEIGHT,
	},
	.name	= "Initialize Settings",
	.status	= UI_BUTTON_ST_NORMAL,
};

static struct st_ui_button uibt_search = {
	.id	= BTN_ID_SEARCHMUSIC,
	.view_area = {
		.pos.x	= SET_BUTTON_X,
		.pos.y	= SET_BUTTON_Y + (SET_BUTTON_VINT + SET_BUTTON_HEIGHT) * 1,
		.sur.width	= SET_BUTTON_WIDTH,
		.sur.height	= SET_BUTTON_HEIGHT,
	},
	.name	= "Search Music Files",
	.status	= UI_BUTTON_ST_NORMAL,
};

static struct st_ui_button uibt_clock = {
	.id	= BTN_ID_TIMESET,
	.view_area = {
		.pos.x	= SET_BUTTON_X,
		.pos.y	= SET_BUTTON_Y + (SET_BUTTON_VINT + SET_BUTTON_HEIGHT) * 2,
		.sur.width	= SET_BUTTON_WIDTH,
		.sur.height	= SET_BUTTON_HEIGHT,
	},
	.name	= "Clock Setting",
	.status	= UI_BUTTON_ST_NORMAL,
};

#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO
static struct st_ui_button uibt_network = {
	.id	= BTN_ID_NETWORKSET,
	.view_area = {
		.pos.x	= SET_BUTTON_X,
		.pos.y	= SET_BUTTON_Y + (SET_BUTTON_VINT + SET_BUTTON_HEIGHT) * 3,
		.sur.width	= SET_BUTTON_WIDTH,
		.sur.height	= SET_BUTTON_HEIGHT,
	},
	.name	= "Network Setting",
	.status	= UI_BUTTON_ST_NORMAL,
};

#if 0 //!!!
static struct st_ui_button uibt_ntp = {
	.id	= BTN_ID_NTPSET,
	.view_area = {
		.pos.x	= SET_BUTTON_X,
		.pos.y	= SET_BUTTON_Y + (SET_BUTTON_VINT + SET_BUTTON_HEIGHT) * 4,
		.sur.width	= SET_BUTTON_WIDTH,
		.sur.height	= SET_BUTTON_HEIGHT,
	},
	.name	= "NTP Setting",
	.status	= UI_BUTTON_ST_NORMAL,
};
#endif
#endif

static struct st_ui_button uibt_cancel = {
	.id	= BTN_ID_CANCEL,
	.view_area = {
		.pos.x	= SET_BUTTON_X,
		.pos.y	= SET_BUTTON_Y + (SET_BUTTON_VINT + SET_BUTTON_HEIGHT) * 5,
		.sur.width	= SET_BUTTON_WIDTH,
		.sur.height	= SET_BUTTON_HEIGHT,
	},
	.name	= "Cancel",
	.status	= UI_BUTTON_ST_NORMAL,
};

static struct st_ui_button *uibt_setting_view[] = {
	&uibt_init,
	&uibt_search,
	&uibt_clock,
#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO
	&uibt_network,
//!!!	&uibt_ntp,
#endif
	&uibt_cancel,
	0
};


void init_settings_view(void)
{
}

static struct st_rect setting_view_area = {
	0, 0, GSC_GRAPHICS_DISPLAY_WIDTH, GSC_GRAPHICS_DISPLAY_HEIGHT
};

void draw_settings_view(void)
{
	set_forecolor(UI_BACK_COLOR);
	draw_fill_rect(&setting_view_area);

	draw_ui_button_list(uibt_setting_view);
}

int settings_proc(struct st_sysevent *event)
{
	struct st_button_event obj_evt;
	int rt = 0;

	if(proc_ui_button_list(&obj_evt, uibt_setting_view, event) != 0) {
		if(obj_evt.what == UI_BUTTON_EVT_PULL) {
			switch(obj_evt.id) {
			case BTN_ID_INITSET:
			{
				reset_musicplay();
				init_settings_volume();
				init_settings_playmode();
				init_settings_playmusic();
				open_now_sdmusic();
				setup_musicinfo();
				set_volume_view(VOL_DEF, 0);
#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO
				musicplay_mode = SDCARD;
#endif
				save_config();
				tprintf("All settings initialized\n");
				return 1;
			}
			break;

			case BTN_ID_SEARCHMUSIC:
			{
				reset_musicplay();
				init_settings_playmusic();
				dispose_filelist();
				create_filelist();
				return 1;
			}
			break;

			case BTN_ID_TIMESET:
			{
				struct st_systime now_time;
				struct st_datetime now_datetime;

				get_systime(&now_time);
				systime_to_datetime(&now_datetime, &now_time);

				prepare_timeset(&now_datetime);

				rt = open_timeset_dialog(sound_proc);
			}
			break;

#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO
			case BTN_ID_NETWORKSET:
				if(obj_evt.what == UI_BUTTON_EVT_PULL) {
					int rt = 0;

					rt = open_netset_dialog(sound_proc);
					if(rt == 1) {
						// Nework setting
						tprintf("Network new setting\n");
						if(musicplay_mode == RADIO) {
							off_radio_play();
							update_radio_list_view();
						}
						save_config();
					} else {
						tprintf("Network setting canceld\n");
					}
					return 1;
				}
			break;

			case BTN_ID_NTPSET:
				break;
#endif

			case BTN_ID_CANCEL:
				return 1;
				break;

			default:
				break;
			}
		}
	}

	return rt;
}

int open_settings_dialog(void)
{
	draw_settings_view();

	flg_setting = 1;

	while(1) {
		struct st_sysevent event;
		int rt = 0;

		get_event(&event, 50);
		rt = settings_proc(&event);
		if(rt != 0) {
			flg_setting = 0;
			return rt;
		}
		sound_proc(&event);
	}

	return 0;
}
