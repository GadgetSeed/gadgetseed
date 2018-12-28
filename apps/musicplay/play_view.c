/** @file
    @brief	音楽再生表示

    @date	2017.05.02
    @auther	Takashi SHUDO
*/

#include "sysconfig.h"
#include "device/audio_ioctl.h"
#include "str.h"
#include "graphics.h"
#include "graphics_object.h"
#include "font.h"
#include "tprintf.h"
#include "shell.h"
#include "../soundplay/music_info.h"
#include "ui_button.h"
#include "ui_seekbar.h"

#include "filelist.h"
#include "musicplay_view.h"
#include "musicplay.h"
#include "play_view.h"
#include "list_view.h"
#include "spectrum_view.h"

//#define DEBUGTBITS 0x02
#include "dtprintf.h"


static struct st_music_info *minfo = 0;

static unsigned int playtime;

static unsigned char title[MAX_TITLE_LEN+1] = {0};
static unsigned char artist[MAX_TITLE_LEN+1] = {0};
static unsigned char album[MAX_TITLE_LEN+1] = {0};
static unsigned char str_ttime[10] = {0};
static unsigned char str_ptime[10] = {0};
static unsigned char track[10] = {0};
int flg_shuffle = 0;

static const struct st_rect screen_rect = { 0, 0, GSC_GRAPHICS_DISPLAY_WIDTH, GSC_GRAPHICS_DISPLAY_HEIGHT };
const struct st_rect info_rect = { 0, 0, INFO_WIDTH, INFO_HEIGHT };

const unsigned int active_color = MP_ACTIVE_BACK_COLOR;
const unsigned int fore_color = MP_FORE_COLOR;
const unsigned int back_color = MP_BACK_COLOR;
const unsigned int green_color = MP_GREEN_COLOR;

#define BTN_POS_X_FR	 16
#define BTN_POS_Y_FR	H_BTN_TOP

#define BTN_POS_X_STOP	(BTN_POS_X_FR + BTN_INTERVAL + BUTTON_WIDTH)
#define BTN_POS_Y_STOP	H_BTN_TOP

#define BTN_POS_X_PLAY	(BTN_POS_X_FR + (BTN_INTERVAL + BUTTON_WIDTH) * 2)
#define BTN_POS_Y_PLAY	H_BTN_TOP

#define BTN_POS_X_FF	(BTN_POS_X_FR + (BTN_INTERVAL + BUTTON_WIDTH) * 3)
#define BTN_POS_Y_FF	H_BTN_TOP

#define BTN_POS_X_SHUFFLE	(BTN_POS_X_FR + (BTN_INTERVAL + BUTTON_WIDTH) * 4)
#define BTN_POS_Y_SHUFFLE	H_BTN_TOP

#define BTN_POS_X_LIST	V_BTN_LEFT
#define BTN_POS_Y_LIST	H_BTN_TOP

