/** @file
    @brief	音楽情報表示

    @date	2017.05.02
    @auther	Takashi SHUDO
*/

#include "sysconfig.h"
#include "str.h"
#include "graphics.h"
#include "graphics_object.h"
#include "font.h"
#include "tprintf.h"
#include "shell.h"
#include "music_info.h"
#include "ui_button.h"
#include "ui_statictext.h"

#include "musicplay.h"
#include "filelist.h"
#include "musicplay_view.h"
#include "musicinfo_view.h"
#include "sdmusic.h"
#include "sdmusic_ctrl_view.h"
#include "list_view.h"
#include "settings_view.h"
#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO
#include "mode_view.h"
#include "radio.h"
#endif

//#define DEBUGTBITS 0x02
#include "dtprintf.h"

#define BTN_POS_X_SHUFFLE	(INFO_WIDTH - (PLAYMODEBTN_WIDTH * 2) - BTN_INTERVAL)
#define BTN_POS_Y_SHUFFLE	(TOPAREAMARGINE + TEXT_INTERVAL)

#define BTN_POS_X_REPEAT	(INFO_WIDTH - (PLAYMODEBTN_WIDTH))
#define BTN_POS_Y_REPEAT	(TOPAREAMARGINE + TEXT_INTERVAL)


const struct st_graph_object gobj_radio[] = {
	{ GO_TYPE_FILL_BOX,	{ GX(4), GY(12), GX(24), GY(16) } },
	{ GO_TYPE_VERTEX4,	{ GX(14), GY(2), GX(15), GY(3), GX(6), GY(12), GX(4), GY(12) } },
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_REVERSE }, },
	{ GO_TYPE_FILL_CIRCLE,	{ GX(12), GY(20), GX(6) } },
	{ GO_TYPE_FILL_BOX,	{ GX(20), GY(14), GX(6), GY(12) } },
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_NORMAL }, },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object gobj_radio_icon[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0, 0, 3, 2 },  (void *)gobj_radio },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object gobj_cd[] = {
	{ GO_TYPE_FILL_CIRCLE,	{ GX(16), GY(16), GX(11) } },
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_REVERSE }, },
	{ GO_TYPE_FILL_CIRCLE,	{ GX(16), GY(16), GX(4) } },
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_NORMAL }, },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object gobj_cd_icon[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_FILL_BOX,	{ 0, 0, ART_WIDTH, ART_HEIGHT } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0, 0, 3, 2 },  (void *)gobj_cd },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object gobj_shuffle[] = {
	{ GO_TYPE_VERTEX4,	{ GXW(8), GY(7), GXW(17), GY(7), GXW(16), GY(11), GXW(8), GY(11) } },
	{ GO_TYPE_VERTEX4,	{ GXW(17), GY(7), GXW(31), GY(21), GXW(30), GY(25), GXW(16), GY(11) } },
	{ GO_TYPE_VERTEX4,	{ GXW(31), GY(21), GXW(38), GY(21), GXW(38), GY(25), GXW(30), GY(25) } },
	{ GO_TYPE_TRIANGLE,	{ GXW(38), GY(19), GXW(42), GY(23), GXW(38), GY(27) } },
	{ GO_TYPE_VERTEX4,	{ GXW(8), GY(21), GXW(16), GY(21), GXW(17), GY(25), GXW(8), GY(25) } },
	{ GO_TYPE_VERTEX4,	{ GXW(16), GY(21), GXW(20), GY(17), GXW(23), GY(19), GXW(17), GY(25) } },
	{ GO_TYPE_VERTEX4,	{ GXW(24), GY(13), GXW(30), GY(7), GXW(31), GY(11), GXW(27), GY(15) } },
	{ GO_TYPE_VERTEX4,	{ GXW(30), GY(7), GXW(38), GY(7), GXW(38), GY(11), GXW(31), GY(11) } },
	{ GO_TYPE_TRIANGLE,	{ GXW(38), GY(5), GXW(42), GY(9), GXW(38), GY(13) } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object playmode_btn_obj[] = {
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, PLAYMODEBTN_WIDTH, PLAYMODEBTN_HEIGHT,  PLAYMODEBTN_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,		{ MP_FORE_COLOR } },
	{ GO_TYPE_ROUND_BOX,		{ 0, 0, PLAYMODEBTN_WIDTH, PLAYMODEBTN_HEIGHT,  PLAYMODEBTN_WIDTH/8 } },
	{ 0, { 0, 0, 0, 0 }}
};

extern const struct st_bitmap shuffle_icon;

const struct st_graph_object shuffle_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0, 0 },  (void *)playmode_btn_obj},

	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
