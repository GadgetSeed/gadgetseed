/** @file
    @brief	ファイル/ラジオ切り替え

    @date	2019.01.03
    @auther	Takashi SHUDO
*/

#include "sysconfig.h"
#include "str.h"
#include "graphics.h"
#include "graphics_object.h"
#include "font.h"
#include "tprintf.h"
#include "ui_button.h"

#include "mode_view.h"
#include "musicplay.h"
#include "radio.h"
#include "radio_ctrl_view.h"
#include "radiolist_view.h"
#include "sdmusic.h"
#include "sdmusic_ctrl_view.h"
#include "musicplay_view.h"
#include "musicinfo_view.h"
#include "playtime_slider.h"
#include "radiobuffer_view.h"

#include "soundplay.h"

//#define DEBUGTBITS 0x01
#include "dtprintf.h"

enum_musicplay_mode musicplay_mode = SDCARD;

extern const unsigned int fore_color;
extern const unsigned int back_color;

#define BTN_POS_X_MODE	V_BTN_LEFT
#define BTN_POS_Y_MODE	H_BTN_TOP

const struct st_graph_object obliqueline_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_VERTEX4,	{ GX(27), GY(4), GX(28), GY(5), GX(5), GY(28), GX(4), GY(27) } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object sdcard_obj[] = {
	{ GO_TYPE_VERTEX4,	{ GX(6), GY(4), GX(13), GY(4), GX(13), GY(7), GX(3), GY(7) } },
	{ GO_TYPE_FILL_BOX,	{ GX(3), GY(7), GX(10), GY(9) } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object radio_obj[] = {
	{ GO_TYPE_FILL_BOX,	{ GX(17), GY(20), GX(12), GY(8) } },
	{ GO_TYPE_VERTEX4,	{ GX(25), GY(12), GX(17), GY(20), GX(19), GY(20), GX(26), GY(13) } },
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_REVERSE }, },
	{ GO_TYPE_FILL_CIRCLE,	{ GX(21), GY(24), GX(3) } },
	{ GO_TYPE_FILL_BOX,	{ GX(25), GY(21), GX(3), GY(6) } },
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_NORMAL }, },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object normal_button_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_REVERSE }, },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_NORMAL }, },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object active_button_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_REVERSE }, },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_NORMAL }, },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ 0, { 0, 0, 0, 0 }}
};


const struct st_graph_object radio_btn_obj[] = {
	{ GO_TYPE_OBJECT,	{ 0 }, (void *)normal_button_obj },

	{ GO_TYPE_OBJECT,	{ 0 }, (void *)obliqueline_obj },
	{ GO_TYPE_FORECOLOR,	{ MP_GREEN_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 }, (void *)sdcard_obj },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 }, (void *)radio_obj },

	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object radio_btn_obj_a[] = {
	{ GO_TYPE_OBJECT,	{ 0 }, (void *)active_button_obj },

	{ GO_TYPE_OBJECT,	{ 0 }, (void *)obliqueline_obj },
	{ GO_TYPE_FORECOLOR,	{ MP_GREEN_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 }, (void *)sdcard_obj },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 }, (void *)radio_obj },

	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object sd_btn_obj[] = {
	{ GO_TYPE_OBJECT,	{ 0 }, (void *)normal_button_obj },

	{ GO_TYPE_OBJECT,	{ 0 }, (void *)obliqueline_obj },
	{ GO_TYPE_OBJECT,	{ 0 }, (void *)sdcard_obj },
	{ GO_TYPE_FORECOLOR,	{ MP_GREEN_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 }, (void *)radio_obj },

	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object sd_btn_obj_a[] = {
	{ GO_TYPE_OBJECT,	{ 0 }, (void *)active_button_obj },

	{ GO_TYPE_OBJECT,	{ 0 }, (void *)obliqueline_obj },
	{ GO_TYPE_OBJECT,	{ 0 }, (void *)sdcard_obj },
	{ GO_TYPE_FORECOLOR,	{ MP_GREEN_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 }, (void *)radio_obj },

	{ 0, { 0, 0, 0, 0 }}
};


#define BWS	(BUTTON_WIDTH/16)
#define BHS	(BUTTON_HEIGHT/16)


/* [RADIO] */
static const struct st_ui_button_image ui_view_radio = {
	radio_btn_obj,
	radio_btn_obj_a
};

/* [SD] */
static const struct st_ui_button_image ui_view_sd = {
	sd_btn_obj,
	sd_btn_obj_a
};

static struct st_ui_button ui_btn_mode = {
	0,
	{ {BTN_POS_X_MODE,  BTN_POS_Y_MODE}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	&ui_view_radio,
	UI_BUTTON_ST_NORMAL
};


extern const struct st_graph_object normal_color_view[];

void init_mode_view(void)
{
	switch(musicplay_mode) {
	case SDCARD:
		ui_btn_mode.view = &ui_view_radio;
		break;

	case RADIO:
		ui_btn_mode.view = &ui_view_sd;
		break;
	}
}

void draw_mode_view(void)
{
	set_forecolor(back_color);
	set_forecolor(fore_color);
	draw_ui_button(&ui_btn_mode);
}

void mode_proc(struct st_sysevent *event)
{
	int evt;

	evt = proc_ui_button(&ui_btn_mode, event);

	if(evt == UI_BUTTON_EVT_PUSH) {
		switch(musicplay_mode) {
		case SDCARD:
			tprintf("Change to RADIO MODE\n");
			suspend_sdmusic();
			musicplay_mode = RADIO;
			ui_btn_mode.view = &ui_view_sd;
			draw_mode_view();
			resume_radio();
			draw_radio();
			draw_radio_play_button();
			save_config();
			break;

		case RADIO:
			tprintf("Change to SDCARD MODE\n");
			suspend_radio();
			musicplay_mode = SDCARD;
			ui_btn_mode.view = &ui_view_radio;
			draw_mode_view();
			resume_sdmusic();
			draw_sdmusic();
			{
				int count = soundplay_playbackpos();
				unsigned int time = calc_play_time(minfo, count);

				set_playtime(time/1000);
				set_playtime_slider(count);
			}
			save_config();
			break;

		default:
			break;
		}
	}
}
