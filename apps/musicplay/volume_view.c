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
#include "ui_statictext.h"
#include "task/syscall.h"

#include "soundplay.h"
#include "musicplay.h"
#include "musicplay_view.h"
#include "volume_view.h"

//#define DEBUGTBITS 0x01
#include "dtprintf.h"

int volume = VOL_DEF;
int flg_mute = 0;

/*
 */

#if GSC_GRAPHICS_DISPLAY_WIDTH < 800
#undef BUTTON_HEIGHT
#define BUTTON_HEIGHT	52
#endif

#define BTN_POS_X_PLUS	V_BTN_LEFT
#define BTN_POS_Y_PLUS	(TOPAREAMARGINE + TEXT_INTERVAL)

#define BTN_POS_X_MINUS	V_BTN_LEFT
#define BTN_POS_Y_MINUS	(BTN_POS_Y_PLUS + BUTTON_HEIGHT + BTN_INTERVAL)

#define BTN_POS_X_MUTE	V_BTN_LEFT
#define BTN_POS_Y_MUTE	(BTN_POS_Y_PLUS + BUTTON_HEIGHT + BTN_INTERVAL + BUTTON_HEIGHT + BTN_INTERVAL)

#define UO_ID_PLUS	1
#define UO_ID_MINUS	2
#define UO_ID_MUTE	3

const struct st_box plus_btn_box	= { {BTN_POS_X_PLUS,  BTN_POS_Y_PLUS}, {BUTTON_WIDTH, BUTTON_HEIGHT} };
const struct st_box minus_btn_box	= { {BTN_POS_X_MINUS, BTN_POS_Y_MINUS}, {BUTTON_WIDTH, BUTTON_HEIGHT} };


const struct st_graph_object plus_mark[] = {
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FILL_BOX,	{ BUTTON_WIDTH/4, BUTTON_HEIGHT/16*7, BUTTON_WIDTH/2, BUTTON_HEIGHT/8 } },
	{ GO_TYPE_FILL_BOX,	{ BUTTON_WIDTH/16*7, BUTTON_HEIGHT/4, BUTTON_WIDTH/8, BUTTON_HEIGHT/2 } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object plus_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)plus_mark},
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object plus_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)plus_mark},
	{ 0, { 0, 0, 0, 0 }}
};


const struct st_graph_object minus_mark[] = {
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FILL_BOX,	{ BUTTON_WIDTH/4, BUTTON_HEIGHT/16*7, BUTTON_WIDTH/2, BUTTON_HEIGHT/8 } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object minus_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)minus_mark},
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object minus_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)minus_mark},
	{ 0, { 0, 0, 0, 0 }}
};

#define BWS	(BUTTON_WIDTH/16)
#define BHS	(BUTTON_HEIGHT/16)

const struct st_graph_object mute_mark[] = {
	{ GO_TYPE_FILL_BOX,	{ BWS*2, BHS*6, BWS*2, BHS*4 } },
	{ GO_TYPE_VERTEX4,	{ BWS*8, BHS*2, BWS*8, BHS*14, BWS*4, BHS*10, BWS*4, BHS*6 } },
	{ GO_TYPE_VERTEX4,	{ BWS*11, BHS*6, BWS*10, BHS*7, BWS*13, BHS*10, BWS*14, BHS*9 } },
	{ GO_TYPE_VERTEX4,	{ BWS*13, BHS*6, BWS*12, BHS*7, BWS*13, BHS*8, BWS*14, BHS*7 } },
	{ GO_TYPE_VERTEX4,	{ BWS*11, BHS*8, BWS*12, BHS*9, BWS*11, BHS*10, BWS*10, BHS*9 } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object mute_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)mute_mark},
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object mute_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)mute_mark},
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object muting_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_RED_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)mute_mark},
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object muting_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_RED_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)mute_mark},
	{ 0, { 0, 0, 0, 0 }}
};

/* [+] */
static const struct st_ui_button_image ui_view_plus = {
	plus_btn_obj,
	plus_btn_obj_a
};