#ifdef ICONGOBJ
	{ GO_TYPE_OBJECT,	{ ICONGOBJ, 0, 1, 2 },  (void *)gobj_shuffle},
#else
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_FOREONLY } },
	{ GO_TYPE_BITMAP,	{ 0, 0 },  (void *)&shuffle_icon },
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_NORMAL } },
#endif
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object shuffle_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0, 0 },  (void *)playmode_btn_obj},

	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
#ifdef ICONGOBJ
	{ GO_TYPE_OBJECT,	{ ICONGOBJ, 0, 1, 2 },  (void *)gobj_shuffle},
#else
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_FOREONLY } },
	{ GO_TYPE_BITMAP,	{ 0, 0 },  (void *)&shuffle_icon },
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_NORMAL } },
#endif
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object shuffling_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0, 0 },  (void *)playmode_btn_obj},

	{ GO_TYPE_FORECOLOR,	{ MP_BLUE_COLOR } },
#ifdef ICONGOBJ
	{ GO_TYPE_OBJECT,	{ ICONGOBJ, 0, 1, 2 },  (void *)gobj_shuffle},
#else
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_FOREONLY } },
	{ GO_TYPE_BITMAP,	{ 0, 0 },  (void *)&shuffle_icon },
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_NORMAL } },
#endif
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object shuffling_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0, 0 },  (void *)playmode_btn_obj},

	{ GO_TYPE_FORECOLOR,	{ MP_BLUE_COLOR } },
#ifdef ICONGOBJ
	{ GO_TYPE_OBJECT,	{ ICONGOBJ, 0, 1, 2 },  (void *)gobj_shuffle},
#else
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_FOREONLY } },
	{ GO_TYPE_BITMAP,	{ 0, 0 },  (void *)&shuffle_icon },
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_NORMAL } },
#endif
	{ 0, { 0, 0, 0, 0 }}
};


