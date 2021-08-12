/** @file
    @brief	ラジオ再生表示制御

    @date	2019.01.13
    @auther	Takashi SHUDO
*/

#include "sysconfig.h"
#include "str.h"
#include "key.h"
#include "graphics.h"
#include "graphics_object.h"
#include "font.h"
#include "tprintf.h"
#include "shell.h"
#include "music_info.h"
#include "ui_button.h"

#include "radio.h"
#include "radio_ctrl_view.h"
#include "volume_view.h"
#include "filelist.h"
#include "musicplay.h"
#include "musicplay_view.h"
#include "radiolist_view.h"
#include "musicinfo_view.h"
#include "mode_view.h"
#include "spectrum_view.h"
#include "settings_view.h"
#include "dialogbox/netset/netset.h"

//#define DEBUGTBITS 0x02
#include "dtprintf.h"


#define BTN_POS_X_ON	(BUTTON_WIDTH + (GSC_GRAPHICS_DISPLAY_WIDTH - (BTN_INTERVAL + BUTTON_WIDTH*3))/2 \
			 - (BUTTON_WIDTH + BTN_INTERVAL + BUTTONW_WIDTH/2))
#define BTN_POS_Y_ON	H_BTN_TOP

#define BTN_POS_X_PREV	(BTN_POS_X_ON + BTN_INTERVAL + BUTTONW_WIDTH)
#define BTN_POS_Y_PREV	H_BTN_TOP

#define BTN_POS_X_NEXT	(BTN_POS_X_PREV + BTN_INTERVAL + BUTTON_WIDTH)
#define BTN_POS_Y_NEXT	H_BTN_TOP

#define BTN_POS_X_LIST	(V_BTN_LEFT - BTN_INTERVAL - BUTTON_WIDTH)
#define BTN_POS_Y_LIST	H_BTN_TOP

const struct st_graph_object gobj_on[] = {
	{ GO_TYPE_SECTOR,	{ GXW(24), GY(16), GY(11), GY(10), 0 } },
	{ GO_TYPE_SECTOR,	{ GXW(24), GY(16), GY(11), GY(10), 1 } },
	{ GO_TYPE_SECTOR,	{ GXW(24), GY(16), GY(11), GY(10), 2 } },
	{ GO_TYPE_SECTOR,	{ GXW(24), GY(16), GY(11), GY(10), 3 } },

	{ GO_TYPE_SECTOR,	{ GXW(20), GY(14), GY(3), GY(2), 1 } },
	{ GO_TYPE_SECTOR,	{ GXW(20), GY(14), GY(3), GY(2), 0 } },
	{ GO_TYPE_FILL_BOX,	{ GX(17), GY(14), GX(1), GY(4) } },
	{ GO_TYPE_FILL_BOX,	{ GX(22), GY(14), GX(1), GY(4) } },
	{ GO_TYPE_SECTOR,	{ GXW(20), GY(18), GY(3), GY(2), 2 } },
	{ GO_TYPE_SECTOR,	{ GXW(20), GY(18), GY(3), GY(2), 3 } },

	{ GO_TYPE_FILL_BOX,	{ GX(25), GY(11), GX(1), GY(10) } },
	{ GO_TYPE_VERTEX4,	{ GX(26), GY(11), GX(29), GY(18), GX(29), GY(21), GX(26), GY(14) } },
	{ GO_TYPE_FILL_BOX,	{ GX(29), GY(11), GX(1), GY(10) } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object on_mark[] = {
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTONW_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTONW_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)gobj_on},
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object on_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)on_mark},
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object on_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)on_mark},
	{ 0, { 0, 0, 0, 0 }}
};


const struct st_graph_object oncon_mark[] = {
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTONW_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTONW_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_ORANGE_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)gobj_on},
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object oncon_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)oncon_mark},
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object oncon_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)oncon_mark},
	{ 0, { 0, 0, 0, 0 }}
};


const struct st_graph_object onnow_mark[] = {
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTONW_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTONW_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_GREEN_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)gobj_on},
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object onnow_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)onnow_mark},
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object onnow_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)onnow_mark},
	{ 0, { 0, 0, 0, 0 }}
};


const struct st_graph_object onfail_mark[] = {
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTONW_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTONW_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_RED_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)gobj_on},
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object onfail_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)onfail_mark},
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object onfail_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)onfail_mark},
	{ 0, { 0, 0, 0, 0 }}
};


