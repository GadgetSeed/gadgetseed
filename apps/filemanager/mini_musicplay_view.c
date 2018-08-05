/** @file
    @brief	ファイルマネージャミニミュージックプレーヤ

    @date	2017.12.10
    @author	Takashi SHUDO
*/

#include "key.h"
#include "graphics.h"
#include "font.h"
#include "tprintf.h"
#include "shell.h"
#include "graphics_object.h"
#include "filemanager.h"
#include "ui_button.h"

#include "mini_musicplay_view.h"

const struct st_graph_object stop_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ FM_CTRL_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ FM_CTRL_FORE_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FILL_BOX,	{ BUTTON_WIDTH/4, BUTTON_HEIGHT/4, BUTTON_WIDTH/2, BUTTON_HEIGHT/2 } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object stop_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ FM_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ FM_CTRL_FORE_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FILL_BOX,	{ BUTTON_WIDTH/4, BUTTON_HEIGHT/4, BUTTON_WIDTH/2, BUTTON_HEIGHT/2 } },
	{ 0, { 0, 0, 0, 0 }}
};

#if 0
const struct st_graph_object play_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ FM_CTRL_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ FM_CTRL_FORE_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_VERTEX4,	{ BUTTON_WIDTH/4, BUTTON_HEIGHT/4, BUTTON_WIDTH/4*3, BUTTON_HEIGHT/2,
				  BUTTON_WIDTH/4*3, BUTTON_HEIGHT/2, BUTTON_WIDTH/4, BUTTON_HEIGHT/4*3} },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object play_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ FM_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ FM_CTRL_FORE_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_VERTEX4,	{ BUTTON_WIDTH/4, BUTTON_HEIGHT/4, BUTTON_WIDTH/4*3, BUTTON_HEIGHT/2,
				  BUTTON_WIDTH/4*3, BUTTON_HEIGHT/2, BUTTON_WIDTH/4, BUTTON_HEIGHT/4*3} },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object playing_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ FM_CTRL_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ FM_CTRL_FORE_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_GREEN_COLOR } },
	{ GO_TYPE_VERTEX4,	{ BUTTON_WIDTH/4, BUTTON_HEIGHT/4, BUTTON_WIDTH/4*3, BUTTON_HEIGHT/2,
				  BUTTON_WIDTH/4*3, BUTTON_HEIGHT/2, BUTTON_WIDTH/4, BUTTON_HEIGHT/4*3} },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object playing_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ FM_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ FM_CTRL_FORE_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ MP_GREEN_COLOR } },
	{ GO_TYPE_VERTEX4,	{ BUTTON_WIDTH/4, BUTTON_HEIGHT/4, BUTTON_WIDTH/4*3, BUTTON_HEIGHT/2,
				  BUTTON_WIDTH/4*3, BUTTON_HEIGHT/2, BUTTON_WIDTH/4, BUTTON_HEIGHT/4*3} },
	{ 0, { 0, 0, 0, 0 }}
};
#endif

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

#if 0
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
#endif

struct st_ui_button *ui_play_view[] = {
	&ui_btn_stop,
//	&ui_btn_play,
	0
};

static unsigned int playtime;
static unsigned char str_ptime[8] = "  0:00";

static void draw_playtime(void)
{
	set_forecolor(FM_CTRL_FORE_COLOR);
	set_backcolor(FM_CTRL_BACK_COLOR);
//	set_font_by_name("num48x64");
	set_font_drawmode(FONT_FIXEDWIDTH);
	draw_str(BTN_POS_X_STOP - font_width(' ')*7,
		 GSC_GRAPHICS_DISPLAY_HEIGHT - CONTROL_HEIGHT + (CONTROL_HEIGHT - font_height())/2, str_ptime);
}

static void set_playtime(unsigned int time)
{
	playtime = time;
	tsprintf((char *)str_ptime, "%3d:%02d", playtime/60, playtime % 60);

	draw_playtime();
}

void init_mini_musicplay_view(void)
{
}

void draw_mini_musicplay_view(void)
{
	draw_ui_button_list(ui_play_view);
	draw_playtime();
}

void mini_musicplay_proc(struct st_sysevent *event)
{
	switch(event->what) {
	case EVT_SOUND_ANALYZE:
		break;

	case EVT_SOUND_PREPARE:
		break;

	case EVT_SOUND_START:
		break;

	case EVT_SOUND_END:
		break;

	case EVT_SOUND_STOP:
		break;

	case EVT_SOUND_PAUSE:
		break;

	case EVT_SOUND_CONTINUE:
		break;

	case EVT_SOUND_STATUS:
		set_playtime(*(unsigned long *)event->private_data);
		break;

	case EVT_KEYDOWN:
	case EVT_KEYDOWN_REPEAT:
		switch(event->arg) {
		case KEY_GB_ESC:
		case KEY_GB_BS:
		case KEY_GB_SPACE:
			exec_command((uchar *)"sound stop");
			break;

		default:
			break;
		}

	default:
		break;
	}

	struct st_button_event obj_evt;

	if(proc_ui_button_list(&obj_evt, ui_play_view, event) != 0) {
		switch(obj_evt.id) {
		case UO_ID_STOP:
			if(obj_evt.what == UI_BUTTON_EVT_PUSH) {
				exec_command((uchar *)"sound stop");
			}
			break;

		case UO_ID_PLAY:
#if 0
			if(obj_evt.what == UI_BUTTON_EVT_PUSH) {
				tprintf("PLAY\n");
				if(musicplay_status == MUSICPLAY_STAT_STOP) {
					start_music_play();
				} else {
					continue_music_play();
				}
			}
#endif
			break;

		default:
			break;
		}
	}
}