const struct st_graph_object gobj_repeat[] = {
	{ GO_TYPE_FILL_BOX,	{ GX(8), GY(13), GX(4), GY(3) } },
	{ GO_TYPE_SECTOR,	{ GX(14), GY(13), GX(6), GY(2), 1 } },
	{ GO_TYPE_FILL_BOX,	{ GX(14), GY(7), GX(20), GY(4) } },
	{ GO_TYPE_TRIANGLE,	{ GX(34), GY(5), GX(38), GY(9), GX(34), GY(13) } },
	{ GO_TYPE_TRIANGLE,	{ GX(10), GY(23), GX(14), GY(19), GX(14), GY(27) } },
	{ GO_TYPE_FILL_BOX,	{ GX(14), GY(21), GX(20), GY(4) } },
	{ GO_TYPE_SECTOR,	{ GX(34), GY(19), GX(6), GY(2), 3 } },
	{ GO_TYPE_FILL_BOX,	{ GX(36), GY(16), GX(4), GY(3) } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object gobj_repeat_album[] = {
	{ GO_TYPE_VERTEX4,	{ GX(24), GY(12), GX(26), GY(12), GX(28), GY(14), GX(22), GY(14) } },
	{ GO_TYPE_FILL_BOX,	{ GX(22), GY(14), GX(2), GY(6) } },
	{ GO_TYPE_FILL_BOX,	{ GX(26), GY(14), GX(2), GY(6) } },
	{ GO_TYPE_FILL_BOX,	{ GX(24), GY(269), GX(2), GY(1) } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object gobj_repeat_one[] = {
	{ GO_TYPE_TRIANGLE,	{ GX(24), GY(12), GX(24), GY(14), GX(22), GY(14) } },
	{ GO_TYPE_FILL_BOX,	{ GX(24), GY(12), GX(2), GY(7) } },
	{ GO_TYPE_FILL_BOX,	{ GX(22), GY(19), GX(6), GY(1) } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object repeat_off_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0, 0 },  (void *)playmode_btn_obj},

	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_OBJECT,	{ ICONGOBJ, 0, 1, 2 },  (void *)gobj_repeat},
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object repeat_off_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0, 0 },  (void *)playmode_btn_obj},

	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_OBJECT,	{ ICONGOBJ, 0, 1, 2 },  (void *)gobj_repeat},
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object repeat_all_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0, 0 },  (void *)playmode_btn_obj},

	{ GO_TYPE_FORECOLOR,	{ MP_BLUE_COLOR } },
	{ GO_TYPE_OBJECT,	{ ICONGOBJ, 0, 1, 2 },  (void *)gobj_repeat},
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object repeat_all_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0, 0 },  (void *)playmode_btn_obj},

	{ GO_TYPE_FORECOLOR,	{ MP_BLUE_COLOR } },
	{ GO_TYPE_OBJECT,	{ ICONGOBJ, 0, 1, 2 },  (void *)gobj_repeat},
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object repeat_1album_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0, 0 },  (void *)playmode_btn_obj},

	{ GO_TYPE_FORECOLOR,	{ MP_BLUE_COLOR } },
	{ GO_TYPE_OBJECT,	{ ICONGOBJ, 0, 1, 2 },  (void *)gobj_repeat},
	{ GO_TYPE_OBJECT,	{ ICONGOBJ, 0, 1, 2 },  (void *)gobj_repeat_album},
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object repeat_1album_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0, 0 },  (void *)playmode_btn_obj},

	{ GO_TYPE_FORECOLOR,	{ MP_BLUE_COLOR } },
	{ GO_TYPE_OBJECT,	{ ICONGOBJ, 0, 1, 2 },  (void *)gobj_repeat},
	{ GO_TYPE_OBJECT,	{ ICONGOBJ, 0, 1, 2 },  (void *)gobj_repeat_album},
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object repeat_1music_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0, 0 },  (void *)playmode_btn_obj},

	{ GO_TYPE_FORECOLOR,	{ MP_BLUE_COLOR } },
	{ GO_TYPE_OBJECT,	{ ICONGOBJ, 0, 1, 2 },  (void *)gobj_repeat},
	{ GO_TYPE_OBJECT,	{ ICONGOBJ, 0, 1, 2 },  (void *)gobj_repeat_one},
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object repeat_1music_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0, 0 },  (void *)playmode_btn_obj},

	{ GO_TYPE_FORECOLOR,	{ MP_BLUE_COLOR } },
	{ GO_TYPE_OBJECT,	{ ICONGOBJ, 0, 1, 2 },  (void *)gobj_repeat},
	{ GO_TYPE_OBJECT,	{ ICONGOBJ, 0, 1, 2 },  (void *)gobj_repeat_one},
	{ 0, { 0, 0, 0, 0 }}
};

#define UO_ID_SHUFFLE	8
#define UO_ID_REPEAT	9

/* SHUFFLE */
static const struct st_ui_button_image ui_view_shuffle = {
	shuffle_btn_obj,
	shuffle_btn_obj_a
};

static const struct st_ui_button_image ui_view_shuffling = {
	shuffling_btn_obj,
	shuffling_btn_obj_a
};

static struct st_ui_button ui_btn_shuffle = {
	UO_ID_SHUFFLE,
	{ {BTN_POS_X_SHUFFLE,  BTN_POS_Y_SHUFFLE}, {BUTTON_WIDTH/2, BUTTON_HEIGHT/2} },
	&ui_view_shuffle,
	UI_BUTTON_ST_NORMAL
};

/* REPEAT */
static const struct st_ui_button_image ui_view_repeat_off = {
	repeat_off_btn_obj,
	repeat_off_btn_obj_a
};

static const struct st_ui_button_image ui_view_repeat_all = {
	repeat_all_btn_obj,
	repeat_all_btn_obj_a
};

static const struct st_ui_button_image ui_view_repeat_1album = {
	repeat_1album_btn_obj,
	repeat_1album_btn_obj_a
};

static const struct st_ui_button_image ui_view_repeat_1music = {
	repeat_1music_btn_obj,
	repeat_1music_btn_obj_a
};

static struct st_ui_button ui_btn_repeat = {
	UO_ID_REPEAT,
	{ {BTN_POS_X_REPEAT,  BTN_POS_Y_REPEAT}, {BUTTON_WIDTH/2, BUTTON_HEIGHT/2} },
	&ui_view_repeat_off,
	UI_BUTTON_ST_NORMAL
};


struct st_ui_button *ui_playmode_view[] = {
	&ui_btn_shuffle,
	&ui_btn_repeat,
	0
};

int flg_shuffle = 0;
int flg_repeat = REPERAT_OFF;

