/** @file
    @brief	musicplay/internetradio各種設定

    @date	2019.12.07
    @auther	Takashi SHUDO
*/

#include "sysconfig.h"
#include "str.h"
#include "graphics.h"
#include "graphics_object.h"
#include "font.h"
#include "tprintf.h"
#include "ui_button.h"
#include "dialogbox/netset/netset.h"

#include "musicplay.h"
#include "radio.h"
#include "mode_view.h"
#include "musicplay_view.h"
#include "volume_view.h"
#include "config_view.h"
#include "settings_view.h"


//#define DEBUGTBITS 0x01
#include "dtprintf.h"

#define BTN_POS_X_CONFIG	(0)
#define BTN_POS_Y_CONFIG	H_BTN_TOP

const struct st_graph_object config_mark[] = {
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
//	{ GO_TYPE_FONT,		{ 0 }, (void *)GSC_FONTS_DEFAULT_FONT },
//	{ GO_TYPE_TEXT_IN_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT, FONT_HATTR_CENTER, FONT_VATTR_CENTER }, (void *)"CONFIG" },
	{ GO_TYPE_SECTOR,	{ BUTTON_WIDTH/2, BUTTON_HEIGHT/2, BUTTON_WIDTH/4, BUTTON_WIDTH/8, 0 } },
	{ GO_TYPE_SECTOR,	{ BUTTON_WIDTH/2, BUTTON_HEIGHT/2, BUTTON_WIDTH/4, BUTTON_WIDTH/8, 1 } },
	{ GO_TYPE_SECTOR,	{ BUTTON_WIDTH/2, BUTTON_HEIGHT/2, BUTTON_WIDTH/4, BUTTON_WIDTH/8, 2 } },
	{ GO_TYPE_SECTOR,	{ BUTTON_WIDTH/2, BUTTON_HEIGHT/2, BUTTON_WIDTH/4, BUTTON_WIDTH/8, 3 } },
	{ GO_TYPE_FILL_BOX,	{ BUTTON_WIDTH/16*7, BUTTON_HEIGHT/16*3, BUTTON_WIDTH/16*2, BUTTON_HEIGHT/32*3 } },
	{ GO_TYPE_FILL_BOX,	{ BUTTON_WIDTH/16*7, BUTTON_HEIGHT/32*23, BUTTON_WIDTH/16*2, BUTTON_HEIGHT/32*3 } },
	{ GO_TYPE_FILL_BOX,	{ BUTTON_WIDTH/16*3, BUTTON_HEIGHT/16*7, BUTTON_WIDTH/32*3, BUTTON_HEIGHT/16*2 } },
	{ GO_TYPE_FILL_BOX,	{ BUTTON_WIDTH/32*23, BUTTON_HEIGHT/16*7, BUTTON_WIDTH/32*3, BUTTON_HEIGHT/16*2 } },
	{ GO_TYPE_VERTEX4,	{ BUTTON_WIDTH/16*5, BUTTON_HEIGHT/32*7, BUTTON_WIDTH/16*6, BUTTON_HEIGHT/32*9,
				  BUTTON_WIDTH/32*9, BUTTON_HEIGHT/16*6, BUTTON_WIDTH/32*7, BUTTON_HEIGHT/16*5 } },
	{ GO_TYPE_VERTEX4,	{ BUTTON_WIDTH/16*11, BUTTON_HEIGHT/32*7, BUTTON_WIDTH/32*25, BUTTON_HEIGHT/16*5,
				  BUTTON_WIDTH/32*23, BUTTON_HEIGHT/16*6, BUTTON_WIDTH/16*10, BUTTON_HEIGHT/32*9 } },
	{ GO_TYPE_VERTEX4,	{ BUTTON_WIDTH/32*23, BUTTON_HEIGHT/16*10, BUTTON_WIDTH/32*25, BUTTON_HEIGHT/16*11,
				  BUTTON_WIDTH/16*11, BUTTON_HEIGHT/32*25, BUTTON_WIDTH/16*10, BUTTON_HEIGHT/32*23 } },
	{ GO_TYPE_VERTEX4,	{ BUTTON_WIDTH/32*9, BUTTON_HEIGHT/16*10, BUTTON_WIDTH/16*6, BUTTON_HEIGHT/32*23,
				  BUTTON_WIDTH/16*5, BUTTON_HEIGHT/32*25, BUTTON_WIDTH/32*7, BUTTON_HEIGHT/16*11 } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object config_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_BACK_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)config_mark},
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object config_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ MP_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)config_mark},
	{ 0, { 0, 0, 0, 0 }}
};

#define UO_ID_RADIO_CONFIG	15

/* CONFIG */
static const struct st_ui_button_image ui_view_config = {
	config_btn_obj,
	config_btn_obj_a
};

static struct st_ui_button ui_btn_config = {
	UO_ID_RADIO_CONFIG,
	{ {BTN_POS_X_CONFIG,  BTN_POS_Y_CONFIG}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	&ui_view_config,
	UI_BUTTON_ST_NORMAL
};


void init_config_view(void)
{
}

void draw_config_view(void)
{
	draw_ui_button(&ui_btn_config);
}

void config_proc(struct st_sysevent *event)
{
	int evt;

	evt = proc_ui_button(&ui_btn_config, event);

	if(evt == UI_BUTTON_EVT_PULL) {
		open_settings_dialog();

		init_musicplay_view();
		draw_musicplay_view();
	}
}