const struct st_graph_object gobj_down[] = {
	{ GO_TYPE_TRIANGLE,	{ GX(10), GY(10), GX(22), GY(10), GX(16), GY(22) } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object next_mark[] = {
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)gobj_down},
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object next_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)next_mark},
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object next_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)next_mark},
	{ 0, { 0, 0, 0, 0 }}
};


const struct st_graph_object gobj_up[] = {
	{ GO_TYPE_TRIANGLE,	{ GX(16), GY(10), GX(22), GY(22), GX(10), GY(22) } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object prev_mark[] = {
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)gobj_up},
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object prev_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)prev_mark},
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object prev_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)prev_mark},
	{ 0, { 0, 0, 0, 0 }}
};


const struct st_graph_object radiolist_mark[] = {
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FILL_BOX,	{ BUTTON_WIDTH/4, BUTTON_HEIGHT/4, BUTTON_WIDTH/2, BUTTON_HEIGHT/8 } },
	{ GO_TYPE_FILL_BOX,	{ BUTTON_WIDTH/4, BUTTON_HEIGHT/16*7, BUTTON_WIDTH/2, BUTTON_HEIGHT/8 } },
	{ GO_TYPE_FILL_BOX,	{ BUTTON_WIDTH/4, BUTTON_HEIGHT/8*5, BUTTON_WIDTH/2, BUTTON_HEIGHT/8 } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object radiolist_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)radiolist_mark},
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object radiolist_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)radiolist_mark},
	{ 0, { 0, 0, 0, 0 }}
};

extern const struct st_graph_object gobj_radio[];

const struct st_graph_object radioview_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)gobj_radio},
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object radioview_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)gobj_radio},
	{ 0, { 0, 0, 0, 0 }}
};


#define UO_ID_RADIO_ON	11
#define UO_ID_RADIO_NEXT	12
#define UO_ID_RADIO_PREV	13
#define UO_ID_RADIO_LIST	14

/* ON */
static const struct st_ui_button_image ui_view_on = {
	on_btn_obj,
	on_btn_obj_a
};

static const struct st_ui_button_image ui_view_connecting = {
	oncon_btn_obj,
	oncon_btn_obj_a
};

static const struct st_ui_button_image ui_view_playing = {
	onnow_btn_obj,
	onnow_btn_obj_a
};

static const struct st_ui_button_image ui_view_fail = {
	onfail_btn_obj,
	onfail_btn_obj_a
};