extern int play_file_num;

struct st_music_info *minfo = 0;

static unsigned int playtime;

static unsigned char str_title[MAX_TITLE_LEN+1] = {0};
static unsigned char str_artist[MAX_TITLE_LEN+1] = {0};
static unsigned char str_album_name[MAX_TITLE_LEN+1] = {0};
static unsigned char str_album_track[10] = {0};
static unsigned char str_bitrate[10] = {0};
static unsigned char str_ttime[10] = {0};
static unsigned char str_ptime[10] = {0};
static unsigned char str_all_track[10] = {0};

static struct st_ui_statictext stext_all_track = {
	.view_area = {
		.pos.x		= INFO_WIDTH - (DEFFONT_WIDTH * 9),
		.pos.y		= 0,
		.sur.width	= DEFFONT_WIDTH * 9,
		.sur.height	= DEFFONT_HEIGHR,
	},
	.fore_color	= &fore_color,
	.back_color	= &back_color,
	.font_name	= MPFONT,
	.hattr		= FONT_HATTR_RIGHT,
	.vattr		= FONT_VATTR_TOP,
	.fillattr	= UI_STATICTEXT_FILLARRT_FILL,
	.text		= str_all_track,
};

static struct st_ui_statictext stext_playtime = {
	.view_area = {
#if GSC_GRAPHICS_DISPLAY_WIDTH >= 800
		.pos.x		= 0,
#else
		.pos.x		= (36+(32*2)),
#endif
		.pos.y		= TOPAREAMARGINE + TEXT_INTERVAL * 4,
#if GSC_GRAPHICS_DISPLAY_WIDTH >= 800
		.sur.width	= 48 * 8,
		.sur.height	= 64,
#else
		.sur.width	= 32 * 6,
		.sur.height	= 48,
#endif
	},
	.fore_color	= &fore_color,
	.back_color	= &back_color,
#if GSC_GRAPHICS_DISPLAY_WIDTH >= 800
	.font_name	= "num48x64",
#else
	.font_name	= "num32x48",
#endif
	.hattr		= FONT_HATTR_LEFT,
	.vattr		= FONT_VATTR_TOP,
	.fillattr	= UI_STATICTEXT_FILLARRT_FILL,
	.text		= str_ptime,
};

static struct st_ui_statictext stext_audio_info = {
	.view_area = {
		.pos.x		= INFO_WIDTH - DEFFONT_WIDTH * 11,
		.pos.y		= TOPAREAMARGINE + TEXT_INTERVAL * 4,
		.sur.width	= DEFFONT_WIDTH * 3,
		.sur.height	= DEFFONT_HEIGHR,
	},
	.fore_color	= &fore_color,
	.back_color	= &back_color,
	.font_name	= MPFONT,
	.hattr		= FONT_HATTR_LEFT,
	.vattr		= FONT_VATTR_TOP,
	.fillattr	= UI_STATICTEXT_FILLARRT_FILL,
	.text		= 0,
};

static struct st_ui_statictext stext_bitrate = {
	.view_area = {
		.pos.x		= INFO_WIDTH - DEFFONT_WIDTH * 8,
		.pos.y		= TOPAREAMARGINE + TEXT_INTERVAL * 4,
		.sur.width	= DEFFONT_WIDTH * 8,
		.sur.height	= DEFFONT_HEIGHR,
	},
	.fore_color	= &fore_color,
	.back_color	= &back_color,
	.font_name	= MPFONT,
	.hattr		= FONT_HATTR_LEFT,
	.vattr		= FONT_VATTR_TOP,
	.fillattr	= UI_STATICTEXT_FILLARRT_FILL,
	.text		= str_bitrate,
};

static struct st_ui_statictext stext_ttime = {
	.view_area = {
		.pos.x		= INFO_WIDTH - DEFFONT_WIDTH * 9,
		.pos.y		= TOPAREAMARGINE + TEXT_INTERVAL * 5,
		.sur.width	= DEFFONT_WIDTH * 9,
		.sur.height	= DEFFONT_HEIGHR,
	},
	.fore_color	= &fore_color,
	.back_color	= &back_color,
	.font_name	= MPFONT,
	.hattr		= FONT_HATTR_LEFT,
	.vattr		= FONT_VATTR_TOP,
	.fillattr	= UI_STATICTEXT_FILLARRT_FILL,
	.text		= str_ttime,
};