const struct st_graph_object stop_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FILL_BOX,	{ BUTTON_WIDTH/4, BUTTON_HEIGHT/4, BUTTON_WIDTH/2, BUTTON_HEIGHT/2 } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object stop_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FILL_BOX,	{ BUTTON_WIDTH/4, BUTTON_HEIGHT/4, BUTTON_WIDTH/2, BUTTON_HEIGHT/2 } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object play_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_VERTEX4,	{ BUTTON_WIDTH/4, BUTTON_HEIGHT/4, BUTTON_WIDTH/4*3, BUTTON_HEIGHT/2,
				  BUTTON_WIDTH/4*3, BUTTON_HEIGHT/2, BUTTON_WIDTH/4, BUTTON_HEIGHT/4*3} },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object play_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_VERTEX4,	{ BUTTON_WIDTH/4, BUTTON_HEIGHT/4, BUTTON_WIDTH/4*3, BUTTON_HEIGHT/2,
				  BUTTON_WIDTH/4*3, BUTTON_HEIGHT/2, BUTTON_WIDTH/4, BUTTON_HEIGHT/4*3} },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object playing_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_GREEN_COLOR } },
	{ GO_TYPE_VERTEX4,	{ BUTTON_WIDTH/4, BUTTON_HEIGHT/4, BUTTON_WIDTH/4*3, BUTTON_HEIGHT/2,
				  BUTTON_WIDTH/4*3, BUTTON_HEIGHT/2, BUTTON_WIDTH/4, BUTTON_HEIGHT/4*3} },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object playing_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_GREEN_COLOR } },
	{ GO_TYPE_VERTEX4,	{ BUTTON_WIDTH/4, BUTTON_HEIGHT/4, BUTTON_WIDTH/4*3, BUTTON_HEIGHT/2,
				  BUTTON_WIDTH/4*3, BUTTON_HEIGHT/2, BUTTON_WIDTH/4, BUTTON_HEIGHT/4*3} },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object ff_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_VERTEX4,	{ BUTTON_WIDTH/4, BUTTON_HEIGHT/4, BUTTON_WIDTH/4*2, BUTTON_HEIGHT/2,
				  BUTTON_WIDTH/4*2, BUTTON_HEIGHT/2, BUTTON_WIDTH/4, BUTTON_HEIGHT/4*3} },
	{ GO_TYPE_VERTEX4,	{ BUTTON_WIDTH/2, BUTTON_HEIGHT/4, BUTTON_WIDTH/4*3, BUTTON_HEIGHT/2,
				  BUTTON_WIDTH/4*3, BUTTON_HEIGHT/2, BUTTON_WIDTH/2, BUTTON_HEIGHT/4*3} },
	{ GO_TYPE_FILL_BOX,	{ BUTTON_WIDTH/4*3, BUTTON_HEIGHT/4, BUTTON_WIDTH/8, BUTTON_HEIGHT/2 } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object ff_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_VERTEX4,	{ BUTTON_WIDTH/4, BUTTON_HEIGHT/4, BUTTON_WIDTH/4*2, BUTTON_HEIGHT/2,
				  BUTTON_WIDTH/4*2, BUTTON_HEIGHT/2, BUTTON_WIDTH/4, BUTTON_HEIGHT/4*3} },
	{ GO_TYPE_VERTEX4,	{ BUTTON_WIDTH/2, BUTTON_HEIGHT/4, BUTTON_WIDTH/4*3, BUTTON_HEIGHT/2,
				  BUTTON_WIDTH/4*3, BUTTON_HEIGHT/2, BUTTON_WIDTH/2, BUTTON_HEIGHT/4*3} },
	{ GO_TYPE_FILL_BOX,	{ BUTTON_WIDTH/4*3, BUTTON_HEIGHT/4, BUTTON_WIDTH/8, BUTTON_HEIGHT/2 } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object fr_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FILL_BOX,	{ BUTTON_WIDTH/8, BUTTON_HEIGHT/4, BUTTON_WIDTH/8, BUTTON_HEIGHT/2 } },
	{ GO_TYPE_VERTEX4,	{ BUTTON_WIDTH/4, BUTTON_HEIGHT/2, BUTTON_WIDTH/2, BUTTON_HEIGHT/4,
				  BUTTON_WIDTH/2, BUTTON_HEIGHT/4, BUTTON_WIDTH/2, BUTTON_HEIGHT/4*3} },
	{ GO_TYPE_VERTEX4,	{ BUTTON_WIDTH/2, BUTTON_HEIGHT/2, BUTTON_WIDTH/4*3, BUTTON_HEIGHT/4,
				  BUTTON_WIDTH/4*3, BUTTON_HEIGHT/4, BUTTON_WIDTH/4*3, BUTTON_HEIGHT/4*3} },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object fr_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FILL_BOX,	{ BUTTON_WIDTH/8, BUTTON_HEIGHT/4, BUTTON_WIDTH/8, BUTTON_HEIGHT/2 } },
	{ GO_TYPE_VERTEX4,	{ BUTTON_WIDTH/4, BUTTON_HEIGHT/2, BUTTON_WIDTH/2, BUTTON_HEIGHT/4,
				  BUTTON_WIDTH/2, BUTTON_HEIGHT/4, BUTTON_WIDTH/2, BUTTON_HEIGHT/4*3} },
	{ GO_TYPE_VERTEX4,	{ BUTTON_WIDTH/2, BUTTON_HEIGHT/2, BUTTON_WIDTH/4*3, BUTTON_HEIGHT/4,
				  BUTTON_WIDTH/4*3, BUTTON_HEIGHT/4, BUTTON_WIDTH/4*3, BUTTON_HEIGHT/4*3} },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object list_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FILL_BOX,	{ BUTTON_WIDTH/4, BUTTON_HEIGHT/4, BUTTON_WIDTH/2, BUTTON_HEIGHT/8 } },
	{ GO_TYPE_FILL_BOX,	{ BUTTON_WIDTH/4, BUTTON_HEIGHT/16*7, BUTTON_WIDTH/2, BUTTON_HEIGHT/8 } },
	{ GO_TYPE_FILL_BOX,	{ BUTTON_WIDTH/4, BUTTON_HEIGHT/8*5, BUTTON_WIDTH/2, BUTTON_HEIGHT/8 } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object list_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FILL_BOX,	{ BUTTON_WIDTH/4, BUTTON_HEIGHT/4, BUTTON_WIDTH/2, BUTTON_HEIGHT/8 } },
	{ GO_TYPE_FILL_BOX,	{ BUTTON_WIDTH/4, BUTTON_HEIGHT/16*7, BUTTON_WIDTH/2, BUTTON_HEIGHT/8 } },
	{ GO_TYPE_FILL_BOX,	{ BUTTON_WIDTH/4, BUTTON_HEIGHT/8*5, BUTTON_WIDTH/2, BUTTON_HEIGHT/8 } },
	{ 0, { 0, 0, 0, 0 }}
};


