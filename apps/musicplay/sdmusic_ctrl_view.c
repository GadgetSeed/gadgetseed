/** @file
    @brief	SD音楽再生制御

    @date	2017.05.02
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

#include "sdmusic.h"
#include "sdmusic_ctrl_view.h"
#include "musicplay.h"
#include "musicplay_view.h"
#include "list_view.h"
#include "musicinfo_view.h"
#include "spectrum_view.h"
#include "settings_view.h"

//#define DEBUGTBITS 0x02
#include "dtprintf.h"


#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO
#define BTN_POS_X_FR	(BUTTON_WIDTH + (GSC_GRAPHICS_DISPLAY_WIDTH - (BTN_INTERVAL + BUTTON_WIDTH*3))/2 \
			 - (BUTTON_WIDTH + BTN_INTERVAL + BUTTONW_WIDTH/2))
#else
#define BTN_POS_X_FR	(BUTTON_WIDTH + (GSC_GRAPHICS_DISPLAY_WIDTH - (BUTTON_WIDTH*2))/2 \
			 - (BUTTON_WIDTH + BTN_INTERVAL + BUTTONW_WIDTH/2))
#endif
#define BTN_POS_Y_FR	H_BTN_TOP

#define BTN_POS_X_PLAY	(BTN_POS_X_FR + BUTTON_WIDTH + BTN_INTERVAL)
#define BTN_POS_Y_PLAY	H_BTN_TOP

#define BTN_POS_X_FF	(BTN_POS_X_PLAY + BUTTONW_WIDTH + BTN_INTERVAL)
#define BTN_POS_Y_FF	H_BTN_TOP

#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO
#define BTN_POS_X_LIST	(V_BTN_LEFT - BTN_INTERVAL - BUTTON_WIDTH)
#else
#define BTN_POS_X_LIST	V_BTN_LEFT
#endif
#define BTN_POS_Y_LIST	H_BTN_TOP


const struct st_graph_object gobj_button[] = {
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT, BUTTON_WIDTH/8 } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object gobj_play[] = {
	{ GO_TYPE_TRIANGLE,	{ GXW(6), GY(10), GXW(16), GY(16), GXW(6), GY(22) } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object gobj_pause[] = {
	{ GO_TYPE_VERTEX4,	{ GXW(28), GY(7), GXW(30), GY(8), GXW(20), GY(24), GXW(18), GY(23) } },
	{ GO_TYPE_FILL_BOX,	{ GXW(32), GY(10), GXW(4), GY(12) } },
	{ GO_TYPE_FILL_BOX,	{ GXW(40), GY(10), GXW(4), GY(12) } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object play_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTONW_WIDTH, BUTTON_HEIGHT,  BUTTON_HEIGHT/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTONW_WIDTH, BUTTON_HEIGHT,  BUTTON_HEIGHT/8 } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)gobj_play},
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)gobj_pause},
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object play_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTONW_WIDTH, BUTTON_HEIGHT,  BUTTON_HEIGHT/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTONW_WIDTH, BUTTON_HEIGHT,  BUTTON_HEIGHT/8 } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)gobj_play},
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)gobj_pause},
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object playing_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTONW_WIDTH, BUTTON_HEIGHT,  BUTTON_HEIGHT/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTONW_WIDTH, BUTTON_HEIGHT,  BUTTON_HEIGHT/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_GREEN_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)gobj_play},
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)gobj_pause},
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object playing_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTONW_WIDTH, BUTTON_HEIGHT,  BUTTON_HEIGHT/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTONW_WIDTH, BUTTON_HEIGHT,  BUTTON_HEIGHT/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_GREEN_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)gobj_play},
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)gobj_pause},
	{ 0, { 0, 0, 0, 0 }}
};


const struct st_graph_object gobj_ff[] = {
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)gobj_button},
	{ GO_TYPE_TRIANGLE,	{ GX(4), GY(10), GX(14), GY(16), GX(4), GY(22) } },
	{ GO_TYPE_TRIANGLE,	{ GX(14), GY(10), GX(24), GY(16), GX(14), GY(22) } },
	{ GO_TYPE_FILL_BOX,	{ GX(24), GY(10), GX(4), GY(12) } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object ff_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_HEIGHT/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)gobj_ff},
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object ff_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_HEIGHT/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)gobj_ff},
	{ 0, { 0, 0, 0, 0 }}
};


const struct st_graph_object gobj_fr[] = {
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)gobj_button},
	{ GO_TYPE_FILL_BOX,	{ GX(4), GY(10), GX(4), GY(12) } },
	{ GO_TYPE_TRIANGLE,	{ GX(18), GY(10), GX(18), GY(22), GX(8), GY(16) } },
	{ GO_TYPE_TRIANGLE,	{ GX(28), GY(10), GX(28), GY(22), GX(18), GY(16) } },
	{ 0, { 0, 0, 0, 0 }}
};


const struct st_graph_object fr_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)gobj_fr},
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object fr_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)gobj_fr},
	{ 0, { 0, 0, 0, 0 }}
};


const struct st_graph_object gobj_list[] = {
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)gobj_button},
	{ GO_TYPE_FILL_BOX,	{ GX(5), GY(8), GX(22), GY(4) } },
	{ GO_TYPE_FILL_BOX,	{ GX(5), GY(14), GX(22), GY(4) } },
	{ GO_TYPE_FILL_BOX,	{ GX(5), GY(20), GX(22), GY(4) } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object gobj_cdlist[] = {
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)gobj_button},
	{ GO_TYPE_FILL_CIRCLE,	{ GX(7), GY(10), GX(2) } },
	{ GO_TYPE_FILL_BOX,	{ GX(11), GY(8), GX(16), GY(4) } },
	{ GO_TYPE_FILL_CIRCLE,	{ GX(7), GY(16), GX(2) } },
	{ GO_TYPE_FILL_BOX,	{ GX(11), GY(14), GX(16), GY(4) } },
	{ GO_TYPE_FILL_CIRCLE,	{ GX(7), GY(22), GX(2) } },
	{ GO_TYPE_FILL_BOX,	{ GX(11), GY(20), GX(16), GY(4) } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object gobj_1cd[] = {
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)gobj_button},
	{ GO_TYPE_FILL_CIRCLE,	{ GX(16), GY(16), GX(11) } },
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_REVERSE }, },
	{ GO_TYPE_FILL_CIRCLE,	{ GX(16), GY(16), GX(4) } },
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_NORMAL }, },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object list_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)gobj_list},
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object list_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)gobj_list},
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object cdlist_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)gobj_cdlist},
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object cdlist_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)gobj_cdlist},
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object cd_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)gobj_1cd},
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object cd_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)gobj_1cd},
	{ 0, { 0, 0, 0, 0 }}
};


#define UO_ID_PLAYPAUSE	4
#define UO_ID_FF	5
#define UO_ID_FR	6
#define UO_ID_LIST	7

/* [▶] */
static const struct st_ui_button_image ui_view_play = {
	play_btn_obj,
	play_btn_obj_a
};