static struct st_ui_statictext stext_album = {
	.view_area = {
		.pos.x		= 0,
		.pos.y		= TOPAREAMARGINE + TEXT_INTERVAL * 3,
		.sur.width	= DEFFONT_WIDTH * 40,
		.sur.height	= DEFFONT_HEIGHR,
	},
	.fore_color	= &fore_color,
	.back_color	= &back_color,
	.font_name	= MPFONT,
	.hattr		= FONT_HATTR_LEFT,
	.vattr		= FONT_VATTR_TOP,
	.fillattr	= UI_STATICTEXT_FILLARRT_FILL,
	.text		= str_album_name,
};

static struct st_ui_statictext stext_album_track = {
	.view_area = {
		.pos.x		= INFO_WIDTH - DEFFONT_WIDTH * 7,
		.pos.y		= TOPAREAMARGINE + TEXT_INTERVAL * 3,
		.sur.width	= DEFFONT_WIDTH * 7,
		.sur.height	= DEFFONT_HEIGHR,
	},
	.fore_color	= &fore_color,
	.back_color	= &back_color,
	.font_name	= MPFONT,
	.hattr		= FONT_HATTR_RIGHT,
	.vattr		= FONT_VATTR_TOP,
	.fillattr	= UI_STATICTEXT_FILLARRT_FILL,
	.text		= str_album_track,
};

static struct st_ui_statictext stext_artist = {
	.view_area = {
		.pos.x		= 0,
		.pos.y		= TOPAREAMARGINE + TEXT_INTERVAL * 2,
#if GSC_GRAPHICS_DISPLAY_WIDTH >= 800
		.sur.width	= DEFFONT_WIDTH * 42,
#else
		.sur.width	= DEFFONT_WIDTH * 34,
#endif
		.sur.height	= DEFFONT_HEIGHR,
	},
	.fore_color	= &fore_color,
	.back_color	= &back_color,
	.font_name	= MPFONT,
	.hattr		= FONT_HATTR_LEFT,
	.vattr		= FONT_VATTR_TOP,
	.fillattr	= UI_STATICTEXT_FILLARRT_FILL,
	.text		= str_artist,
};

static struct st_ui_statictext stext_title = {
	.view_area = {
		.pos.x		= 0,
		.pos.y		= TOPAREAMARGINE + TEXT_INTERVAL * 1,
#if GSC_GRAPHICS_DISPLAY_WIDTH >= 800
		.sur.width	= DEFFONT_WIDTH * 42,
#else
		.sur.width	= DEFFONT_WIDTH * 34,
#endif
		.sur.height	= DEFFONT_HEIGHR,
	},
	.fore_color	= &fore_color,
	.back_color	= &back_color,
	.font_name	= MPFONT,
	.hattr		= FONT_HATTR_LEFT,
	.vattr		= FONT_VATTR_TOP,
	.fillattr	= UI_STATICTEXT_FILLARRT_FILL,
	.text		= str_title,
};


void setup_musicinfo(void)
{
	if(flg_shuffle == 0) {
		ui_btn_shuffle.view = &ui_view_shuffle;
	} else {
		ui_btn_shuffle.view = &ui_view_shuffling;
	}

	switch(flg_repeat) {
	case REPERAT_OFF:
		ui_btn_repeat.view = &ui_view_repeat_off;
		break;

	case REPERAT_ALL:
		ui_btn_repeat.view = &ui_view_repeat_all;
		break;

	case REPERAT_1ALBUM:
		ui_btn_repeat.view = &ui_view_repeat_1album;
		break;

	case REPERAT_1MUSIC:
		ui_btn_repeat.view = &ui_view_repeat_1music;
		break;

	default:
		break;
	}
}

static void prepare_playtime(unsigned int time)
{
	playtime = time;
#if GSC_GRAPHICS_DISPLAY_WIDTH >= 800
	if(playtime < (60*60)) {
		tsprintf((char *)str_ptime, "   %2d:%02d", playtime/60, playtime % 60);
	} else {
		tsprintf((char *)str_ptime, "%2d:%02d:%02d",  playtime/60/60, (playtime/60) % 60, playtime % 60);
	}
	//tsprintf((char *)str_ptime, "%2d:%02d:%02d",  12, 34, 56);	// TEST
#else
	tsprintf((char *)str_ptime, "%3d:%02d", playtime/60, playtime % 60);
	//tsprintf((char *)str_ptime, "%3d:%02d", 234, 56);	// TEST
#endif
}