static struct st_ui_button ui_btn_on = {
	UO_ID_RADIO_ON,
	{ {BTN_POS_X_ON,  BTN_POS_Y_ON}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	&ui_view_on,
	UI_BUTTON_ST_NORMAL
};

/* NEXT */
static const struct st_ui_button_image ui_view_next = {
	next_btn_obj,
	next_btn_obj_a
};

static struct st_ui_button ui_btn_next = {
	UO_ID_RADIO_NEXT,
	{ {BTN_POS_X_NEXT,  BTN_POS_Y_NEXT}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	&ui_view_next,
	UI_BUTTON_ST_NORMAL
};

/* PREV */
static const struct st_ui_button_image ui_view_prev = {
	prev_btn_obj,
	prev_btn_obj_a
};

static struct st_ui_button ui_btn_prev = {
	UO_ID_RADIO_PREV,
	{ {BTN_POS_X_PREV,  BTN_POS_Y_PREV}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	&ui_view_prev,
	UI_BUTTON_ST_NORMAL
};

/* [≡] */
static const struct st_ui_button_image ui_view_radiolist = {
	radiolist_btn_obj,
	radiolist_btn_obj_a
};

/* RADIO */
static const struct st_ui_button_image ui_view_radio = {
	radioview_btn_obj,
	radioview_btn_obj_a
};

static struct st_ui_button ui_btn_radiolist = {
	UO_ID_RADIO_LIST,
	{ {BTN_POS_X_LIST,  BTN_POS_Y_LIST}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	&ui_view_radiolist,
	UI_BUTTON_ST_NORMAL
};


/* RADIO操作画面 */
struct st_ui_button *ui_radio_view[] = {
	&ui_btn_on,
	&ui_btn_next,
	&ui_btn_prev,
	&ui_btn_radiolist,
	0
};


/*
 */

void draw_radio_play_button(void)
{
	switch(radioplay_status) {
	case RADIOPLAY_STAT_STOP:
		ui_btn_on.view = &ui_view_on;
		break;

	case RADIOPLAY_STAT_CONNECTING:
		ui_btn_on.view = &ui_view_connecting;
		break;

	case RADIOPLAY_STAT_PLAYING:
		ui_btn_on.view = &ui_view_playing;
		break;

	case RADIOPLAY_STAT_CONNECTFAIL:
		ui_btn_on.view = &ui_view_fail;
		break;

	default:
		break;
	}
		
	if(flg_setting != 0) {
		return;
	}

	draw_ui_button((struct st_ui_button *)&ui_btn_on);
}


/*
 *
 */

void init_radio_ctrl_view(void)
{
	switch(radio_disp_mode) {
	case MODE_RADIO_INFO:
		ui_btn_radiolist.view = &ui_view_radiolist;
		break;

	case MODE_RADIO_SEL:
		ui_btn_radiolist.view = &ui_view_radio;
		break;

	default:
		break;
	}
}

extern const struct st_graph_object ctrl_back[];

void draw_radio_ctrl_view(void)
{
	draw_graph_object(0, 0, ctrl_back);

	draw_ui_button_list(ui_radio_view);
}

void do_on_radio(void)
{
	tprintf("RADIO ON\n");
	select_radio_num = select_radio_num;
	radioplay_status = RADIOPLAY_STAT_CONNECTING;

	set_title_str((uchar *)"Connecting...");
	set_artist_str((uchar *)radio_list[select_radio_num]->url);
	draw_music_info();

	draw_radio_play_button();
	update_radio_list_view();
	on_radio_play();
}

void do_off_radio(void)
{
	tprintf("RADIO OFF\n");
	radioplay_status = RADIOPLAY_STAT_STOP;

	set_title_str((uchar *)"");
	set_artist_str((uchar *)"");
	draw_music_info();

	draw_radio_play_button();
	off_radio_play();
	update_radio_list_view();
}

void do_on_off_radio(void)
{
	if(radioplay_status == RADIOPLAY_STAT_STOP) {
		do_on_radio();
	} else {
		do_off_radio();
	}
}

static void do_next_radio(void)
{
	tprintf("RADIO NEXT\n");
	next_radio_play();
	draw_track_view();
	switch(radio_disp_mode) {
	case MODE_RADIO_INFO:
		draw_musicinfo_view();
		break;

	case MODE_RADIO_SEL:
		draw_radiolist_view();
		break;
	}
	save_config();
}

static void do_prev_radio(void)
{
	tprintf("RADIO PREV\n");
	prev_radio_play();
	draw_track_view();
	switch(radio_disp_mode) {
	case MODE_RADIO_INFO:
		draw_musicinfo_view();
		break;

	case MODE_RADIO_SEL:
		draw_radiolist_view();
		break;
	}
	save_config();
}

void radio_ctrl_proc(struct st_sysevent *event)
{
	struct st_button_event obj_evt;

	if(proc_ui_button_list(&obj_evt, ui_radio_view, event) != 0) {
		switch(obj_evt.id) {
		case UO_ID_RADIO_ON:
			if(obj_evt.what == UI_BUTTON_EVT_PUSH) {
				do_on_off_radio();
				save_config();
			}
			break;

		case UO_ID_RADIO_NEXT:
			if(obj_evt.what == UI_BUTTON_EVT_PUSH) {
				do_next_radio();
			}
			break;

		case UO_ID_RADIO_PREV:
			if(obj_evt.what == UI_BUTTON_EVT_PUSH) {
				do_prev_radio();
			}
			break;

		case UO_ID_RADIO_LIST:
			if(obj_evt.what == UI_BUTTON_EVT_PUSH) {
				switch(radio_disp_mode) {
				case MODE_RADIO_INFO:
					tprintf("RADIO LIST\n");
					radio_disp_mode = MODE_RADIO_SEL;
					ui_btn_radiolist.view = &ui_view_radio;
					draw_ui_button(&ui_btn_radiolist);
					draw_radiolist_view();
					save_config();
					break;

				case MODE_RADIO_SEL:
					tprintf("RADIO INFO\n");
					radio_disp_mode = MODE_RADIO_INFO;
					ui_btn_radiolist.view = &ui_view_radiolist;
					draw_ui_button(&ui_btn_radiolist);
					draw_musicinfo_view();
					draw_spectrum(0);
					save_config();
					break;

				default:
					break;
				}
			}
			break;

		default:
			break;
		}
	}


	switch(event->what) {
	case EVT_KEYDOWN:
		switch(event->arg) {
		case KEY_GB_LEFT:
			do_prev_radio();
			break;

		case KEY_GB_RIGHT:
			do_next_radio();
			break;

		default:
			break;
		}
		break;
	}
}