#define BWS	(BUTTON_WIDTH/16)
#define BHS	(BUTTON_HEIGHT/16)

#define SHUFFLE_OBJS(X) \
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } }, \
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },				\
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } }, \
	{ GO_TYPE_FORECOLOR,	{ (X) } },				\
	{ GO_TYPE_VERTEX4,	{ BWS*2, BHS*3, BWS*2, BHS*5, BWS*3, BHS*5, BWS*4, BHS*3 } }, \
	{ GO_TYPE_VERTEX4,	{ BWS*4, BHS*3, BWS*3, BHS*5, BWS*11, BHS*13, BWS*12, BHS*11 } }, \
	{ GO_TYPE_VERTEX4,	{ BWS*12, BHS*11, BWS*11, BHS*13, BWS*13, BHS*13, BWS*13, BHS*11 } }, \
	{ GO_TYPE_VERTEX4,	{ BWS*13, BHS*10, BWS*13, BHS*14, BWS*15, BHS*12, BWS*15, BHS*12 } }, \
	{ GO_TYPE_VERTEX4,	{ BWS*2, BHS*11, BWS*2, BHS*13, BWS*4, BHS*13, BWS*3, BHS*11 } }, \
	{ GO_TYPE_VERTEX4,	{ BWS*5, BHS*9, BWS*3, BHS*11, BWS*4, BHS*13, BWS*7, BHS*10 } }, \
	{ GO_TYPE_VERTEX4,	{ BWS*11, BHS*3, BWS*8, BHS*6, BWS*10, BHS*7, BWS*12, BHS*5 } }, \
	{ GO_TYPE_VERTEX4,	{ BWS*11, BHS*3, BWS*12, BHS*5, BWS*13, BHS*5, BWS*13, BHS*3 } }, \
	{ GO_TYPE_VERTEX4,	{ BWS*13, BHS*2, BWS*13, BHS*6, BWS*15, BHS*4, BWS*15, BHS*4 } }, \
	{ 0, { 0, 0, 0, 0 }}

const struct st_graph_object shuffle_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	SHUFFLE_OBJS(MP_FORE_COLOR)
};

const struct st_graph_object shuffle_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	SHUFFLE_OBJS(MP_FORE_COLOR)
};

const struct st_graph_object shuffling_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	SHUFFLE_OBJS(MP_BLUE_COLOR)
};

const struct st_graph_object shuffling_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	SHUFFLE_OBJS(MP_BLUE_COLOR)
};

#define UO_ID_STOP	3
#define UO_ID_PLAY	4
#define UO_ID_FF	5
#define UO_ID_FR	6
#define UO_ID_LIST	7
#define UO_ID_SHUFFLE	8