static int is_visible(void)
{
#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO
	if(((musicplay_mode == SDCARD) && (sd_disp_mode == MODE_SD_INFO)) ||
	   ((musicplay_mode == RADIO) && (radio_disp_mode == MODE_RADIO_INFO)))
#else
	if(sd_disp_mode == MODE_SD_INFO)
#endif
	{
		if(flg_setting == 0) {
			return 1;
		}
	}

	return 0;
}

void init_musicinfo_view(void)
{
	set_draw_mode(GRP_DRAWMODE_NORMAL);
	set_forecolor(back_color);
	draw_fill_rect((struct st_rect *)&info_rect);
	prepare_playtime(0);

	setup_musicinfo();
}

void reset_musicinfo(void)
{
	if(minfo != 0) {
		minfo->format = MUSIC_FMT_UNKNOWN;
	}

	str_title[0] = 0;
	str_artist[0] = 0;
	str_album_name[0] = 0;
	str_album_track[0] = 0;
	str_bitrate[0] = 0;
	str_ttime[0] = 0;
	str_all_track[0] = 0;
	set_playtime(0);
}

static void draw_playtime(void)
{
	draw_ui_statictext(&stext_playtime);
}

void draw_track_view(void)
{
#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO
	if(musicplay_mode == RADIO) {
		tsprintf((char *)str_all_track, "%4d/%d", select_radio_num + 1, radio_count);
	} else {
		tsprintf((char *)str_all_track, "%4d/%d", play_file_num + 1, music_file_count);
	}
#else
	tsprintf((char *)str_all_track, "%4d/%d", play_file_num + 1, music_file_count);
#endif

	if(flg_setting != 0) {
		return;
	}

	draw_ui_statictext(&stext_all_track);
}

static void draw_title(void)
{
	draw_ui_statictext(&stext_title);
	draw_ui_statictext(&stext_artist);
	draw_ui_statictext(&stext_album);
	draw_ui_statictext(&stext_bitrate);

#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO
	if(musicplay_mode == SDCARD) {
		draw_ui_statictext(&stext_album_track);
		draw_ui_statictext(&stext_ttime);
	}
#else
	draw_ui_statictext(&stext_album_track);
	draw_ui_statictext(&stext_ttime);
#endif
}

static void draw_audioinfo(void)
{
	char *fmts = "";

	if(minfo != 0) {
		switch(minfo->format) {
		case MUSIC_FMT_MP3:
			fmts = "MP3";
			break;

		case MUSIC_FMT_AAC:
			fmts = "AAC";
			break;

		case MUSIC_FMT_WAV:
			fmts = "WAV";
			break;

		case MUSIC_FMT_UNKNOWN:
			fmts = "   ";
			break;

		default:
			fmts = "???";
			break;
		}
	}

	stext_audio_info.text = (uchar *)fmts;
	draw_ui_statictext(&stext_audio_info);
}

void draw_artwork(void)
{
#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO
	if(musicplay_mode == RADIO) {
		draw_graph_object(ARTWORK_LEFT, ARTWORK_TOP, gobj_radio_icon);
		return;
	}
#endif

	if(minfo == 0) {
		return;
	}

#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO
	if(((musicplay_mode == SDCARD) && (sd_disp_mode == MODE_SD_INFO)) ||
	   ((musicplay_mode == RADIO) && (radio_disp_mode == MODE_RADIO_INFO)))
#else
	if(sd_disp_mode == MODE_SD_INFO)
#endif
	{
		if(flg_setting != 0) {
			return;
		}

		if(minfo->flg_have_artwork != 0) {
			draw_image(ARTWORK_LEFT, ARTWORK_TOP, ART_WIDTH, ART_HEIGHT, minfo->artwork, ART_WIDTH);
		} else {
			draw_graph_object(ARTWORK_LEFT, ARTWORK_TOP, gobj_cd_icon);
		}
	}
}