static struct st_ui_button ui_btn_plus = {
	UO_ID_PLUS,
	{ {BTN_POS_X_PLUS,  BTN_POS_Y_PLUS}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	&ui_view_plus,
	UI_BUTTON_ST_NORMAL
};

/* [-] */
static const struct st_ui_button_image ui_view_minus = {
	minus_btn_obj,
	minus_btn_obj_a
};

static struct st_ui_button ui_btn_minus = {
	UO_ID_MINUS,
	{ {BTN_POS_X_MINUS,  BTN_POS_Y_MINUS}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	&ui_view_minus,
	UI_BUTTON_ST_NORMAL
};

/* MUTE */
static const struct st_ui_button_image ui_view_mute = {
	mute_btn_obj,
	mute_btn_obj_a
};

static const struct st_ui_button_image ui_view_muting = {
	muting_btn_obj,
	muting_btn_obj_a
};

static struct st_ui_button ui_btn_mute = {
	UO_ID_MUTE,
	{ {BTN_POS_X_MUTE,  BTN_POS_Y_MUTE}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
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

const struct st_graph_object active_bar_color[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BAR_COLOR } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object inactive_bar_color[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_INACTIVE_BAR_COLOR } },
	{ 0, { 0, 0, 0, 0 }}
};


static struct st_ui_seekbar ui_volume_slider = {
	.view_area = {{V_BTN_LEFT - SCRBAR_WIDTH - 4, BTN_POS_Y_PLUS}, {SCRBAR_WIDTH, INFO_HEIGHT-BTN_POS_Y_PLUS} },
	.type = UI_SKB_TYPE_VERTICAL,
	.attr = UI_SKB_ATTR_REALTIME_VALUE_CAHNGE,
	.flg_active = 1,
	.normal_view = normal_color_view,
	.bar_color = active_bar_color,
	.bar_inactive_color = inactive_bar_color,
	.value = VOL_DEF,
	.max_value = 99,
};

static unsigned char vol_str[3];
static struct st_ui_statictext ui_volume_vol = {
	.view_area = {
		.pos.x		= V_BTN_LEFT,
		.pos.y		= 0,
		.sur.width	= DEFFONT_WIDTH * 3,
		.sur.height	= DEFFONT_HEIGHR,
	},
	.fore_color	= &fore_color,
	.back_color	= &back_color,
	.font_name	= MPFONT,
	.hattr		= FONT_HATTR_LEFT,
	.vattr		= FONT_VATTR_TOP,
	.text		= (uchar *)"VOL",
};

static struct st_ui_statictext ui_volume_val = {
	.view_area = {
		.pos.x		= V_BTN_LEFT + DEFFONT_WIDTH * 4,
		.pos.y		= 0,
		.sur.width	= DEFFONT_WIDTH * 2,
		.sur.height	= DEFFONT_HEIGHR,
	},
	.fore_color	= &fore_color,
	.back_color	= &back_color,
	.font_name	= MPFONT,
	.hattr		= FONT_HATTR_LEFT,
	.vattr		= FONT_VATTR_TOP,
	.text		= vol_str,
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
	tsprintf((char *)vol_str, "%2d", volume);
	draw_ui_statictext(&ui_volume_val);
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

static void set_now_volume(unsigned short vol)
{
	soundplay_volume(vol);
}

void init_volume_view(void)
{
	ui_volume_slider.value = volume;

	if(flg_mute != 0) {
		ui_btn_mute.view = &ui_view_muting;
		set_now_volume(0);
	} else {
		ui_btn_mute.view = &ui_view_mute;
		set_now_volume(volume);
	}

	task_sleep(10);

	tsprintf((char *)vol_str, "%2d", volume);
}

void set_volume_view(int vol, int mute)
{
	ui_volume_slider.value = vol;

	if(mute != 0) {
		ui_btn_mute.view = &ui_view_muting;
		set_now_volume(0);
	} else {
		ui_btn_mute.view = &ui_view_mute;
		set_now_volume(vol);
	}

	tsprintf((char *)vol_str, "%2d", vol);
}

void draw_volume_view(void)
{
	draw_ui_button_list(ui_volume_view);

	draw_ui_seekbar(&ui_volume_slider);

	draw_ui_statictext(&ui_volume_vol);
	draw_ui_statictext(&ui_volume_val);
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
			case UI_BUTTON_EVT_REPEAT:
				if(volume < VOL_MAX) {
					volume ++;
					set_now_volume(volume);
					if(flg_mute != 0) {
						flg_mute = 0;
						set_mute_button_notmute();
					}
				}
				break;
			}
			switch(obj_evt.what) {
			case UI_BUTTON_EVT_PULL:
				save_config();
				break;
			}
			break;

		case UO_ID_MINUS:
			switch(obj_evt.what) {
			case UI_BUTTON_EVT_PUSH:
			case UI_BUTTON_EVT_REPEAT:
				if(VOL_MIN < volume) {
					volume --;
					set_now_volume(volume);
					if(flg_mute != 0) {
						flg_mute = 0;
						set_mute_button_notmute();
					}
				}
				break;
			}
			switch(obj_evt.what) {
			case UI_BUTTON_EVT_PULL:
				save_config();
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
				save_config();
				break;
			}
			break;

		default:
			break;

		}
	}

	{
		int sbrtn = proc_ui_seekbar(&ui_volume_slider, event, &new_volume);
		switch(sbrtn) {
		case UI_SKB_EVT_CHANGE:
			DTPRINTF(0x01, "slider = %d\n", new_volume);
			flg_mute = 0;
			set_mute_button_notmute();
			set_volume(new_volume);
			break;

		case UI_SKB_EVT_TOUCHEND:
			DTPRINTF(0x01, "slider = %d\n", new_volume);
			set_volume(new_volume);
			save_config();
			break;

		default:
			break;
		}
	}

	switch(event->what) {
	case EVT_SOUND_VOLUME:
		DTPRINTF(0x01, "volume = %d\n", event->arg);
		set_disp_volume(event->arg);
		break;
	}
}