/* [■] */
static const struct st_ui_button_image ui_view_stop = {
	{ {BTN_POS_X_STOP,  BTN_POS_Y_STOP}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	stop_btn_obj,
	stop_btn_obj_a
};

static struct st_ui_button ui_btn_stop = {
	UO_ID_STOP,
	&ui_view_stop,
	UI_BUTTON_ST_NORMAL
};

/* [▶] */
static const struct st_ui_button_image ui_view_play = {
	{ {BTN_POS_X_PLAY,  BTN_POS_Y_PLAY}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	play_btn_obj,
	play_btn_obj_a
};

static const struct st_ui_button_image ui_view_playing = {
	{ {BTN_POS_X_PLAY,  BTN_POS_Y_PLAY}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	playing_btn_obj,
	playing_btn_obj_a
};

static struct st_ui_button ui_btn_play = {
	UO_ID_PLAY,
	&ui_view_play,
	UI_BUTTON_ST_NORMAL
};

/* [▶▶|] */
static const struct st_ui_button_image ui_view_ff = {
	{ {BTN_POS_X_FF,  BTN_POS_Y_FF}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	ff_btn_obj,
	ff_btn_obj_a
};

static struct st_ui_button ui_btn_ff = {
	UO_ID_FF,
	&ui_view_ff,
	UI_BUTTON_ST_NORMAL
};

/* [|◀◀] */
static const struct st_ui_button_image ui_view_fr = {
	{ {BTN_POS_X_FR,  BTN_POS_Y_FR}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	fr_btn_obj,
	fr_btn_obj_a
};

static struct st_ui_button ui_btn_fr = {
	UO_ID_FR,
	&ui_view_fr,
	UI_BUTTON_ST_NORMAL
};

/* [≡] */
static const struct st_ui_button_image ui_view_list = {
	{ {BTN_POS_X_LIST,  BTN_POS_Y_LIST}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	list_btn_obj,
	list_btn_obj_a
};

static struct st_ui_button ui_btn_list = {
	UO_ID_LIST,
	&ui_view_list,
	UI_BUTTON_ST_NORMAL
};

/* SHUFFLE */
static const struct st_ui_button_image ui_view_shuffle = {
	{ {BTN_POS_X_SHUFFLE,  BTN_POS_Y_SHUFFLE}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	shuffle_btn_obj,
	shuffle_btn_obj_a
};

static const struct st_ui_button_image ui_view_shuffling = {
	{ {BTN_POS_X_SHUFFLE,  BTN_POS_Y_SHUFFLE}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	shuffling_btn_obj,
	shuffling_btn_obj_a
};

static struct st_ui_button ui_btn_shuffle = {
	UO_ID_SHUFFLE,
	&ui_view_shuffle,
	UI_BUTTON_ST_NORMAL
};


/* PLAY画面 */
struct st_ui_button *ui_play_view[] = {
	&ui_btn_stop,
	&ui_btn_play,
	&ui_btn_ff,
	&ui_btn_fr,
	&ui_btn_list,
	&ui_btn_shuffle,
	0
};

extern const struct st_graph_object normal_color_view[];

static struct st_ui_seekbar ui_playtime_slider = {
	.view_area = {{0, H_BTN_TOP - SCRBAR_WIDTH - 8}, {INFO_WIDTH, SCRBAR_WIDTH} },
	.type = UI_SKB_TYPE_HOLIZONTAL,
	.attr = UI_SKB_ATTR_DROP_VALUE_CHANGE,
	.normal_view = normal_color_view,
	.bar_color = RGB(0,100,100),
	.value = 0,
	.max_value = INFO_WIDTH,
};


void init_musicplay_view(void)
{
	clear_screen();
	set_draw_mode(GRP_DRAWMODE_NORMAL);

	set_forecolor(back_color);
	draw_fill_rect((struct st_rect *)&screen_rect);

	set_forecolor(fore_color);
	set_backcolor(back_color);
}

static void draw_playtime(void)
{
	set_forecolor(MP_FORE_COLOR);
	set_backcolor(MP_BACK_COLOR);
	set_font_by_name("num48x64");
	set_font_drawmode(FONT_FIXEDWIDTH);
	draw_str(0, TEXT_INTERVAL * 4, str_ptime);
}

static void draw_title(void)
{
	set_forecolor(MP_FORE_COLOR);
	set_backcolor(MP_BACK_COLOR);
	set_font_by_name(MPFONT);
	set_font_drawmode(FONT_FIXEDWIDTH);

	draw_str(0, 0, title);
	draw_str(0, TEXT_INTERVAL, artist);
	draw_str(0, TEXT_INTERVAL * 2, album);
	draw_str(48, TEXT_INTERVAL * 3, str_ttime);
	draw_str(INFO_WIDTH - 96, TEXT_INTERVAL * 5, track);
}

static void draw_audioinfo(void)
{
	char *fmts = "";

	set_forecolor(MP_FORE_COLOR);
	set_backcolor(MP_BACK_COLOR);
	set_font_by_name(MPFONT);
	set_font_drawmode(FONT_FIXEDWIDTH);

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
		default:
			fmts = "???";
			break;
		}
	}

	draw_str(0, TEXT_INTERVAL * 3, (unsigned char *)fmts);
}

static void draw_artwork(void)
{
	if(minfo == 0) {
		return;
	}

	if(minfo->flg_have_artwork != 0) {
		draw_image(ARTWORK_LEFT, ARTWORK_TOP, ART_WIDTH, ART_HEIGHT, minfo->artwork, ART_WIDTH);
	} else {
		struct st_box artbox = { {ARTWORK_LEFT, ARTWORK_TOP}, {ART_WIDTH, ART_HEIGHT} };
		set_forecolor(back_color);
		draw_fill_box(&artbox);
	}
}

void set_music_info(struct st_music_info *info)
{
	minfo = info;

	tsprintf((char *)title, "%48s", &(minfo->title[0]));
	tsprintf((char *)artist, "%48s", &(minfo->artist[0]));
	tsprintf((char *)album, "%d/%d %42s",
		 minfo->track, minfo->last_track,
		 &(minfo->album[0]));
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
	tsprintf((char *)track, "%4d/%d", play_file_num + 1, music_file_count);
	set_playtime(0);

	if(disp_mode == MODE_PLAY) {
		draw_title();
		draw_audioinfo();
		draw_playtime();
		draw_artwork();
	}
}

void set_playtime(unsigned int time)
{
	playtime = time;
	if(playtime < (60*60)) {
		tsprintf((char *)str_ptime, "   %2d:%02d", playtime/60, playtime % 60);
	} else {
		tsprintf((char *)str_ptime, "%2d:%02d:%02d",  playtime/60/60, (playtime/60) % 60, playtime % 60);
	}

	if(disp_mode == MODE_PLAY) {
		draw_playtime();
	}

#if 0
	if(minfo->time_length != 0) {
		set_value_ui_seekbar(&ui_playtime_slider,
				    (playtime * INFO_WIDTH)/(minfo->time_length/1000));
	}
#endif
}


/*
 */

void set_play_button_playing(void)
{
	ui_btn_play.view = &ui_view_playing;
	draw_ui_button((struct st_ui_button *)&ui_btn_play);
}

static void set_play_button_stoping(void)
{
	ui_btn_play.view = &ui_view_play;
	draw_ui_button((struct st_ui_button *)&ui_btn_play);
}


static void set_shuffle_button_shuffling(void)
{
	ui_btn_shuffle.view = &ui_view_shuffling;
	draw_ui_button((struct st_ui_button *)&ui_btn_shuffle);
}

static void set_shuffle_button_shuffle(void)
{
	ui_btn_shuffle.view = &ui_view_shuffle;
	draw_ui_button((struct st_ui_button *)&ui_btn_shuffle);
}

/*
 *
 */

void init_play_view(void)
{
	struct st_box fbox = { {0, 0}, {GSC_GRAPHICS_DISPLAY_WIDTH, GSC_GRAPHICS_DISPLAY_HEIGHT} };

	set_forecolor(back_color);
	set_backcolor(back_color);
	draw_fill_box(&fbox);
	draw_ui_button_list(ui_play_view);
	draw_ui_seekbar(&ui_playtime_slider);
}

static void draw_music_info(void)
{
	draw_title();
	draw_audioinfo();
	draw_playtime();
	draw_artwork();
	draw_spectrum(0);
}

void draw_play_view(void)
{
	set_forecolor(back_color);
	draw_fill_rect((struct st_rect *)&info_rect);
	set_forecolor(fore_color);

	draw_music_info();
}

void set_playtime_slider(unsigned int frame_num)
{
	if(minfo->sample_count != 0) {
		set_value_ui_seekbar(&ui_playtime_slider,
				    (frame_num * INFO_WIDTH)/minfo->sample_count);
	}
}

int flg_frame_move = 0;

static void set_frame(unsigned long frame)
{
	uchar cmd[32];

	tsprintf((char *)cmd, "sound move %ld", frame);
	flg_frame_move = 1;
	exec_command(cmd);
}

void play_proc(struct st_sysevent *event)
{
	struct st_button_event obj_evt;

	if(proc_ui_button_list(&obj_evt, ui_play_view, event) != 0) {
		switch(obj_evt.id) {
		case UO_ID_STOP:
			if(obj_evt.what == UI_BUTTON_EVT_PUSH) {
				tprintf("STOP\n");
				//musicplay_status = MUSICPLAY_STAT_STOP;
				set_play_button_stoping();
				//stop_music_play();
				pause_music_play();
			}
			break;

		case UO_ID_PLAY:
			if(obj_evt.what == UI_BUTTON_EVT_PUSH) {
				tprintf("PLAY\n");
				if(musicplay_status == MUSICPLAY_STAT_STOP) {
					start_music_play();
				} else {
					continue_music_play();
				}
			}
			break;

		case UO_ID_FF:
			if(obj_evt.what == UI_BUTTON_EVT_PUSH) {
				tprintf("FF\n");
				ff_music_play();
				if(musicplay_status == MUSICPLAY_STAT_PLAY) {
					stop_music_play();
				} else if(musicplay_status == MUSICPLAY_STAT_PAUSE) {
					stop_music_play();
					musicplay_status = MUSICPLAY_STAT_STOP;
					analyze_now_music();
				} else {
					analyze_now_music();
				}
			}
			break;

		case UO_ID_FR:
			if(obj_evt.what == UI_BUTTON_EVT_PUSH) {
				tprintf("FR\n");
				fr_music_play();
				if(musicplay_status == MUSICPLAY_STAT_PLAY) {
					stop_music_play();
				} else if(musicplay_status == MUSICPLAY_STAT_PAUSE) {
					stop_music_play();
					musicplay_status = MUSICPLAY_STAT_STOP;
					analyze_now_music();
				} else {
					analyze_now_music();
				}
			}
			break;

		case UO_ID_LIST:
			if(obj_evt.what == UI_BUTTON_EVT_PUSH) {
				switch(disp_mode) {
				case MODE_PLAY:
					tprintf("ALBUM LIST\n");
					disp_mode = MODE_ALBUM_SEL;
					prepare_album_view();
					draw_list_view();
					break;

				case MODE_ALBUM_SEL:
					tprintf("MUSIC LIST\n");
					disp_mode = MODE_MUSIC_SEL;
					prepare_music_view();
					draw_list_view();
					break;

				case MODE_MUSIC_SEL:
					tprintf("MUSIC PLAY INFO\n");
					disp_mode = MODE_PLAY;
					draw_play_view();
					break;

				default:
					break;
				}
			}
			break;

		case UO_ID_SHUFFLE:
			if(obj_evt.what == UI_BUTTON_EVT_PUSH) {
				if(flg_shuffle == 0) {
					flg_shuffle = 1;
					set_shuffle_button_shuffling();
				} else {
					flg_shuffle = 0;
					set_shuffle_button_shuffle();
				}
			}
			break;

		default:
			break;
		}
	}

	int new_time;

	if(proc_ui_seekbar(&ui_playtime_slider, event, &new_time) == UI_SKB_EVT_CHANGE) {
		unsigned int next_frame;
		next_frame = (minfo->sample_count * new_time) / INFO_WIDTH;
		//tprintf("NF:%d %ld\n", new_time, next_frame);

		set_frame(next_frame);
	}
}

/*
 */

void draw_searching(void)
{
	static const unsigned char str_searching[] = "Music File Searching...";
	int strw;

	set_font_by_name(MPFONT);
	set_font_drawmode(FONT_FIXEDWIDTH);

	strw = str_width((unsigned char *)str_searching);
	draw_str((GSC_GRAPHICS_DISPLAY_WIDTH - strw)/2, GSC_GRAPHICS_DISPLAY_HEIGHT/2 - font_height(),
		 (unsigned char *)str_searching);
}

void draw_search_count(int count)
{
	unsigned char str_cnt[8];
	int strw;

	tsprintf((char *)str_cnt, "%5d", count);
	strw = str_width(str_cnt);
	draw_str((GSC_GRAPHICS_DISPLAY_WIDTH - strw)/2, GSC_GRAPHICS_DISPLAY_HEIGHT/2 + font_height()*2,
		 str_cnt);
}

void draw_search_album_count(int count)
{
	unsigned char str_cnt[8];
	int strw;

	tsprintf((char *)str_cnt, "%5d", count);
	strw = str_width(str_cnt);
	draw_str((GSC_GRAPHICS_DISPLAY_WIDTH - strw)/2, GSC_GRAPHICS_DISPLAY_HEIGHT/2 + font_height()*4,
		 str_cnt);
}
