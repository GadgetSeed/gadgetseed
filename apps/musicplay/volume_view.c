/** @file
    @brief	ボリューム表示

    @date	2017.06.03
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
#include "ui_button.h"
#include "ui_seekbar.h"

#include "musicplay.h"
#include "musicplay_view.h"
#include "volume_view.h"

//#define DEBUGTBITS 0x01
#include "dtprintf.h"


#define VOL_MAX		 99
#define VOL_MIN		  0

static int volume;
static volatile int flg_mute = 0;

extern const unsigned long fore_color;
extern const unsigned long back_color;

/*
 */

#if GSC_GRAPHICS_DISPLAY_WIDTH < 800
#undef BUTTON_HEIGHT
#define BUTTON_HEIGHT	48
#endif

#define BTN_POS_X_PLUS	V_BTN_LEFT
#define BTN_POS_Y_PLUS	TEXT_INTERVAL

#define BTN_POS_X_MINUS	V_BTN_LEFT
#define BTN_POS_Y_MINUS	(BTN_POS_Y_PLUS + BUTTON_HEIGHT + BTN_INTERVAL)

#define BTN_POS_X_MUTE	V_BTN_LEFT
#define BTN_POS_Y_MUTE	(BTN_POS_Y_PLUS + BUTTON_HEIGHT + BTN_INTERVAL + BUTTON_HEIGHT + BTN_INTERVAL)

#define UO_ID_PLUS	1
#define UO_ID_MINUS	2
#define UO_ID_MUTE	3

const struct st_box plus_btn_box	= { {BTN_POS_X_PLUS,  BTN_POS_Y_PLUS}, {BUTTON_WIDTH, BUTTON_HEIGHT} };
const struct st_box minus_btn_box	= { {BTN_POS_X_MINUS, BTN_POS_Y_MINUS}, {BUTTON_WIDTH, BUTTON_HEIGHT} };


const struct st_graph_object plus_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FILL_BOX,	{ BUTTON_WIDTH/4, BUTTON_HEIGHT/16*7, BUTTON_WIDTH/2, BUTTON_HEIGHT/8 } },
	{ GO_TYPE_FILL_BOX,	{ BUTTON_WIDTH/16*7, BUTTON_HEIGHT/4, BUTTON_WIDTH/8, BUTTON_HEIGHT/2 } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object plus_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FILL_BOX,	{ BUTTON_WIDTH/4, BUTTON_HEIGHT/16*7, BUTTON_WIDTH/2, BUTTON_HEIGHT/8 } },
	{ GO_TYPE_FILL_BOX,	{ BUTTON_WIDTH/16*7, BUTTON_HEIGHT/4, BUTTON_WIDTH/8, BUTTON_HEIGHT/2 } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object minus_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FILL_BOX,	{ BUTTON_WIDTH/4, BUTTON_HEIGHT/16*7, BUTTON_WIDTH/2, BUTTON_HEIGHT/8 } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object minus_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FILL_BOX,	{ BUTTON_WIDTH/4, BUTTON_HEIGHT/16*7, BUTTON_WIDTH/2, BUTTON_HEIGHT/8 } },
	{ 0, { 0, 0, 0, 0 }}
};

#define BWS	(BUTTON_WIDTH/16)
#define BHS	(BUTTON_HEIGHT/16)

#define MUTE_OBJS(X)							\
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } }, \
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },				\
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } }, \
	{ GO_TYPE_FORECOLOR,	{ (X) } },				\
	{ GO_TYPE_FILL_BOX,	{ BWS*2, BHS*6, BWS*2, BHS*4 } }, \
	{ GO_TYPE_VERTEX4,	{ BWS*8, BHS*2, BWS*8, BHS*14, BWS*4, BHS*10, BWS*4, BHS*6 } }, \
	{ GO_TYPE_VERTEX4,	{ BWS*11, BHS*6, BWS*10, BHS*7, BWS*13, BHS*10, BWS*14, BHS*9 } }, \
	{ GO_TYPE_VERTEX4,	{ BWS*13, BHS*6, BWS*12, BHS*7, BWS*13, BHS*8, BWS*14, BHS*7 } }, \
	{ GO_TYPE_VERTEX4,	{ BWS*11, BHS*8, BWS*12, BHS*9, BWS*11, BHS*10, BWS*10, BHS*9 } }, \
	{ 0, { 0, 0, 0, 0 }}

const struct st_graph_object mute_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	MUTE_OBJS(MP_FORE_COLOR)
};

const struct st_graph_object mute_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	MUTE_OBJS(MP_FORE_COLOR)
};

const struct st_graph_object muting_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	MUTE_OBJS(MP_RED_COLOR)
};

const struct st_graph_object muting_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	MUTE_OBJS(MP_RED_COLOR)
};