static const struct st_ui_button_image ui_view_playing = {
	playing_btn_obj,
	playing_btn_obj_a
};

static struct st_ui_button ui_btn_play = {
	UO_ID_PLAYPAUSE,
	{ {BTN_POS_X_PLAY,  BTN_POS_Y_PLAY}, {BUTTONW_WIDTH, BUTTON_HEIGHT} },
	&ui_view_play,
	UI_BUTTON_ST_NORMAL
};

/* [▶▶|] */
static const struct st_ui_button_image ui_view_ff = {
	ff_btn_obj,
	ff_btn_obj_a
};

static struct st_ui_button ui_btn_ff = {
	UO_ID_FF,
	{ {BTN_POS_X_FF,  BTN_POS_Y_FF}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	&ui_view_ff,
	UI_BUTTON_ST_NORMAL
};

/* [|◀◀] */
static const struct st_ui_button_image ui_view_fr = {
	fr_btn_obj,
	fr_btn_obj_a
};

static struct st_ui_button ui_btn_fr = {
	UO_ID_FR,
	{ {BTN_POS_X_FR,  BTN_POS_Y_FR}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	&ui_view_fr,
	UI_BUTTON_ST_NORMAL
};

/* [≡] */
const struct st_ui_button_image ui_view_list = {
	list_btn_obj,
	list_btn_obj_a
};

/* [≡] */
const struct st_ui_button_image ui_view_cdlist = {
	cdlist_btn_obj,
	cdlist_btn_obj_a
};

/* ◎ */
const struct st_ui_button_image ui_view_cd = {
	cd_btn_obj,
	cd_btn_obj_a
};

struct st_ui_button ui_btn_list = {
	UO_ID_LIST,
	{ {BTN_POS_X_LIST,  BTN_POS_Y_LIST}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	&ui_view_cdlist,
	UI_BUTTON_ST_NORMAL
};


/* PLAY画面 */
struct st_ui_button *ui_play_view[] = {
	&ui_btn_play,
	&ui_btn_ff,
	&ui_btn_fr,
	&ui_btn_list,
	0
};


/*
 */

void set_sdmusic_button_playing(void)
{
	ui_btn_play.view = &ui_view_playing;

	if(flg_setting != 0) {
		return;
	}

	draw_ui_button((struct st_ui_button *)&ui_btn_play);
}

void set_sdmusic_button_stoping(void)
{
	ui_btn_play.view = &ui_view_play;

	if(flg_setting != 0) {
		return;
	}

	draw_ui_button((struct st_ui_button *)&ui_btn_play);
}


/*
 *
 */

void init_sdmusic_ctrl_view(void)
{
	struct st_box fbox = { {0, 0}, {GSC_GRAPHICS_DISPLAY_WIDTH, GSC_GRAPHICS_DISPLAY_HEIGHT} };

	set_forecolor(back_color);
	set_backcolor(back_color);
	draw_fill_box(&fbox);
}

const struct st_graph_object ctrl_back[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_FILL_BOX,	{ BTN_POS_X_FR, BTN_POS_Y_FR,
				  BUTTON_WIDTH+BTN_INTERVAL+BUTTONW_WIDTH+BTN_INTERVAL+BUTTON_WIDTH, BUTTON_HEIGHT } },
	{ 0, { 0, 0, 0, 0 }}
};


void draw_sdmusic_ctrl_view(void)
{
	draw_graph_object(0, 0, ctrl_back);

	draw_ui_button_list(ui_play_view);
}

void do_sdmusic_play(void)
{
	tprintf("PLAY\n");
	sdmusicplay_status = SDMUSICPLAY_STAT_PLAY;
	start_sdmusic_play();
}

void do_sdmusic_pause(void)
{
	tprintf("STOP\n");
	sdmusicplay_status = SDMUSICPLAY_STAT_STOP;
	set_sdmusic_button_stoping();
	pause_sdmusic_play();
}

void do_play_pause(void)
{
	switch(sdmusicplay_status) {
	case SDMUSICPLAY_STAT_OPENED:
	case SDMUSICPLAY_STAT_STOP:
		do_sdmusic_play();
		break;

	case SDMUSICPLAY_STAT_PLAY:
		do_sdmusic_pause();
		break;

	default:
		break;
	}
}

static void do_ff_sdmusic(void)
{
	tprintf("FF\n");
	stop_sdmusic_play();
	ff_sdmusic_play();
	open_now_sdmusic();
	//save_config();
}

static void do_fr_sdmusic(void)
{
	tprintf("FR\n");
	stop_sdmusic_play();
	fr_sdmusic_play();
	open_now_sdmusic();
	//save_config();
}

void sdmusic_ctrl_proc(struct st_sysevent *event)
{
	struct st_button_event obj_evt;

	if(proc_ui_button_list(&obj_evt, ui_play_view, event) != 0) {
		switch(obj_evt.id) {
		case UO_ID_PLAYPAUSE:
			if(obj_evt.what == UI_BUTTON_EVT_PUSH) {
				do_play_pause();
				save_config();
			}
			break;

		case UO_ID_FF:
			if(obj_evt.what == UI_BUTTON_EVT_PUSH) {
				do_ff_sdmusic();
			}
			break;

		case UO_ID_FR:
			if(obj_evt.what == UI_BUTTON_EVT_PUSH) {
				do_fr_sdmusic();
			}
			break;

		case UO_ID_LIST:
			if(obj_evt.what == UI_BUTTON_EVT_PUSH) {
				switch(sd_disp_mode) {
				case MODE_SD_INFO:
					tprintf("ALBUM LIST\n");
					sd_disp_mode = MODE_ALBUM_SEL;
					ui_btn_list.view = &ui_view_list;
					draw_ui_button((struct st_ui_button *)&ui_btn_list);
					prepare_album_view();
					draw_list_view();
					break;

				case MODE_ALBUM_SEL:
					tprintf("MUSIC LIST\n");
					sd_disp_mode = MODE_MUSIC_SEL;
					ui_btn_list.view = &ui_view_cd;
					draw_ui_button((struct st_ui_button *)&ui_btn_list);
					prepare_music_view();
					draw_list_view();
					break;

				case MODE_MUSIC_SEL:
					tprintf("MUSIC PLAY INFO\n");
					sd_disp_mode = MODE_SD_INFO;
					ui_btn_list.view = &ui_view_cdlist;
					draw_ui_button((struct st_ui_button *)&ui_btn_list);
					draw_musicinfo_view();
					draw_spectrum(0);
					break;

				default:
					break;
				}
				save_config();
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
			do_fr_sdmusic();
			break;

		case KEY_GB_RIGHT:
			do_ff_sdmusic();
			break;

		default:
			break;
		}
		break;
	}
}