void set_music_info(struct st_music_info *info)
{
	minfo = info;

	tsnprintf((char *)str_title, MAX_TITLE_LEN, "%s", &(minfo->title[0]));
	tsnprintf((char *)str_artist, MAX_TITLE_LEN, "%s", &(minfo->artist[0]));
	tsnprintf((char *)str_album_name, MAX_TITLE_LEN, "%s", &(minfo->album[0]));
	tsprintf((char *)str_bitrate, "%4dKbps", minfo->bit_rate);
	tsprintf((char *)str_album_track, "%4d/%d", minfo->track, minfo->last_track);
	if(minfo->time_length < (60 * 60)) {
		tsprintf((char *)str_ttime, "    %2d:%02d",
			 minfo->time_length/1000/60,
			 (minfo->time_length/1000) % 60);
	} else {
		tsprintf((char *)str_ttime, "%3d:%02d:%02d",
			 minfo->time_length/1000/60/60,
			 (minfo->time_length/1000/60) % 60,
			 (minfo->time_length/1000) % 60);
	}

	draw_track_view();

	if(is_visible() != 0) {
		draw_title();
		draw_audioinfo();
		draw_playtime();
		draw_artwork();
	}
}

void set_playtime(unsigned int time)
{
	prepare_playtime(time);

	if(is_visible() != 0) {
		draw_playtime();
	}
}

extern enum_sd_disp_mode sd_disp_mode;

void draw_music_info(void)
{
	if(is_visible() == 0) {
		return;
	}

	draw_title();
	draw_audioinfo();
	draw_playtime();
	draw_artwork();

#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO
	if(musicplay_mode == SDCARD) {
		draw_ui_button_list(ui_playmode_view);
	}
#else
	draw_ui_button_list(ui_playmode_view);
#endif
}

void set_title_str(uchar *str)
{
	tsprintf((char *)str_title, "%64s", str);
}

void set_artist_str(uchar *str)
{
	tsprintf((char *)str_artist, "%64s", str);
}

void set_album_str(uchar *str)
{
	tsprintf((char *)str_album_name, "%64s", str);
}

/*
 */

void draw_musicinfo_view(void)
{
	set_forecolor(back_color);
	draw_fill_rect((struct st_rect *)&info_rect);
	set_forecolor(fore_color);

	draw_music_info();
}

static void set_shuffle_button_off(void)
{
	ui_btn_shuffle.view = &ui_view_shuffle;
	draw_ui_button((struct st_ui_button *)&ui_btn_shuffle);
}

static void set_shuffle_button_on(void)
{
	ui_btn_shuffle.view = &ui_view_shuffling;
	draw_ui_button((struct st_ui_button *)&ui_btn_shuffle);
}

/**/

static void set_repeat_button_off(void)
{
	ui_btn_repeat.view = &ui_view_repeat_off;
	draw_ui_button((struct st_ui_button *)&ui_btn_repeat);
}

static void set_repeat_button_all(void)
{
	ui_btn_repeat.view = &ui_view_repeat_all;
	draw_ui_button((struct st_ui_button *)&ui_btn_repeat);
}

static void set_repeat_button_1album(void)
{
	ui_btn_repeat.view = &ui_view_repeat_1album;
	draw_ui_button((struct st_ui_button *)&ui_btn_repeat);
}

static void set_repeat_button_1music(void)
{
	ui_btn_repeat.view = &ui_view_repeat_1music;
	draw_ui_button((struct st_ui_button *)&ui_btn_repeat);
}


void musicinfo_proc(struct st_sysevent *event)
{
	struct st_button_event obj_evt;

#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO
	if(musicplay_mode == RADIO) {
		return;
	}
#endif

	if(sd_disp_mode != MODE_SD_INFO) {
		return;
	}

	if(proc_ui_button_list(&obj_evt, ui_playmode_view, event) != 0) {
		switch(obj_evt.id) {
		case UO_ID_SHUFFLE:
			if(obj_evt.what == UI_BUTTON_EVT_PUSH) {
				if(flg_shuffle == 0) {
					flg_shuffle = 1;
					set_shuffle_button_on();
				} else {
					flg_shuffle = 0;
					set_shuffle_button_off();
				}
				save_config();
			}
			break;

		case UO_ID_REPEAT:
			if(obj_evt.what == UI_BUTTON_EVT_PUSH) {
				switch(flg_repeat) {
				case REPERAT_OFF:
					flg_repeat = REPERAT_ALL;
					set_repeat_button_all();
					break;

				case REPERAT_ALL:
					flg_repeat = REPERAT_1ALBUM;
					set_repeat_button_1album();
					break;

				case REPERAT_1ALBUM:
					flg_repeat = REPERAT_1MUSIC;
					set_repeat_button_1music();
					break;

				case REPERAT_1MUSIC:
					flg_repeat = REPERAT_OFF;
					set_repeat_button_off();
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
}