/* [+] */
static const struct st_ui_button_image ui_view_plus = {
	{ {BTN_POS_X_PLUS,  BTN_POS_Y_PLUS}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	plus_btn_obj,
	plus_btn_obj_a
};

static struct st_ui_button ui_btn_plus = {
	UO_ID_PLUS,
	&ui_view_plus,
	UI_BUTTON_ST_NORMAL
};

/* [-] */
static const struct st_ui_button_image ui_view_minus = {
	{ {BTN_POS_X_MINUS,  BTN_POS_Y_MINUS}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	minus_btn_obj,
	minus_btn_obj_a
};

static struct st_ui_button ui_btn_minus = {
	UO_ID_MINUS,
	&ui_view_minus,
	UI_BUTTON_ST_NORMAL
};

/* MUTE */
static const struct st_ui_button_image ui_view_mute = {
	{ {BTN_POS_X_MUTE,  BTN_POS_Y_MUTE}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	mute_btn_obj,
	mute_btn_obj_a
};

static const struct st_ui_button_image ui_view_muting = {
	{ {BTN_POS_X_MUTE,  BTN_POS_Y_MUTE}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	muting_btn_obj,
	muting_btn_obj_a
};

static struct st_ui_button ui_btn_mute = {
	UO_ID_MUTE,
	&ui_view_mute,
	UI_BUTTON_ST_NORMAL
};


/* VOLUME画面 */
struct st_ui_button *ui_volume_view[] = {
	&ui_btn_plus,
	&ui_btn_minus,
	&ui_btn_mute,
	0
};


const struct st_graph_object normal_color_view[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ MP_BACK_COLOR } },
	{ 0, { 0, 0, 0, 0 }}
};

static struct st_ui_seekbar ui_volume_slider = {
	.view_area = {{V_BTN_LEFT - SCRBAR_WIDTH - 4, BTN_POS_Y_PLUS}, {SCRBAR_WIDTH, INFO_HEIGHT-BTN_POS_Y_PLUS} },
	.type = UI_SKB_TYPE_VERTICAL,
	.attr = UI_SKB_ATTR_REALTIME_VALUE_CAHNGE,
	.normal_view = normal_color_view,
	.bar_color = RGB(0,100,100),
	.value = VOL_DEF,
	.max_value = 99,
};

static void set_mute_button_muting(void)
{
	ui_btn_mute.view = &ui_view_muting;
	draw_ui_button((struct st_ui_button *)&ui_btn_mute);
}

static void set_mute_button_notmute(void)
{
	ui_btn_mute.view = &ui_view_mute;
	draw_ui_button((struct st_ui_button *)&ui_btn_mute);
}

static void draw_volume_value(void)
{
	unsigned char str[8];

	set_font_by_name(MPFONT);
	set_font_drawmode(FONT_FIXEDWIDTH);
	tsprintf((char *)str, "VOL %2d", volume);
	set_forecolor(fore_color);
	draw_str(V_BTN_LEFT, 0, str);
}

void set_disp_volume(int vol)
{
	DTPRINTF(0x01, "flg_mute = %d\n", flg_mute);

	if(flg_mute == 0) {
		DTPRINTF(0x01, "New volume = %d\n", vol);
		volume = vol;
		set_value_ui_seekbar(&ui_volume_slider, volume);
		draw_volume_value();
	} else {
		DTPRINTF(0x01, "MUTE\n");
	}
}

/*
 *
 */

void init_volume_view(void)
{
	draw_ui_button_list(ui_volume_view);
}

void draw_volume_view(void)
{
	draw_ui_seekbar(&ui_volume_slider);

	draw_volume_value();
}

static void set_now_volume(unsigned short vol)
{
	uchar cmd[32];

	tsprintf((char *)cmd, "sound volume %d", vol);
	exec_command(cmd);
}

void set_volume(unsigned short vol)
{
	volume = vol;

	set_now_volume(volume);
}

void volume_proc(struct st_sysevent *event)
{
	struct st_button_event obj_evt;
	int new_volume;

	if(proc_ui_button_list(&obj_evt, ui_volume_view, event) != 0) {
		DTPRINTF(0x02, "EVT %d\n", obj_evt.id);
		switch(obj_evt.id) {
		case UO_ID_PLUS:
			switch(obj_evt.what) {
			case UI_BUTTON_EVT_PUSH:
				if(volume < VOL_MAX) {
					volume ++;
					set_now_volume(volume);
					if(flg_mute !=0 ) {
						flg_mute = 0;
						set_mute_button_notmute();
					}
				}
				break;
			}
			break;

		case UO_ID_MINUS:
			switch(obj_evt.what) {
			case UI_BUTTON_EVT_PUSH:
				if(VOL_MIN < volume) {
					volume --;
					set_now_volume(volume);
					if(flg_mute !=0 ) {
						flg_mute = 0;
						set_mute_button_notmute();
					}
				}
				break;
			}
			break;

		case UO_ID_MUTE:
			switch(obj_evt.what) {
			case UI_BUTTON_EVT_PUSH:
				if(flg_mute == 0) {
					flg_mute = 1;
					set_mute_button_muting();
					set_now_volume(0);
				} else {
					flg_mute = 0;
					set_mute_button_notmute();
					set_now_volume(volume);
				}
				break;
			}
			break;

		default:
			break;

		}
	}

	if(proc_ui_seekbar(&ui_volume_slider, event, &new_volume) == UI_SKB_EVT_CHANGE) {
		DTPRINTF(0x01, "slider = %d\n", new_volume);
		flg_mute = 0;
		set_mute_button_notmute();
		set_volume(new_volume);
	}

	switch(event->what) {
	case EVT_SOUND_VOLUME:
		DTPRINTF(0x01, "volume = %d\n", event->arg);
		set_disp_volume(event->arg);
		break;
	}
}
